/*!
 * @file Neuron.cpp
 * @brief Implements the Neuron class for a neural network.
 *
 * This file contains the implementation of the Neuron class, including functions for
 * forward propagation, backpropagation, and weight updates.
 */

#include "Neuron.hpp"

/*!
 * @brief Learning rate for the network.
 *
 * Controls how fast the network learns. A lower value results in slower learning,
 * while a higher value can make the network learn faster but may cause instability.
 */
double Neuron::eta = 0.15;

/*!
 * @brief Momentum factor for the network.
 *
 * Helps smooth out the weight updates by using information from the previous update.
 * A higher value increases the momentum effect.
 */
double Neuron::alpha = 0.5;

/*!
 * @brief Updates the weights of connections from the previous layer.
 *
 * Adjusts each weight based on the neuron's gradient and learning parameters
 * (eta and alpha).
 *
 * @param prevLayer The layer of neurons from which the current neuron receives inputs.
 */
void Neuron::updateInputWeights(Layer& prevLayer) {
    for (unsigned n = 0; n < prevLayer.size(); ++n) {
        Neuron& neuron = prevLayer[n];
        double prevDeltaWeight = neuron._outputWeights[_myIdx].deltaWeight;

        double currDeltaWeight =
            eta * neuron.getOutputVal() * _gradient
            + alpha * prevDeltaWeight;

        neuron._outputWeights[_myIdx].deltaWeight = currDeltaWeight;
        neuron._outputWeights[_myIdx].weight += currDeltaWeight;
    }
}

/*!
 * @brief Calculates the sum of contributions from the next layer.
 *
 * Computes the sum of weights multiplied by gradients for the neurons in the next layer,
 * which is used to adjust the gradient for the hidden layer neurons.
 *
 * @param nextLayer The layer of neurons to which the current neuron connects.
 * @return The sum of contributions from the next layer.
 */
double Neuron::sumDOW(const Layer& nextLayer) const {
    double sum = 0.0;
    for (unsigned n = 0; n < nextLayer.size() - 1; ++n) {
        sum += _outputWeights[n].weight * nextLayer[n]._gradient;
    }
    return sum;
}

/*!
 * @brief Calculates gradients for hidden layer neurons.
 *
 * Computes the gradient for hidden neurons based on the sum of weights and gradients
 * from the next layer and the derivative of the transfer function.
 *
 * @param nextLayer The next layer of neurons, used to compute the gradient.
 */
void Neuron::calcHiddenGradients(const Layer& nextLayer) {
    double dow = sumDOW(nextLayer);
    _gradient = dow * Neuron::transferFunctionDerivative(_outputVal);
}

/*!
 * @brief Calculates the gradient for output layer neurons.
 *
 * Uses the difference between the target and actual output values, adjusted
 * by the derivative of the transfer function, to set the gradient.
 *
 * @param targetVal The target value for the output neuron.
 */
void Neuron::calcOutputGradients(double targetVal) {
    double delta = targetVal - _outputVal;
    _gradient = delta * Neuron::transferFunctionDerivative(_outputVal);
}

/*!
 * @brief The transfer function for the neuron.
 *
 * Applies the hyperbolic tangent function to limit the neuron's output to the range [-1, 1].
 * This function is used for activation in the network.
 *
 * @param x The input value to the transfer function.
 * @return The output value after applying the transfer function.
 */
double Neuron::transferFunction(double x) {
    return tanh(x);
}

/*!
 * @brief Derivative of the transfer function.
 *
 * Calculates the derivative of the hyperbolic tangent function, which is used
 * during backpropagation to adjust weights.
 *
 * @param x The input value.
 * @return The derivative of the transfer function.
 */
double Neuron::transferFunctionDerivative(double x) {
    return 1 - x * x;
}

/*!
 * @brief Feeds the input values forward through the neuron.
 *
 * Calculates the neuron's output by summing the products of inputs and weights,
 * and then applies the transfer function to the result.
 *
 * @param prevLayer The previous layer of neurons, used as inputs to this neuron.
 */
void Neuron::feedForward(const Layer& prevLayer) {
    double sum = 0.0;

    for (unsigned n = 0; n < prevLayer.size(); ++n) {
        sum += prevLayer[n].getOutputVal() * prevLayer[n]._outputWeights[_myIdx].weight;
    }
    _outputVal = Neuron::transferFunction(sum);
}

/*!
 * @brief Constructor for the Neuron class.
 *
 * Initializes a neuron with a specified number of outputs and index. Each output
 * connection is initialized with a random weight.
 *
 * @param numOutputs The number of connections from this neuron to the next layer.
 * @param myIdx The index of this neuron in its layer.
 */
Neuron::Neuron(unsigned numOutputs, unsigned myIdx) {
    for (unsigned c = 0; c < numOutputs; ++c) {
        _outputWeights.push_back(Connection());
        _outputWeights.back().weight = randomWeight();
    }
    _myIdx = myIdx;
}
