#include "Net.h"

Net::Net() = default;

double Net::_recentAverageSmoothFactor = 100.0;


void Net::getResults(std::vector<double>& resultVals) const {
    resultVals.clear();
    for (unsigned n = 0; n < _layers.back().size() - 1; ++n) {
        resultVals.push_back(_layers.back()[n].getOutputVal());
    }
}

void Net::backProp(const std::vector<double>& targetVals) {
    // calc overal net err (RMS of output neuron errs)

    Layer& outputLayer = _layers.back();
    _error = 0.0;

    for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
        double delta = targetVals[n] - outputLayer[n].getOutputVal();
        _error += delta * delta;

    }
    _error /= outputLayer.size() - 1;
    _error = sqrt(_error); // RMS

    // Implement a recent average measurement

    _recentAverageError = (_recentAverageError * _recentAverageSmoothFactor + _error) / (_recentAverageSmoothFactor + 1.0);

    // calc output layer gradient

    for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
        outputLayer[n].calcOutputGradients(targetVals[n]);
    }

    // calc grad on hidden layers

    for (unsigned layerNum = _layers.size() - 2; layerNum > 0; --layerNum) {
        Layer& hiddenLayer = _layers[layerNum];
        Layer& nextLayer = _layers[layerNum + 1];

        for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
            hiddenLayer[n].calcHiddenGradients(nextLayer);
        }
    }

    // for all layers from outputs to first hidden layer, update connection weight

    for (unsigned layerNum = _layers.size() - 1; layerNum > 0; --layerNum) {
        Layer& layer = _layers[layerNum];
        Layer& prevLayer = _layers[layerNum - 1];

        for (unsigned n = 0; n < layer.size() - 1; ++n) {
            layer[n].updateInputWeights(prevLayer);
        }
    }


}

void Net::feedForward(const std::vector<double>& inputVals) {

    //assert(inputVals.size() == _layers[0].size() - 1); // TODO: CHECK THIS

    for (unsigned i = 0; i < inputVals.size(); ++i) {
        _layers[0][i].setOutputVal(inputVals[i]);
    }
    // forward propagation
    for (unsigned layerNum = 1; layerNum < _layers.size(); ++layerNum) {
        Layer& prevLayer = _layers[layerNum - 1];
        for (unsigned n = 0; n < _layers[layerNum].size() - 1; ++n) {
            _layers[layerNum][n].feedForward(prevLayer);
        }
    }
}

