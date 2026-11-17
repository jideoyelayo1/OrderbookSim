/*!
 * @file MatchingContractTests.cpp
 * @brief Tests that pin the matching contract: price-time priority, no crossed book,
 *        each order type, cancel and modify.
 *
 * The data-driven suite in test.cpp only checks order and level counts, so it cannot tell
 * a correctly ordered book from one with the bid/ask comparators reversed. Every test here
 * checks which resting order traded, at which price, and in which order.
 */

#include "pch.h"

#include "../Orderbook.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

namespace {

OrderPtr makeOrder(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
    return std::make_shared<Order>(type, id, side, price, qty);
}

Trades gtc(Orderbook& book, OrderId id, Side side, Price price, Quantity qty) {
    return book.addOrder(makeOrder(OrderType::GoodTillCancel, id, side, price, qty));
}

Trades market(Orderbook& book, OrderId id, Side side, Quantity qty) {
    return book.addOrder(std::make_shared<Order>(id, side, qty));
}

std::vector<Price> prices(const LevelInfos& levels) {
    std::vector<Price> out;
    for (const auto& level : levels) out.push_back(level._price);
    return out;
}

std::vector<Quantity> quantities(const LevelInfos& levels) {
    std::vector<Quantity> out;
    for (const auto& level : levels) out.push_back(level._quantity);
    return out;
}

// Resting quantity an aggressor on `side` could reach at or better than `limit`.
Quantity reachable(const Orderbook& book, Side side, Price limit) {
    const auto infos = book.getOrderInfos();
    Quantity total = 0;
    if (side == Side::Buy) {
        for (const auto& level : infos.getAsks()) if (level._price <= limit) total += level._quantity;
    } else {
        for (const auto& level : infos.getBids()) if (level._price >= limit) total += level._quantity;
    }
    return total;
}

void expectUncrossedAndSorted(const Orderbook& book) {
    const auto infos = book.getOrderInfos();
    const auto bids = prices(infos.getBids());
    const auto asks = prices(infos.getAsks());
    EXPECT_TRUE(std::is_sorted(bids.begin(), bids.end(), std::greater<Price>())) << "bids must be best (highest) first";
    EXPECT_TRUE(std::is_sorted(asks.begin(), asks.end())) << "asks must be best (lowest) first";
    if (!bids.empty() && !asks.empty()) {
        EXPECT_LT(*std::max_element(bids.begin(), bids.end()), *std::min_element(asks.begin(), asks.end()))
            << "book is crossed";
    }
}

} // namespace

// ---------------------------------------------------------------- price priority

TEST(PricePriority, SellAggressorHitsHighestBidFirst) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 99, 10);
    gtc(book, 2, Side::Buy, 101, 10);
    gtc(book, 3, Side::Buy, 100, 10);

    const auto trades = gtc(book, 4, Side::Sell, 99, 10);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 2u);
    EXPECT_EQ(trades[0].getBidTrade()._price, 101);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 4u);
    EXPECT_EQ(trades[0].getAskTrade()._quantity, 10u);
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 100, 99 }));
}

TEST(PricePriority, BuyAggressorLiftsLowestAskFirst) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 103, 10);
    gtc(book, 2, Side::Sell, 101, 10);
    gtc(book, 3, Side::Sell, 102, 10);

    const auto trades = gtc(book, 4, Side::Buy, 103, 10);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 2u);
    EXPECT_EQ(trades[0].getAskTrade()._price, 101);
    EXPECT_EQ(prices(book.getOrderInfos().getAsks()), (std::vector<Price>{ 102, 103 }));
}

TEST(PricePriority, AggressorWalksLevelsBestToWorst) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 103, 5);
    gtc(book, 2, Side::Sell, 101, 5);
    gtc(book, 3, Side::Sell, 102, 5);

    const auto trades = gtc(book, 4, Side::Buy, 103, 12);

    ASSERT_EQ(trades.size(), 3u);
    EXPECT_EQ(trades[0].getAskTrade()._price, 101);
    EXPECT_EQ(trades[0].getAskTrade()._quantity, 5u);
    EXPECT_EQ(trades[1].getAskTrade()._price, 102);
    EXPECT_EQ(trades[1].getAskTrade()._quantity, 5u);
    EXPECT_EQ(trades[2].getAskTrade()._price, 103);
    EXPECT_EQ(trades[2].getAskTrade()._quantity, 2u);
    EXPECT_EQ(prices(book.getOrderInfos().getAsks()), (std::vector<Price>{ 103 }));
    EXPECT_EQ(quantities(book.getOrderInfos().getAsks()), (std::vector<Quantity>{ 3 }));
    EXPECT_TRUE(book.getOrderInfos().getBids().empty());
}

