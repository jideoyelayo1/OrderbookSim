/*!
 * @file Net.cpp
 * @brief Implements the Net class for a neural network.
 *
 * This file contains the implementation of the Net class, including functions for
 * forward propagation, backpropagation, and calculating results.
 */

#include "Net.hpp"

/*!
 * @brief Default constructor for the Net class.
 *
 * Initializes the Net object.
 */
Net::Net() = default;

/*!
 * @brief Smooth factor for calculating recent average error.
 *
 * This factor is used in the backpropagation process to update the recent
 * average error of the network.
 */
double Net::_recentAverageSmoothFactor = 100.0;

/*!
 * @brief Retrieves the output values of the network.
 *
 * This function clears the resultVals vector and populates it with the output
 * values from the last layer of neurons.
 *
 * @param resultVals A reference to a vector where output values will be stored.
 */
void Net::getResults(std::vector<double>& resultVals) const {
    resultVals.clear();
    for (unsigned n = 0; n < _layers.back().size() - 1; ++n) {
        resultVals.push_back(_layers.back()[n].getOutputVal());
    }
}

/*!
 * @brief Implements backpropagation to adjust weights based on target values.
 *
 * This function calculates the overall network error (Root Mean Square) and
 * adjusts the weights based on gradients calculated from output and hidden layers.
 *
 * @param targetVals A vector containing the target values for output neurons.
 */
void Net::backProp(const std::vector<double>& targetVals) {
    // Calculate overall net error (RMS of output neuron errors)
    Layer& outputLayer = _layers.back();
    _error = 0.0;

    for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
        double delta = targetVals[n] - outputLayer[n].getOutputVal();
        _error += delta * delta;
    }
    _error /= outputLayer.size() - 1;
    _error = sqrt(_error); // Root Mean Square error

    // Update recent average error measurement
    _recentAverageError = (_recentAverageError * _recentAverageSmoothFactor + _error) / (_recentAverageSmoothFactor + 1.0);

    // Calculate gradients for output layer
    for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
        outputLayer[n].calcOutputGradients(targetVals[n]);
    }

    // Calculate gradients for hidden layers
    for (unsigned layerNum = _layers.size() - 2; layerNum > 0; --layerNum) {
        Layer& hiddenLayer = _layers[layerNum];
        Layer& nextLayer = _layers[layerNum + 1];

        for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
            hiddenLayer[n].calcHiddenGradients(nextLayer);
        }
    }

    // Update connection weights from output to first hidden layer
    for (unsigned layerNum = _layers.size() - 1; layerNum > 0; --layerNum) {
        Layer& layer = _layers[layerNum];
        Layer& prevLayer = _layers[layerNum - 1];

        for (unsigned n = 0; n < layer.size() - 1; ++n) {
            layer[n].updateInputWeights(prevLayer);
        }
    }
}

/*!
 * @brief Feeds input values forward through the network.
 *
 * This function sets the input values to the input layer and performs forward
 * propagation through each layer, calculating the outputs of each neuron.
 *
 * @param inputVals A vector containing input values for the network.
 */
void Net::feedForward(const std::vector<double>& inputVals) {
    // Set the output values of the input neurons
    for (unsigned i = 0; i < inputVals.size(); ++i) {
        _layers[0][i].setOutputVal(inputVals[i]);
    }

    // Forward propagation through the layers
    for (unsigned layerNum = 1; layerNum < _layers.size(); ++layerNum) {
        Layer& prevLayer = _layers[layerNum - 1];
        for (unsigned n = 0; n < _layers[layerNum].size() - 1; ++n) {
            _layers[layerNum][n].feedForward(prevLayer);
        }
    }
}
