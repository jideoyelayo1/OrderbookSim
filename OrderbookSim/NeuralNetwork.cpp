#include "NeuralNetwork.h"




void NeuralNetwork::loadTogology(const std::vector<unsigned>& topology) {
    myNet.loadTopology(topology);
}

void NeuralNetwork::Run(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals) {
    trainingPass++;


    myNet.feedForward(inputVals);

    myNet.getResults(resultVals);


    //assert(targetVals.size() == topology.back()); // TODO: CHECK THIS

    myNet.backProp(targetVals);

    //std::cout << "Not recent average error: " << myNet.getRecentAverageError() << "\n";
    //allData += "Not recent average error: " + std::to_string(myNet.getRecentAverageError()) + "\n";
}

std::vector<double> NeuralNetwork::RunWithReturn(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals) {
    Run(inputVals, targetVals, resultVals);
    return resultVals;
}