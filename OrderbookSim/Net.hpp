#pragma once

#include "Neuron.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

class Net {

public:
    Net();

    void loadTopology(const std::vector<unsigned> topology) {
        unsigned numLayers = topology.size();
        for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum) {
            _layers.push_back(Layer());
            unsigned numOutputs = (layerNum == topology.size() - 1) ? 0 : topology[layerNum + 1];

            for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum) { // one more for bias
                _layers.back().push_back(Neuron(numOutputs, neuronNum)); //
                //std::cout << "made a neuron" << std::endl;
            }
            // force bias to 1.0
            _layers.back().back().setOutputVal(1.0);
        }
    }

    Net(const std::vector<unsigned> topology) {
        unsigned numLayers = topology.size();
        for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum) {
            _layers.push_back(Layer());
            unsigned numOutputs = (layerNum == topology.size() - 1) ? 0 : topology[layerNum + 1];

            for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum) { // one more for bias
                _layers.back().push_back(Neuron(numOutputs, neuronNum)); //
                //std::cout << "made a neuron" << std::endl;
            }
            // force bias to 1.0
            _layers.back().back().setOutputVal(1.0);
        }
    }

    void feedForward(const std::vector<double>& inputVals);
    void backProp(const std::vector<double>& targetVals);
    void getResults(std::vector<double>& resultsVals) const;
    double getRecentAverageError(void) const { return _recentAverageError; }
private:
    std::vector<Layer> _layers; // _layers[layerNum][neuronNum]
    double _error = 0;
    double _recentAverageError = 0;
    static double _recentAverageSmoothFactor;

};
