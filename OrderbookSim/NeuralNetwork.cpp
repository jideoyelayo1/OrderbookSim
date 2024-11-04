/*!
 * @file NeuralNetwork.cpp
 * @brief Implements the NeuralNetwork class for managing and running a neural network.
 *
 * This file contains the implementation of the NeuralNetwork class, including functions
 * for loading the network topology and running the network with forward and backward propagation.
 */

#include "NeuralNetwork.hpp"

/*!
 * @brief Loads the topology of the neural network.
 *
 * Sets up the network structure based on the provided topology, where each element
 * in the vector specifies the number of neurons in each layer.
 *
 * @param topology A vector representing the number of neurons in each layer of the network.
 */
void NeuralNetwork::loadTogology(const std::vector<unsigned>& topology) {
    myNet.loadTopology(topology);
}

/*!
 * @brief Runs a single training pass through the network.
 *
 * This function performs forward propagation with the input values, retrieves the
 * results, and then executes backpropagation with the target values to adjust weights.
 *
 * @param inputVals A vector containing input values for the network.
 * @param targetVals A vector containing target values for the network's output neurons.
 * @param resultVals A vector to store the output values produced by the network.
 */
void NeuralNetwork::Run(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals) {
    trainingPass++;

    // Perform forward propagation
    myNet.feedForward(inputVals);

    // Retrieve the results from the output layer
    myNet.getResults(resultVals);

    // Perform backpropagation to adjust weights
    myNet.backProp(targetVals);
}

/*!
 * @brief Runs the network with input and target values, and returns the result.
 *
 * This function is similar to Run but also returns the result values vector.
 * It performs forward and backward propagation to adjust network weights.
 *
 * @param inputVals A vector containing input values for the network.
 * @param targetVals A vector containing target values for the network's output neurons.
 * @param resultVals A vector to store the output values produced by the network.
 * @return A vector containing the output values from the network after running.
 */
std::vector<double> NeuralNetwork::RunWithReturn(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals) {
    Run(inputVals, targetVals, resultVals);
    return resultVals;
}