TEST(PricePriority, LevelsAreReportedBestFirst) {
    Orderbook book;
    for (Price p : { 97, 99, 98 }) gtc(book, static_cast<OrderId>(p), Side::Buy, p, 1);
    for (Price p : { 103, 101, 102 }) gtc(book, static_cast<OrderId>(p), Side::Sell, p, 1);

    const auto infos = book.getOrderInfos();
    EXPECT_EQ(prices(infos.getBids()), (std::vector<Price>{ 99, 98, 97 }));
    EXPECT_EQ(prices(infos.getAsks()), (std::vector<Price>{ 101, 102, 103 }));
    EXPECT_EQ(book.Size(), 6u);
}

// ---------------------------------------------------------------- time priority

TEST(TimePriority, FirstInFirstOutWithinALevel) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 100, 5);
    gtc(book, 3, Side::Buy, 100, 5);

    auto trades = gtc(book, 4, Side::Sell, 100, 7);
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 1u);
    EXPECT_EQ(trades[0].getBidTrade()._quantity, 5u);
    EXPECT_EQ(trades[1].getBidTrade()._orderId, 2u);
    EXPECT_EQ(trades[1].getBidTrade()._quantity, 2u);

    trades = gtc(book, 5, Side::Sell, 100, 3);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 2u) << "partially filled order keeps its place";
    EXPECT_EQ(book.Size(), 1u);
}

// ---------------------------------------------------------------- crossing

TEST(Crossing, NonCrossingOrdersRestWithoutTrading) {
    Orderbook book;
    EXPECT_TRUE(gtc(book, 1, Side::Buy, 99, 10).empty());
    EXPECT_TRUE(gtc(book, 2, Side::Sell, 101, 10).empty());
    EXPECT_TRUE(gtc(book, 3, Side::Buy, 100, 10).empty());
    EXPECT_TRUE(gtc(book, 4, Side::Sell, 100 + 1, 10).empty());
    EXPECT_EQ(book.Size(), 4u);
    expectUncrossedAndSorted(book);
}

TEST(Crossing, EqualPricesTrade) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 10);
    const auto trades = gtc(book, 2, Side::Buy, 100, 10);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(book.Size(), 0u);
}

// ---------------------------------------------------------------- order types

TEST(GoodTillCancel, RemainderRestsAtItsLimit) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    const auto trades = gtc(book, 2, Side::Buy, 101, 8);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._price, 100);
    EXPECT_EQ(trades[0].getBidTrade()._price, 101);
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 101 }));
    EXPECT_EQ(quantities(book.getOrderInfos().getBids()), (std::vector<Quantity>{ 3 }));
    EXPECT_TRUE(book.getOrderInfos().getAsks().empty());
}

TEST(FillAndKill, PartialFillThenRemainderIsCancelled) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    const auto trades = book.addOrder(makeOrder(OrderType::FillAndKill, 2, Side::Buy, 101, 8));
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._quantity, 5u);
    EXPECT_EQ(book.Size(), 0u);
    EXPECT_TRUE(book.getOrderInfos().getBids().empty());
}

TEST(FillAndKill, RejectedWhenNothingCrosses) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 101, 5);
    EXPECT_TRUE(book.addOrder(makeOrder(OrderType::FillAndKill, 2, Side::Buy, 100, 5)).empty());
    EXPECT_EQ(book.Size(), 1u);
    EXPECT_TRUE(book.getOrderInfos().getBids().empty());
}

TEST(FillOrKill, FillsAcrossLevelsWhenEnoughQuantity) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    gtc(book, 2, Side::Sell, 101, 5);
    const auto trades = book.addOrder(makeOrder(OrderType::FillOrKill, 3, Side::Buy, 101, 10));
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 1u);
    EXPECT_EQ(trades[1].getAskTrade()._orderId, 2u);
    EXPECT_EQ(book.Size(), 0u);
}

