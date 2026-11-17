/*!
 * @file BookTests.cpp
 * @brief Tests for Book (web/Book.hpp), the wrapper behind the JavaScript binding: the status
 *        it reports for each order type, its own ids and its argument checks.
 */

#include "pch.h"

#include "../../web/Book.hpp"

#include <cmath>
#include <vector>

namespace {

std::vector<Price> prices(const LevelInfos& levels) {
    std::vector<Price> out;
    for (const auto& level : levels) out.push_back(level._price);
    return out;
}

void expectOutcome(const BookResult& result, const std::string& status, Quantity restingQuantity, std::size_t trades) {
    EXPECT_EQ(result.status, status);
    EXPECT_EQ(result.restingQuantity, restingQuantity);
    EXPECT_EQ(result.trades.size(), trades);
}

} // namespace

TEST(BookStatus, GoodTillCancelRestsPartlyFillsAndFills) {
    Book book;
    expectOutcome(book.add("GoodTillCancel", "Sell", 100, 5), "resting", 5, 0);
    expectOutcome(book.add("GoodTillCancel", "Buy", 100, 8), "partial", 3, 1);
    expectOutcome(book.add("GoodTillCancel", "Sell", 100, 3), "filled", 0, 1);
    EXPECT_EQ(book.size(), 0u);
}

TEST(BookStatus, GoodForDayRestsUntilEndOfDay) {
    Book book;
    const auto result = book.add("GoodForDay", "Buy", 100, 5);
    expectOutcome(result, "resting", 5, 0);
    expectOutcome(book.add("GoodTillCancel", "Sell", 100, 2), "filled", 0, 1);
    book.endOfDay();
    EXPECT_EQ(book.size(), 0u);
    EXPECT_FALSE(book.cancel(static_cast<double>(result.id)));
}

// A Fill-and-Kill remainder is cancelled: "partial" with nothing resting.
TEST(BookStatus, FillAndKillIsRejectedPartlyFilledOrFilled) {
    Book book;
    expectOutcome(book.add("FillAndKill", "Buy", 100, 5), "rejected", 0, 0);
    book.add("GoodTillCancel", "Sell", 100, 5);
    expectOutcome(book.add("FillAndKill", "Buy", 101, 8), "partial", 0, 1);
    EXPECT_EQ(book.size(), 0u);
    EXPECT_TRUE(book.levels().getBids().empty());
    book.add("GoodTillCancel", "Sell", 100, 5);
    expectOutcome(book.add("FillAndKill", "Buy", 100, 5), "filled", 0, 1);
}

TEST(BookStatus, FillOrKillIsRejectedOrFilled) {
    Book book;
    book.add("GoodTillCancel", "Sell", 100, 5);
    expectOutcome(book.add("FillOrKill", "Buy", 100, 6), "rejected", 0, 0);
    EXPECT_EQ(book.size(), 1u);
    expectOutcome(book.add("FillOrKill", "Buy", 100, 5), "filled", 0, 1);
}

// A Market order larger than the opposite side rests its remainder at the worst opposite price.
TEST(BookStatus, MarketIsRejectedFilledOrRestsItsRemainder) {
    Book book;
    expectOutcome(book.add("Market", "Buy", 0, 5), "rejected", 0, 0);
    book.add("GoodTillCancel", "Sell", 100, 5);
    book.add("GoodTillCancel", "Sell", 101, 5);
    expectOutcome(book.add("Market", "Buy", 0, 3), "filled", 0, 1);
    const auto result = book.add("Market", "Buy", 0, 9);
    expectOutcome(result, "partial", 2, 2);
    EXPECT_EQ(prices(book.levels().getBids()), (std::vector<Price>{ 101 }));
    EXPECT_TRUE(book.cancel(static_cast<double>(result.id)));
}

TEST(BookStatus, ModifyReportsLikeAddAndRejectsUnknownIds) {
    Book book;
    const auto bid = book.add("GoodTillCancel", "Buy", 99, 5);
    book.add("GoodTillCancel", "Sell", 101, 5);
    expectOutcome(book.modify(42, "Buy", 99, 5), "rejected", 0, 0);
    expectOutcome(book.modify(static_cast<double>(bid.id), "Buy", 100, 5), "resting", 5, 0);
    const auto crossed = book.modify(static_cast<double>(bid.id), "Buy", 101, 7);
    expectOutcome(crossed, "partial", 2, 1);
    EXPECT_EQ(crossed.id, bid.id);
}

