#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

class TrainingData {
public:
    TrainingData(const std::string filename);
    bool isEof(void) { return _trainingDataFile.eof(); }
    void getTopology(std::vector<unsigned>& topology);
    unsigned getNextInputs(std::vector<double>& inputVals);
    unsigned getTargetOutputs(std::vector<double>& targetOutputsVals);

private:
    std::ifstream _trainingDataFile;
};