TEST(FillOrKill, RejectedWhenQuantityShortAndBookUntouched) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    gtc(book, 2, Side::Sell, 101, 5);
    EXPECT_TRUE(book.addOrder(makeOrder(OrderType::FillOrKill, 3, Side::Buy, 101, 11)).empty());
    EXPECT_EQ(book.Size(), 2u);
    EXPECT_EQ(quantities(book.getOrderInfos().getAsks()), (std::vector<Quantity>{ 5, 5 }));
}

TEST(FillOrKill, IgnoresQuantityBeyondItsLimit) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    gtc(book, 2, Side::Sell, 102, 5);
    EXPECT_TRUE(book.addOrder(makeOrder(OrderType::FillOrKill, 3, Side::Buy, 101, 6)).empty());
    EXPECT_EQ(book.Size(), 2u);
}

TEST(FillOrKill, SellSideUsesBidsAtOrAboveLimit) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 99, 5);
    gtc(book, 3, Side::Buy, 98, 5);
    EXPECT_TRUE(book.addOrder(makeOrder(OrderType::FillOrKill, 4, Side::Sell, 99, 11)).empty());
    const auto trades = book.addOrder(makeOrder(OrderType::FillOrKill, 5, Side::Sell, 99, 10));
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 1u);
    EXPECT_EQ(trades[1].getBidTrade()._orderId, 2u);
}

// A cross at a price where the aggressor then rests must leave that level visible to FOK.
TEST(FillOrKill, SeesQuantityRestingAfterSamePriceCross) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Sell, 100, 8); // trades 5, 3 rest at 100
    ASSERT_EQ(quantities(book.getOrderInfos().getAsks()), (std::vector<Quantity>{ 3 }));

    const auto trades = book.addOrder(makeOrder(OrderType::FillOrKill, 3, Side::Buy, 100, 3));
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 2u);
    EXPECT_EQ(book.Size(), 0u);
}

TEST(Market, BuySweepsAsksBestFirstAndDoesNotRest) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 102, 5);
    gtc(book, 2, Side::Sell, 100, 5);
    gtc(book, 3, Side::Sell, 101, 5);
    const auto trades = market(book, 4, Side::Buy, 7);
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getAskTrade()._price, 100);
    EXPECT_EQ(trades[0].getAskTrade()._quantity, 5u);
    EXPECT_EQ(trades[1].getAskTrade()._price, 101);
    EXPECT_EQ(trades[1].getAskTrade()._quantity, 2u);
    EXPECT_TRUE(book.getOrderInfos().getBids().empty());
    EXPECT_EQ(quantities(book.getOrderInfos().getAsks()), (std::vector<Quantity>{ 3, 5 }));
}

TEST(Market, SellSweepsBidsBestFirst) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 98, 5);
    gtc(book, 2, Side::Buy, 100, 5);
    gtc(book, 3, Side::Buy, 99, 5);
    const auto trades = market(book, 4, Side::Sell, 7);
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getBidTrade()._price, 100);
    EXPECT_EQ(trades[1].getBidTrade()._price, 99);
    EXPECT_EQ(trades[1].getBidTrade()._quantity, 2u);
    EXPECT_TRUE(book.getOrderInfos().getAsks().empty());
}

TEST(Market, RejectedWhenOppositeSideEmpty) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    EXPECT_TRUE(market(book, 2, Side::Buy, 5).empty());
    EXPECT_EQ(book.Size(), 1u);
}

// Existing contract (see TestFiles/Match_Market.txt): a market order larger than the opposite
// side becomes a limit order at the worst opposite price and its remainder rests there.
TEST(Market, RemainderRestsAtWorstOppositePrice) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 5);
    gtc(book, 2, Side::Sell, 101, 5);
    const auto trades = market(book, 3, Side::Buy, 12);
    EXPECT_EQ(trades.size(), 2u);
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 101 }));
    EXPECT_EQ(quantities(book.getOrderInfos().getBids()), (std::vector<Quantity>{ 2 }));
}

