/*!
 * @file Neuron.hpp
 * @brief Defines the Neuron class for a neural network.
 *
 * This file contains the Neuron class, which represents an individual neuron in a
 * neural network. The Neuron class manages the neuron's value, gradients, weights,
 * and provides methods for forward propagation, gradient calculation, and weight updates.
 */

#pragma once

#include <vector>
#include <random>
#include "Connection.hpp"

/*!
 * @typedef Layer
 * @brief Represents a layer of neurons in the neural network.
 */
typedef std::vector<Neuron> Layer;

/*!
 * @class Neuron
 * @brief Represents a single neuron within a neural network layer.
 *
 * The Neuron class handles the neuron's output value, weights for connections to
 * subsequent neurons, and methods for updating weights and calculating gradients.
 */
class Neuron {
public:
    /*!
     * @brief Constructs a Neuron with a specified number of outputs.
     *
     * Initializes a neuron with a certain number of connections (outputs) and an
     * index within its layer.
     *
     * @param numOutputs The number of connections from this neuron to the next layer.
     * @param myIdx The index of this neuron in its layer.
     */
    Neuron(unsigned numOutputs, unsigned myIdx);

    /*!
     * @brief Sets the neuron's output value.
     * 
     * @param val The output value to be set.
     */
    void setOutputVal(double val) { _outputVal = val; }

    /*!
     * @brief Retrieves the neuron's output value.
     *
     * @return The output value of the neuron.
     */
    double getOutputVal() const { return _outputVal; }

    /*!
     * @brief Feeds the input values forward through the neuron.
     *
     * Calculates the neuron's output by summing the products of inputs and weights,
     * then applies the transfer function.
     *
     * @param prevLayer The previous layer of neurons, used as inputs to this neuron.
     */
    void feedForward(const Layer& prevLayer);

    /*!
     * @brief Calculates the gradient for output layer neurons.
     *
     * Uses the difference between the target and actual output values, adjusted
     * by the derivative of the transfer function, to set the gradient.
     *
     * @param targetVal The target value for the output neuron.
     */
    void calcOutputGradients(double targetVal);

    /*!
     * @brief Calculates gradients for hidden layer neurons.
     *
     * Computes the gradient for hidden neurons based on the sum of weights and gradients
     * from the next layer and the derivative of the transfer function.
     *
     * @param nextLayer The next layer of neurons, used to compute the gradient.
     */
    void calcHiddenGradients(const Layer& nextLayer);

    /*!
     * @brief Updates the weights of connections from the previous layer.
     *
     * Adjusts each weight based on the neuron's gradient and learning parameters
     * (eta and alpha).
     *
     * @param prevLayer The layer of neurons from which the current neuron receives inputs.
     */
    void updateInputWeights(Layer& prevLayer);

private:
    static double eta;  ///< Learning rate for weight updates.
    static double alpha; ///< Momentum factor for weight updates.
    
    /*!
     * @brief Generates a random weight for initializing connections.
     *
     * @return A random weight between 0 and 1.
     */
    static double randomWeight() { return rand() / double(RAND_MAX); }

    double _outputVal; ///< The output value of the neuron.
    std::vector<Connection> _outputWeights; ///< Weights for each connection to the next layer.
    unsigned _myIdx; ///< Index of the neuron within its layer.
    double _gradient; ///< The gradient for the neuron, used in backpropagation.

    /*!
     * @brief Activation function used in the network.
     *
     * Applies the hyperbolic tangent function to limit the neuron's output to the range [-1, 1].
     *
     * @param x The input value to the transfer function.
     * @return The output value after applying the transfer function.
     */
    static double transferFunction(double x);

    /*!
     * @brief Derivative of the activation function.
     *
     * Calculates the derivative of the hyperbolic tangent function, used during
     * backpropagation to adjust weights.
     *
     * @param x The input value.
     * @return The derivative of the transfer function.
     */
    static double transferFunctionDerivative(double x);

    /*!
     * @brief Calculates the sum of contributions from the next layer.
     *
     * Computes the sum of weights multiplied by gradients for the neurons in the next layer,
     * which is used to adjust the gradient for the hidden layer neurons.
     *
     * @param nextLayer The layer of neurons to which the current neuron connects.
     * @return The sum of contributions from the next layer.
     */
    double sumDOW(const Layer& nextLayer) const;
};
