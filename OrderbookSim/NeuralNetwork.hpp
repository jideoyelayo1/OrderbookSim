#pragma once

#include "Net.hpp"
#include "TrainingData.hpp"

class NeuralNetwork
{
private:
    std::vector<unsigned> topology;
    Net myNet;
    std::vector<double> inputVals, targetVals, resultVals;
    int trainingPass = 0;

public:
    NeuralNetwork() = default;

    void loadTogology(const std::vector<unsigned>& topology);

    void Run(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals);

    std::vector<double> RunWithReturn(std::vector<double>& inputVals, std::vector<double>& targetVals, std::vector<double>& resultVals);

    

};