TEST(GoodForDay, RestsAndMatchesLikeGoodTillCancel) {
    Orderbook book;
    book.addOrder(makeOrder(OrderType::GoodForDay, 1, Side::Buy, 100, 5));
    EXPECT_EQ(book.Size(), 1u);
    const auto trades = gtc(book, 2, Side::Sell, 100, 5);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 1u);
}

TEST(GoodForDay, EndOfDayCancelsOnlyGoodForDayOrders) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 99, 5);
    book.addOrder(makeOrder(OrderType::GoodForDay, 2, Side::Buy, 98, 5));
    book.addOrder(makeOrder(OrderType::GoodForDay, 3, Side::Sell, 105, 5));
    gtc(book, 4, Side::Sell, 106, 5);

    book.EndOfDay();

    EXPECT_EQ(book.Size(), 2u);
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 99 }));
    EXPECT_EQ(prices(book.getOrderInfos().getAsks()), (std::vector<Price>{ 106 }));

    // The surviving orders are still fully managed: cancellable and matchable.
    book.CancelOrder(1);
    EXPECT_EQ(book.Size(), 1u);
    const auto trades = gtc(book, 5, Side::Buy, 106, 5);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 4u);
    EXPECT_EQ(book.Size(), 0u);
}

// ---------------------------------------------------------------- cancel

TEST(Cancel, RemovesOrderAndEmptyLevelOnly) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 99, 5);
    gtc(book, 3, Side::Sell, 101, 5);
    book.CancelOrder(1);
    EXPECT_EQ(book.Size(), 2u);
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 99 }));
    EXPECT_EQ(prices(book.getOrderInfos().getAsks()), (std::vector<Price>{ 101 }));
}

TEST(Cancel, MiddleOfQueueKeepsOthersInOrder) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 100, 5);
    gtc(book, 3, Side::Buy, 100, 5);
    book.CancelOrder(2);
    EXPECT_EQ(quantities(book.getOrderInfos().getBids()), (std::vector<Quantity>{ 10 }));
    const auto trades = gtc(book, 4, Side::Sell, 100, 10);
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 1u);
    EXPECT_EQ(trades[1].getBidTrade()._orderId, 3u);
}

TEST(Cancel, UnknownOrAlreadyFilledIdIsANoOp) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Sell, 100, 5); // fills 1 and 2
    gtc(book, 3, Side::Buy, 99, 5);
    book.CancelOrder(1);
    book.CancelOrder(42);
    book.CancelOrder(3);
    book.CancelOrder(3);
    EXPECT_EQ(book.Size(), 0u);
    EXPECT_TRUE(book.getOrderInfos().getBids().empty());
}

// ---------------------------------------------------------------- modify

TEST(Modify, LosesTimePriority) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 100, 5);
    book.ModifyOrder(OrderModify{ 1, Side::Buy, 100, 5 });
    const auto trades = gtc(book, 3, Side::Sell, 100, 5);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getBidTrade()._orderId, 2u);
}

TEST(Modify, NewPriceCanCrossAndTrade) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 99, 5);
    gtc(book, 2, Side::Sell, 101, 5);
    const auto trades = book.ModifyOrder(OrderModify{ 1, Side::Buy, 101, 5 });
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].getAskTrade()._orderId, 2u);
    EXPECT_EQ(book.Size(), 0u);
}

TEST(Modify, ChangesQuantityAndKeepsOrderType) {
    Orderbook book;
    book.addOrder(makeOrder(OrderType::GoodForDay, 1, Side::Buy, 100, 5));
    book.ModifyOrder(OrderModify{ 1, Side::Buy, 98, 9 });
    EXPECT_EQ(prices(book.getOrderInfos().getBids()), (std::vector<Price>{ 98 }));
    EXPECT_EQ(quantities(book.getOrderInfos().getBids()), (std::vector<Quantity>{ 9 }));
    book.EndOfDay();
    EXPECT_EQ(book.Size(), 0u) << "a modified Good-For-Day order is still Good-For-Day";
}

TEST(Modify, UnknownIdIsANoOp) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    EXPECT_TRUE(book.ModifyOrder(OrderModify{ 7, Side::Sell, 90, 5 }).empty());
    EXPECT_EQ(book.Size(), 1u);
}

