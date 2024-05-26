#pragma once

#include <vector>
#include <random>
#include "Connection.h"


class Neuron;

typedef std::vector<Neuron> Layer;

class Neuron
{
public:
    Neuron(unsigned numOutputs, unsigned myIdx);
    void setOutputVal(double val) { _outputVal = val; }
    //double getOutputVal(void) { return _outputVal; }
    double getOutputVal(void) const { return _outputVal; }
    void feedForward(const Layer& prevLayer);
    void calcOutputGradients(double targetVal);
    void calcHiddenGradients(const Layer& nextLayer);
    void updateInputWeights(Layer& prevLayer);
private:
    static double eta; // [0.0..1.0]
    static double alpha; // [0.0..1.0]
    static double randomWeight(void) { return rand() / double(RAND_MAX); }
    double _outputVal;
    std::vector<Connection> _outputWeights;
    unsigned _myIdx;
    static double transferFunction(double x);
    static double transferFunctionDerivative(double x);
    double _gradient;
    double sumDOW(const Layer& nextlayer) const;

};

