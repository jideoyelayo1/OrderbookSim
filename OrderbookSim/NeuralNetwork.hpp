/*!
 * @file NeuralNetwork.hpp
 * @brief Defines the NeuralNetwork class for managing and running a neural network.
 *
 * This file contains the NeuralNetwork class, which provides methods for loading
 * the network topology, running the network, and retrieving results from the network.
 */

#pragma once

#include "Net.hpp"
#include "TrainingData.hpp"

/*!
 * @class NeuralNetwork
 * @brief Manages a neural network's structure, data, and training process.
 *
 * The NeuralNetwork class provides functionality to set up the network topology,
 * run the network with input and target values, and retrieve the results. It acts
 * as a high-level manager for the neural network operations.
 */
class NeuralNetwork {
private:
    std::vector<unsigned> topology; ///< Stores the topology of the network.
    Net myNet;                      ///< The network object representing the neural network.
    std::vector<double> inputVals;  ///< Input values for the network.
    std::vector<double> targetVals; ///< Target output values for the network.
    std::vector<double> resultVals; ///< Results produced by the network.
    int trainingPass = 0;           ///< Counter to track the number of training passes.

public:
    /*!
     * @brief Default constructor for the NeuralNetwork class.
     *
     * Initializes an empty NeuralNetwork object.
     */
    NeuralNetwork() = default;

    /*!
     * @brief Loads the topology of the neural network.
     *
     * Sets up the network structure based on the provided topology, where each element
     * in the vector specifies the number of neurons in each layer.
     *
     * @param topology A vector representing the number of neurons in each layer of the network.
     */
    void loadTogology(const std::vector<unsigned>& topology);

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
    void Run(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals);

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
    std::vector<double> RunWithReturn(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals);
};
