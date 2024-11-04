/*!
 * @file Net.hpp
 * @brief Defines the Net class for a neural network.
 *
 * This file contains the Net class, which represents a neural network with functionalities
 * for forward propagation, backpropagation, and managing network topology.
 */

#pragma once

#include "Neuron.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

/*!
 * @class Net
 * @brief Represents a neural network composed of layers of neurons.
 *
 * The Net class manages the structure and operations of a neural network, including
 * setting up the network topology, performing forward propagation, and executing backpropagation
 * to adjust weights based on errors.
 */
class Net {
public:
    /*!
     * @brief Default constructor for the Net class.
     *
     * Initializes the Net object.
     */
    Net();

    /*!
     * @brief Constructs the network and sets up the topology.
     *
     * Initializes the network based on the provided topology, creating neurons
     * for each layer with the specified number of outputs.
     *
     * @param topology A vector representing the number of neurons in each layer.
     */
    Net(const std::vector<unsigned> topology);

    /*!
     * @brief Sets up the network topology.
     *
     * Loads the network structure based on the provided topology vector. Each entry
     * in the vector represents the number of neurons in a layer, with additional bias neurons added.
     *
     * @param topology A vector representing the number of neurons in each layer.
     */
    void loadTopology(const std::vector<unsigned> topology);

    /*!
     * @brief Performs forward propagation through the network.
     *
     * Sets the input values and calculates the outputs of each neuron in subsequent layers.
     *
     * @param inputVals A vector containing the input values for the network.
     */
    void feedForward(const std::vector<double>& inputVals);

    /*!
     * @brief Executes backpropagation to adjust weights based on target values.
     *
     * Calculates the error at the output layer and propagates gradients backward to
     * adjust weights for each layer in the network.
     *
     * @param targetVals A vector containing the target values for the network's output neurons.
     */
    void backProp(const std::vector<double>& targetVals);

    /*!
     * @brief Retrieves the output values from the final layer of the network.
     *
     * Clears the provided vector and fills it with the network's output values.
     *
     * @param resultVals A reference to a vector where output values will be stored.
     */
    void getResults(std::vector<double>& resultVals) const;

    /*!
     * @brief Gets the recent average error of the network.
     *
     * The recent average error is calculated during backpropagation to track
     * the network's performance over time.
     *
     * @return The recent average error.
     */
    double getRecentAverageError() const { return _recentAverageError; }

private:
    std::vector<Layer> _layers; ///< Layers of the network, where each layer contains neurons.
    double _error = 0; ///< The root mean square error of the network on the most recent iteration.
    double _recentAverageError = 0; ///< The recent average error of the network.
    static double _recentAverageSmoothFactor; ///< Smooth factor for recent average error calculation.
};
