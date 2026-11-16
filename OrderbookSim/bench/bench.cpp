/*!
 * @file bench.cpp
 * @brief Per-operation latency, throughput and allocation benchmark for the std::map Orderbook.
 *
 * 1. Generate: a seeded synthetic order flow is run once against a book; every call (prefill,
 *    warm-up, measured) is recorded with its outcome, so later passes replay identical work.
 * 2. For each repetition, on a fresh book:
 *    - latency pass: replay prefill and warm-up untimed, then time every measured call
 *      separately and count the allocations it makes;
 *    - throughput pass: replay again and time the measured calls as one block.
 *
 * Operations are classified by outcome: add (rests, no trade), match (at least one trade, split
 * by order type), reject (Fill-and-Kill / Fill-or-Kill / Market with no trade) and cancel.
 * Output: CSV files in --out (see bench/RESULTS.md for the columns).
 */

#include "Orderbook.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <new>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <sys/resource.h>
#if defined(__APPLE__)
#include <pthread/qos.h>
#endif

// ---------------------------------------------------------------- allocation counting
// Replaces global operator new/delete for this binary. Plain counters: the only other thread
// (the Good-For-Day pruner) does not allocate unless it fires at 16:00 local time.
static std::uint64_t g_allocs = 0;
static std::uint64_t g_bytes = 0;