TEST(BookStatus, TradesNameBothOrders) {
    Book book;
    const auto ask = book.add("GoodTillCancel", "Sell", 100, 5);
    const auto bid = book.add("GoodTillCancel", "Buy", 102, 3);
    ASSERT_EQ(bid.trades.size(), 1u);
    EXPECT_EQ(bid.trades[0].getBidTrade()._orderId, bid.id);
    EXPECT_EQ(bid.trades[0].getBidTrade()._price, 102);
    EXPECT_EQ(bid.trades[0].getAskTrade()._orderId, ask.id);
    EXPECT_EQ(bid.trades[0].getAskTrade()._price, 100);
    EXPECT_EQ(bid.trades[0].getAskTrade()._quantity, 3u);
}

// Orderbook::addOrder(type, side, price, qty) numbers orders Size() + 1, which repeats ids once orders fill.
TEST(BookIds, NeverReusedAfterFills) {
    Book book;
    EXPECT_EQ(book.add("GoodTillCancel", "Sell", 100, 5).id, 1u);
    EXPECT_EQ(book.add("GoodTillCancel", "Buy", 100, 5).id, 2u);
    EXPECT_EQ(book.size(), 0u);
    EXPECT_EQ(book.add("FillOrKill", "Buy", 100, 5).id, 3u) << "a rejected order still uses an id";
    EXPECT_EQ(book.add("GoodTillCancel", "Buy", 100, 5).id, 4u);
}

TEST(BookIds, ResetEmptiesTheBookAndRestartsIds) {
    Book book;
    book.add("GoodTillCancel", "Sell", 100, 5);
    book.add("GoodTillCancel", "Buy", 100, 2);
    book.add("GoodTillCancel", "Buy", 90, 2);
    book.reset();
    EXPECT_EQ(book.size(), 0u);
    EXPECT_TRUE(book.levels().getBids().empty());
    EXPECT_EQ(book.vwap(), 0.0);
    EXPECT_EQ(book.prediction(), 0);
    EXPECT_EQ(book.recentError(), 0.0);
    EXPECT_EQ(book.add("GoodTillCancel", "Buy", 100, 5).id, 1u);
}

TEST(BookArguments, InvalidAddsLeaveTheBookAndIdsAlone) {
    Book book;
    for (const auto& result : {
             book.add("Limit", "Buy", 100, 5),
             book.add("GoodTillCancel", "buy", 100, 5),
             book.add("GoodTillCancel", "Buy", 100, 0),
             book.add("GoodTillCancel", "Buy", 100, 1.5),
             book.add("GoodTillCancel", "Buy", 0, 5),
             book.add("GoodTillCancel", "Buy", -1, 5),
             book.add("GoodTillCancel", "Buy", 100.5, 5),
             book.add("GoodTillCancel", "Buy", std::nan(""), 5),
             book.add("GoodTillCancel", "Buy", 2147483648.0, 5),
             book.add("Market", "Sell", 0, 4294967296.0),
         }) {
        EXPECT_EQ(result.status, "invalid");
        EXPECT_EQ(result.id, 0u);
    }
    EXPECT_EQ(book.size(), 0u);
    EXPECT_EQ(book.add("GoodTillCancel", "Buy", 100, 5).id, 1u);
}

TEST(BookArguments, CancelAndModifyCheckIds) {
    Book book;
    const auto bid = book.add("GoodTillCancel", "Buy", 100, 5);
    EXPECT_FALSE(book.cancel(0));
    EXPECT_FALSE(book.cancel(1.5));
    EXPECT_FALSE(book.cancel(std::nan("")));
    EXPECT_FALSE(book.cancel(2));
    EXPECT_EQ(book.modify(1.5, "Buy", 100, 5).status, "invalid");
    EXPECT_EQ(book.modify(static_cast<double>(bid.id), "Buy", 100, 0).status, "invalid");
    EXPECT_TRUE(book.cancel(static_cast<double>(bid.id)));
    EXPECT_FALSE(book.cancel(static_cast<double>(bid.id)));
}
