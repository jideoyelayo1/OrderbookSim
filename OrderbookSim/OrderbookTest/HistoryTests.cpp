/*!
 * @file HistoryTests.cpp
 * @brief Tests for the trade history: the neural network's inputs and training, and the
 *        Orderbook read-outs (resting orders, prediction, recent error, VWAP).
 *
 * The network has topology {2, 3, 1}: inputs are the time of day and the traded quantity,
 * the target is the trade price, all divided by OrderDetailHistory::PRICESCALE.
 */

#include "pch.h"

#include "../Orderbook.hpp"

#include <cmath>
#include <memory>
#include <random>
#include <vector>

namespace {

// Weights from input neuron 0 (time of day) to each hidden neuron, bias excluded.
std::vector<double> timeInputWeights(const OrderDetailHistory& history) {
    const Net& net = history.getNeuralNetwork().getNet();
    std::vector<double> weights;
    for (unsigned n = 0; n + 1 < net.getLayer(1).size(); ++n) weights.push_back(net.getLayer(0)[0].getOutputWeight(n));
    return weights;
}

} // namespace

TEST(NeuralNetworkInput, TimeOfDayReachesTheNetwork) {
    OrderDetailHistory history;
    history.updateNeutralNetwork(100, 10, 0.5); // noon
    const Layer& inputs = history.getNeuralNetwork().getNet().getLayer(0);
    EXPECT_DOUBLE_EQ(inputs[0].getOutputVal(), 50.0 / history.PRICESCALE);
    EXPECT_DOUBLE_EQ(inputs[1].getOutputVal(), 10.0 / history.PRICESCALE);

    history.updateNeutralNetwork(100, 10, 0.0); // midnight
    EXPECT_DOUBLE_EQ(inputs[0].getOutputVal(), 0.0);
}

TEST(NeuralNetworkInput, TrainingMovesTheTimeInputWeights) {
    OrderDetailHistory history;
    const auto initial = timeInputWeights(history);
    ASSERT_EQ(initial.size(), 3u);

    // A zero input contributes nothing to its weights' updates, which is why the old cast froze them.
    history.updateNeutralNetwork(100, 10, 0.0);
    EXPECT_EQ(timeInputWeights(history), initial);

    history.updateNeutralNetwork(100, 10, 0.5);
    const auto trained = timeInputWeights(history);
    for (std::size_t n = 0; n < initial.size(); ++n) EXPECT_NE(trained[n], initial[n]) << "hidden neuron " << n;
}

namespace {

Trades gtc(Orderbook& book, OrderId id, Side side, Price price, Quantity qty) {
    return book.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, id, side, price, qty));
}

} // namespace

TEST(Readouts, HasOrderOnlyWhileResting) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    gtc(book, 2, Side::Buy, 99, 5);
    EXPECT_TRUE(book.hasOrder(1));
    EXPECT_FALSE(book.hasOrder(3)) << "never added";

    gtc(book, 3, Side::Sell, 100, 5); // fills 1 and 3
    EXPECT_FALSE(book.hasOrder(1));
    EXPECT_FALSE(book.hasOrder(3));

    book.CancelOrder(2);
    EXPECT_FALSE(book.hasOrder(2));
}

TEST(Readouts, ZeroBeforeAnyTrade) {
    Orderbook book;
    gtc(book, 1, Side::Buy, 100, 5);
    EXPECT_EQ(book.getPrediction(), 0);
    EXPECT_EQ(book.getRecentAverageError(), 0.0);
    EXPECT_EQ(book.getVWAP(), 0.0);
}

TEST(Readouts, VwapWeightsTradesByQuantity) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 10);
    gtc(book, 2, Side::Buy, 100, 10);
    EXPECT_EQ(book.getVWAP(), 100.0);
    gtc(book, 3, Side::Sell, 110, 30);
    gtc(book, 4, Side::Buy, 110, 30);
    EXPECT_EQ(book.getVWAP(), 107.0) << "(100 * 10 + 110 * 30) / 40 = 107.5, truncated";
}

// 100 * 50,000,000 passes 2^32; multiplied in 32 bits it wrapped and VWAP came out as 14.
TEST(Readouts, VwapDoesNotWrapAtLargeQuantities) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 50'000'000);
    gtc(book, 2, Side::Buy, 100, 50'000'000);
    EXPECT_EQ(book.getVWAP(), 100.0);
}

// getVWAP() keeps running sums; getVWAP(n) with n = every trade rescans the history.
TEST(Readouts, RunningVwapEqualsARescan) {
    OrderDetailHistory history;
    std::mt19937 rng(20241117);
    std::uniform_int_distribution<Price> price(1, 1'000'000);
    std::uniform_int_distribution<Quantity> qty(1, 1'000'000);
    for (int trades = 1; trades <= 5000; ++trades) {
        history.addOrderToPurchaseHistory(price(rng), qty(rng));
        ASSERT_EQ(history.getVWAP(), history.getVWAP(trades)) << "after " << trades << " trades";
    }
}

TEST(Readouts, PredictionLearnsARepeatedTradePrice) {
    Orderbook book;
    gtc(book, 1, Side::Sell, 100, 1);
    gtc(book, 2, Side::Buy, 100, 1);
    const double firstError = book.getRecentAverageError();
    EXPECT_GT(firstError, 0.0);
    EXPECT_TRUE(std::isfinite(firstError));

    for (OrderId id = 3; id < 2003; id += 2) {
        gtc(book, id, Side::Sell, 100, 1);
        gtc(book, id + 1, Side::Buy, 100, 1);
    }
    EXPECT_NEAR(book.getPrediction(), 100, 2);
    EXPECT_LT(book.getRecentAverageError(), firstError);
}