void* operator new(std::size_t n) {
    ++g_allocs;
    g_bytes += n;
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {

using Clock = std::chrono::steady_clock;

struct Config {
    std::uint64_t seed = 42;
    int reps = 10;
    std::size_t events = 1'000'000; // measured calls per repetition
    std::size_t warmup = 100'000;   // untimed calls replayed before the measured ones
    int levels = 50;                // passive prices are mid +/- [1, levels] ticks
    std::size_t target = 1'000;     // resting orders the flow reverts towards (and prefill size)
    std::string out = "bench-out";
};

enum class Kind : std::uint8_t { Add, Cancel };
enum Category { AddRest, Cancel, MatchGTC, MatchGFD, MatchFAK, MatchFOK, MatchMarket, RejectFAK, RejectFOK, RejectMarket, NumCategories };
const char* kCategoryNames[NumCategories] = {
    "add", "cancel", "match_gtc", "match_gfd", "match_fak", "match_fok", "match_market",
    "reject_fak", "reject_fok", "reject_market" };

struct Event {
    Kind kind;
    OrderType type;
    Side side;
    Price price;
    Quantity qty;
    OrderId id;
    std::uint32_t trades; // outcome recorded by the generation pass
    Category category;
};

Category classify(const Event& e) {
    if (e.kind == Kind::Cancel) return Cancel;
    const bool traded = e.trades > 0;
    switch (e.type) {
    case OrderType::GoodTillCancel: return traded ? MatchGTC : AddRest;
    case OrderType::GoodForDay:     return traded ? MatchGFD : AddRest;
    case OrderType::FillAndKill:    return traded ? MatchFAK : RejectFAK;
    case OrderType::FillOrKill:     return traded ? MatchFOK : RejectFOK;
    case OrderType::Market:         return traded ? MatchMarket : RejectMarket;
    }
    return AddRest;
}

struct Workload {
    std::vector<Event> events; // prefill + warm-up + measured, in order
    std::size_t prefill = 0;
    std::size_t finalSize = 0;
    double meanLive = 0, meanBidLevels = 0, meanAskLevels = 0;
    std::size_t minLive = SIZE_MAX, maxLive = 0;
};

// Mirrors which orders rest, from the trades the book returns, so cancels target live orders.
struct LiveSet {
    std::vector<OrderId> ids;
    std::unordered_map<OrderId, std::pair<std::size_t, Quantity>> at; // id -> (index, remaining)

    void insert(OrderId id, Quantity q) { at[id] = { ids.size(), q }; ids.push_back(id); }
    void erase(OrderId id) {
        const auto it = at.find(id);
        const std::size_t i = it->second.first;
        ids[i] = ids.back();
        at[ids[i]].first = i;
        ids.pop_back();
        at.erase(id);
    }
    void fill(OrderId id, Quantity q) {
        const auto it = at.find(id);
        if (it == at.end()) return;
        if ((it->second.second -= q) == 0) erase(id);
    }
};

Workload generate(const Config& cfg) {
    Workload w;
    Orderbook book;
    LiveSet live;
    std::mt19937_64 rng(cfg.seed);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::uniform_int_distribution<int> depth(1, cfg.levels);
    std::uniform_int_distribution<Quantity> passiveQty(1, 100), aggressiveQty(1, 200);
    const Price mid = 100'000;
    OrderId nextId = 1;
    const std::size_t total = cfg.target + cfg.warmup + cfg.events;
    w.events.reserve(total);
    std::size_t samples = 0;

    for (std::size_t i = 0; i < total; ++i) {
        const bool prefill = i < cfg.target;
        const bool measured = i >= cfg.target + cfg.warmup;
        Event e{};
        const double u = unit(rng);
        const Side side = unit(rng) < 0.5 ? Side::Buy : Side::Sell;
        const double pAdd = double(cfg.target) / double(cfg.target + live.ids.size());

        if (!prefill && u < 0.10) {
            // aggressive: one of Market, FAK, FOK, GTC, limit `levels` ticks through the mid
            static const OrderType kinds[] = { OrderType::Market, OrderType::FillAndKill, OrderType::FillOrKill, OrderType::GoodTillCancel };
            e = { Kind::Add, kinds[std::uniform_int_distribution<int>(0, 3)(rng)], side,
                  side == Side::Buy ? mid + cfg.levels : mid - cfg.levels, aggressiveQty(rng), nextId++ };
            if (e.type == OrderType::Market) e.price = 0;
        } else if (prefill || live.ids.empty() || unit(rng) < pAdd) {
            // passive: Good-Till-Cancel (90%) or Good-For-Day (10%) at mid +/- [1, levels]
            const OrderType t = unit(rng) < 0.9 ? OrderType::GoodTillCancel : OrderType::GoodForDay;
            const Price off = depth(rng);
            e = { Kind::Add, t, side, side == Side::Buy ? mid - off : mid + off, passiveQty(rng), nextId++ };
        } else {
            const auto idx = std::uniform_int_distribution<std::size_t>(0, live.ids.size() - 1)(rng);
            e = { Kind::Cancel, OrderType::GoodTillCancel, side, 0, 0, live.ids[idx] };
        }

        if (e.kind == Kind::Cancel) {
            book.CancelOrder(e.id);
            live.erase(e.id);
        } else {
            const Trades trades = book.addOrder(e.type, e.id, e.side, e.price, e.qty);
            e.trades = static_cast<std::uint32_t>(trades.size());
            Quantity filled = 0;
            for (const auto& t : trades) {
                const bool aggressorIsBid = t.getBidTrade()._orderId == e.id;
                live.fill(aggressorIsBid ? t.getAskTrade()._orderId : t.getBidTrade()._orderId, t.getBidTrade()._quantity);
                filled += t.getBidTrade()._quantity;
            }
            const bool rests = e.type == OrderType::GoodTillCancel || e.type == OrderType::GoodForDay
                || (e.type == OrderType::Market && !trades.empty());
            if (rests && filled < e.qty) live.insert(e.id, e.qty - filled);
        }
        e.category = classify(e);
        w.events.push_back(e);

        if (measured && i % 1000 == 0) {
            const auto infos = book.getOrderInfos();
            w.meanLive += live.ids.size();
            w.meanBidLevels += infos.getBids().size();
            w.meanAskLevels += infos.getAsks().size();
            w.minLive = std::min(w.minLive, live.ids.size());
            w.maxLive = std::max(w.maxLive, live.ids.size());
            ++samples;
        }
        if (i % 10'000 == 0 && live.ids.size() != book.Size()) {
            std::fprintf(stderr, "live-set mirror diverged at event %zu (%zu vs %zu)\n", i, live.ids.size(), book.Size());
            std::exit(1);
        }
    }
    w.prefill = cfg.target;
    w.finalSize = book.Size();
    w.meanLive /= samples;
    w.meanBidLevels /= samples;
    w.meanAskLevels /= samples;
    return w;
}

inline std::size_t apply(Orderbook& book, const Event& e) {
    if (e.kind == Kind::Cancel) { book.CancelOrder(e.id); return 0; }
    return book.addOrder(e.type, e.id, e.side, e.price, e.qty).size();
}

void replayUntimed(Orderbook& book, const Workload& w, std::size_t from, std::size_t to) {
    for (std::size_t i = from; i < to; ++i) {
        if (apply(book, w.events[i]) != w.events[i].trades) {
            std::fprintf(stderr, "replay diverged at event %zu\n", i);
            std::exit(1);
        }
    }
}

struct Sample { std::uint32_t ns; std::uint32_t allocs; std::uint64_t bytes; };

double percentile(const std::vector<std::uint32_t>& sorted, double p) {
    // nearest rank
    const std::size_t rank = static_cast<std::size_t>(std::ceil(p / 100.0 * sorted.size()));
    return sorted[std::min(sorted.size() - 1, rank == 0 ? 0 : rank - 1)];
}

std::FILE* openOut(const Config& cfg, const char* name, const char* header) {
    const std::string path = cfg.out + "/" + name;
    std::FILE* f = std::fopen(path.c_str(), "w");
    if (!f) { std::perror(path.c_str()); std::exit(1); }
    std::fputs(header, f);
    return f;
}

} // namespace

int main(int argc, char** argv) {
    Config cfg;
    for (int i = 1; i + 1 < argc; i += 2) {
        const std::string k = argv[i], v = argv[i + 1];
        if (k == "--seed") cfg.seed = std::stoull(v);
        else if (k == "--reps") cfg.reps = std::stoi(v);
        else if (k == "--events") cfg.events = std::stoull(v);
        else if (k == "--warmup") cfg.warmup = std::stoull(v);
        else if (k == "--levels") cfg.levels = std::stoi(v);
        else if (k == "--target") cfg.target = std::stoull(v);
        else if (k == "--out") cfg.out = v;
        else { std::fprintf(stderr, "unknown option %s\n", k.c_str()); return 2; }
    }
#if defined(__APPLE__)
    // macOS has no thread pinning; the highest QoS class keeps the thread on performance cores.
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
    std::string mk = "mkdir -p '" + cfg.out + "'";
    if (std::system(mk.c_str()) != 0) return 1;

    // Clock overhead and resolution: back-to-back reads.
    std::vector<std::uint32_t> overhead(1'000'000);
    std::uint32_t resolution = UINT32_MAX;
    for (auto& o : overhead) {
        const auto a = Clock::now();
        const auto b = Clock::now();
        o = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count());
        if (o > 0) resolution = std::min(resolution, o);
    }
    std::sort(overhead.begin(), overhead.end());

    const auto genStart = Clock::now();
    const Workload w = generate(cfg);
    const double genSeconds = std::chrono::duration<double>(Clock::now() - genStart).count();
    const std::size_t measuredFrom = w.prefill + cfg.warmup;
    const std::size_t total = w.events.size();

    std::size_t counts[NumCategories] = {};
    std::uint64_t tradesIn[NumCategories] = {};
    for (std::size_t i = measuredFrom; i < total; ++i) {
        ++counts[w.events[i].category];
        tradesIn[w.events[i].category] += w.events[i].trades;
    }

    std::FILE* wl = openOut(cfg, "workload.csv", "key,value\n");
    std::fprintf(wl, "seed,%llu\nreps,%d\nprefill_orders,%zu\nwarmup_calls,%zu\nmeasured_calls,%zu\nlevels_each_side,%d\n"
        "target_resting_orders,%zu\nmean_resting_orders,%.1f\nmin_resting_orders,%zu\nmax_resting_orders,%zu\n"
        "mean_bid_levels,%.1f\nmean_ask_levels,%.1f\nfinal_book_size,%zu\nclock_resolution_ns,%u\n"
        "clock_overhead_p50_ns,%u\nclock_overhead_p99_ns,%u\ngeneration_seconds,%.2f\n",
        (unsigned long long)cfg.seed, cfg.reps, w.prefill, cfg.warmup, cfg.events, cfg.levels, cfg.target,
        w.meanLive, w.minLive, w.maxLive, w.meanBidLevels, w.meanAskLevels, w.finalSize, resolution,
        overhead[overhead.size() / 2], overhead[overhead.size() * 99 / 100], genSeconds);
    for (int c = 0; c < NumCategories; ++c)
        std::fprintf(wl, "calls_%s,%zu\ntrades_%s,%llu\n", kCategoryNames[c], counts[c], kCategoryNames[c], (unsigned long long)tradesIn[c]);
    std::fclose(wl);

    std::FILE* lat = openOut(cfg, "latency_per_rep.csv",
        "rep,op,n,mean_ns,p25_ns,p50_ns,p75_ns,p90_ns,p99_ns,p999_ns,max_ns,allocs_per_op,bytes_per_op,max_allocs,slowest_call_bytes\n");
    std::FILE* thr = openOut(cfg, "throughput_per_rep.csv", "rep,calls,seconds,calls_per_second\n");

    // log-spaced histogram, 20 bins per decade from 1 ns, pooled over repetitions
    constexpr int kBinsPerDecade = 20, kBins = 8 * kBinsPerDecade;
    std::vector<std::uint64_t> hist(NumCategories * kBins, 0);

    std::vector<Sample> samples(total - measuredFrom);
    for (int rep = 1; rep <= cfg.reps; ++rep) {
        {   // latency pass
            Orderbook book;
            replayUntimed(book, w, 0, measuredFrom);
            for (std::size_t i = measuredFrom; i < total; ++i) {
                const Event& e = w.events[i];
                const std::uint64_t a0 = g_allocs, b0 = g_bytes;
                const auto t0 = Clock::now();
                const std::size_t trades = apply(book, e);
                const auto t1 = Clock::now();
                const std::uint64_t a1 = g_allocs, b1 = g_bytes;
                if (trades != e.trades) { std::fprintf(stderr, "replay diverged at event %zu\n", i); return 1; }
                samples[i - measuredFrom] = { static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()),
                                              static_cast<std::uint32_t>(a1 - a0), b1 - b0 };
            }
        }
        for (int c = 0; c < NumCategories; ++c) {
            std::vector<std::uint32_t> ns;
            std::uint64_t allocs = 0, bytes = 0, sum = 0;
            std::uint32_t maxAllocs = 0, slowest = 0;
            std::uint64_t slowestBytes = 0; // bytes allocated by the slowest call: explains the max
            for (std::size_t i = measuredFrom; i < total; ++i) {
                if (w.events[i].category != c) continue;
                const Sample& s = samples[i - measuredFrom];
                ns.push_back(s.ns);
                sum += s.ns;
                allocs += s.allocs;
                bytes += s.bytes;
                maxAllocs = std::max(maxAllocs, s.allocs);
                if (s.ns >= slowest) { slowest = s.ns; slowestBytes = s.bytes; }
                const int bin = s.ns == 0 ? 0 : std::clamp(static_cast<int>(std::floor(std::log10(double(s.ns)) * kBinsPerDecade)), 0, kBins - 1);
                ++hist[c * kBins + bin];
            }
            if (ns.empty()) continue;
            std::sort(ns.begin(), ns.end());
            const double n = double(ns.size());
            std::fprintf(lat, "%d,%s,%zu,%.1f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%u,%.3f,%.1f,%u,%llu\n", rep, kCategoryNames[c], ns.size(),
                sum / n, percentile(ns, 25), percentile(ns, 50), percentile(ns, 75), percentile(ns, 90), percentile(ns, 99),
                percentile(ns, 99.9), ns.back(), allocs / n, bytes / n, maxAllocs, (unsigned long long)slowestBytes);
        }
        {   // throughput pass
            Orderbook book;
            replayUntimed(book, w, 0, measuredFrom);
            std::size_t checksum = 0;
            const auto t0 = Clock::now();
            for (std::size_t i = measuredFrom; i < total; ++i) checksum += apply(book, w.events[i]);
            const double s = std::chrono::duration<double>(Clock::now() - t0).count();
            std::fprintf(thr, "%d,%zu,%.6f,%.0f\n", rep, total - measuredFrom, s, (total - measuredFrom) / s);
            if (book.Size() != w.finalSize || checksum == SIZE_MAX) { std::fprintf(stderr, "throughput replay diverged\n"); return 1; }
        }
        std::fprintf(stderr, "rep %d/%d done\n", rep, cfg.reps);
    }
    std::fclose(lat);
    std::fclose(thr);

    std::FILE* h = openOut(cfg, "latency_histogram.csv", "op,bin_lo_ns,bin_hi_ns,count\n");
    for (int c = 0; c < NumCategories; ++c)
        for (int b = 0; b < kBins; ++b)
            if (hist[c * kBins + b])
                std::fprintf(h, "%s,%.1f,%.1f,%llu\n", kCategoryNames[c], std::pow(10.0, double(b) / kBinsPerDecade),
                    std::pow(10.0, double(b + 1) / kBinsPerDecade), (unsigned long long)hist[c * kBins + b]);
    std::fclose(h);

    rusage ru{};
    getrusage(RUSAGE_SELF, &ru);
    std::FILE* m = openOut(cfg, "memory.csv", "key,value\n");
#if defined(__APPLE__)
    std::fprintf(m, "peak_rss_bytes,%ld\n", ru.ru_maxrss);
#else
    std::fprintf(m, "peak_rss_bytes,%ld\n", ru.ru_maxrss * 1024L);
#endif
    std::fclose(m);
    return 0;
}