// ---------------------------------------------------------------- randomised contract check

// Seeded random flow of every order type plus cancels and modifies. After every operation the
// book must be uncrossed and sorted best-first; each trade must cross; an aggressor must take
// levels best-first; FOK must execute exactly when enough quantity is reachable; FAK never rests.
TEST(RandomFlow, ContractHoldsAfterEveryOperation) {
    Orderbook book;
    std::mt19937_64 rng(20240427);
    std::uniform_int_distribution<int> pick(0, 99), offset(-5, 5), qty(1, 20);
    std::vector<OrderId> ids;
    OrderId next = 1;
    const Price mid = 1000;

    for (int step = 0; step < 20000; ++step) {
        const int r = pick(rng);
        const Side side = (pick(rng) < 50) ? Side::Buy : Side::Sell;
        const Price price = mid + offset(rng);
        const Quantity q = static_cast<Quantity>(qty(rng));
        Trades trades;

        if (r < 15 && !ids.empty()) {
            const auto i = std::uniform_int_distribution<std::size_t>(0, ids.size() - 1)(rng);
            book.CancelOrder(ids[i]);
            ids[i] = ids.back();
            ids.pop_back();
        } else if (r < 20 && !ids.empty()) {
            const auto id = ids[std::uniform_int_distribution<std::size_t>(0, ids.size() - 1)(rng)];
            trades = book.ModifyOrder(OrderModify{ id, side, price, q });
        } else if (r < 30) {
            const Quantity before = reachable(book, side, price);
            const OrderId id = next++;
            trades = book.addOrder(makeOrder(OrderType::FillOrKill, id, side, price, q));
            Quantity filled = 0;
            for (const auto& t : trades) filled += t.getBidTrade()._quantity;
            if (before >= q) EXPECT_EQ(filled, q) << "FOK rejected with " << before << " reachable, step " << step;
            else EXPECT_EQ(filled, 0u) << "FOK executed without enough quantity, step " << step;
        } else if (r < 40) {
            const OrderId id = next++;
            const std::size_t sizeBefore = book.Size();
            trades = book.addOrder(makeOrder(OrderType::FillAndKill, id, side, price, q));
            EXPECT_LE(book.Size(), sizeBefore) << "FAK must never rest, step " << step;
        } else if (r < 45) {
            trades = market(book, next++, side, std::min<Quantity>(q, 5));
        } else {
            const OrderId id = next++;
            const auto type = (r < 55) ? OrderType::GoodForDay : OrderType::GoodTillCancel;
            trades = book.addOrder(makeOrder(type, id, side, price, q));
            ids.push_back(id);
        }

        for (std::size_t i = 0; i < trades.size(); ++i) {
            const auto& t = trades[i];
            EXPECT_GE(t.getBidTrade()._price, t.getAskTrade()._price) << "trade without a cross, step " << step;
            EXPECT_GT(t.getBidTrade()._quantity, 0u);
            if (i > 0 && side == Side::Buy) EXPECT_GE(t.getAskTrade()._price, trades[i - 1].getAskTrade()._price);
            if (i > 0 && side == Side::Sell) EXPECT_LE(t.getBidTrade()._price, trades[i - 1].getBidTrade()._price);
        }
        expectUncrossedAndSorted(book);
        if (::testing::Test::HasFailure()) FAIL() << "first failure at step " << step;
    }
}

// ---------------------------------------------------------------- lifecycle

// The first Good-For-Day order starts the pruning thread. Destroying a book must not wait for the
// next 16:00. Before the fix the shutdown flag could be set and notified between the pruner's
// check and its wait (a lost wake-up), leaving the destructor blocked in join(). The varying
// delay sweeps the destructor across the pruner's start-up so that window is hit.
TEST(Lifecycle, DestroyingManyBooksDoesNotHang) {
    for (OrderId i = 0; i < 2000; ++i) {
        Orderbook book;
        book.addOrder(makeOrder(OrderType::GoodForDay, i, Side::Buy, 100, 1));
        const auto until = std::chrono::steady_clock::now() + std::chrono::microseconds(i % 200);
        while (std::chrono::steady_clock::now() < until) {}
    }
    SUCCEED();
}
