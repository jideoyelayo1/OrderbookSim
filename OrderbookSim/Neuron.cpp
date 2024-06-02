#include "Neuron.hpp"

double Neuron::eta = 0.15;
double Neuron::alpha = 0.5;

void Neuron::updateInputWeights(Layer& prevLayer) {
    // The weights to be updated

    for (unsigned n = 0; n < prevLayer.size(); ++n) {
        Neuron& neuron = prevLayer[n];
        double prevDeltaWeight = neuron._outputWeights[_myIdx].deltaWeight;

        double currDeltaWeight =
            eta // eta -> overall net learning rate: 0.0 slow learner < 0.2 medium learner < 1.0 reckless learner;
            * neuron.getOutputVal()
            * _gradient
            + alpha // momentum  0.0 -> no momentum : 0.5 -> moderate momentum
            * prevDeltaWeight;
        neuron._outputWeights[_myIdx].deltaWeight = currDeltaWeight;
        neuron._outputWeights[_myIdx].weight += currDeltaWeight;

    }
}
double Neuron::sumDOW(const Layer& nextlayer) const {
    double sum = 0.0;
    // Sum our contributiions of the errors at nodes we feed
    for (unsigned n = 0; n < nextlayer.size() - 1; ++n) {
        sum += _outputWeights[n].weight * nextlayer[n]._gradient;
    }
    return sum;
}
void Neuron::calcHiddenGradients(const Layer& nextLayer) {
    double dow = sumDOW(nextLayer);
    _gradient = dow * Neuron::transferFunctionDerivative(_outputVal);
}
void Neuron::calcOutputGradients(double targetVal) {
    double delta = targetVal - _outputVal;
    _gradient = delta * Neuron::transferFunctionDerivative(_outputVal);
}
double Neuron::transferFunction(double x) {
    // using a tanh x: could use a step, ramp,sigmond, etc
    // needs t be range [-1 ti 1]
    return tanh(x);
}
double Neuron::transferFunctionDerivative(double x) {
    // using a tanh x: could use a step, ramp,sigmond, etc
    // needs t be range [-1 ti 1]
    return 1 - x * x;
}
void Neuron::feedForward(const Layer& prevLayer) {
    double sum = 0.0;

    for (unsigned n = 0; n < prevLayer.size(); ++n) {
        sum += prevLayer[n].getOutputVal() *
            prevLayer[n]._outputWeights[_myIdx].weight;
    }
    _outputVal = Neuron::transferFunction(sum);
}

Neuron::Neuron(unsigned numOutputs, unsigned myIdx) {
    for (unsigned c = 0; c < numOutputs; ++c) {
        _outputWeights.push_back(Connection());
        _outputWeights.back().weight = randomWeight();
    }
    _myIdx = myIdx;
}
