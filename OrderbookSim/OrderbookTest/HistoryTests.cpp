/*!
 * @file HistoryTests.cpp
 * @brief Tests for the trade history: the neural network's inputs and training.
 *
 * The network has topology {2, 3, 1}: inputs are the time of day and the traded quantity,
 * the target is the trade price, all divided by OrderDetailHistory::PRICESCALE.
 */

#include "pch.h"

#include "../OrderDetailHistory.hpp"

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
