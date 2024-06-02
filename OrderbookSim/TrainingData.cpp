#include "TrainingData.hpp"

void TrainingData::getTopology(std::vector<unsigned>& topology) {
    std::string line;
    std::string label;

    if (!getline(_trainingDataFile, line)) {
        // Failed to read a line from the file. Handle the error.
        std::cerr << "Error reading from the file." << std::endl;
        abort();
    }

    std::stringstream ss(line);
    ss >> label;

    if (label != "topology:") {
        // Invalid format. Handle the error.
        std::cerr << "Invalid format. Expected 'topology:'." << std::endl;
        abort();
    }

    unsigned numLayers;
    while (ss >> numLayers) {
        topology.push_back(numLayers);
    }

    if (topology.empty()) {
        // No topology data found. Handle the error.
        std::cerr << "No topology data found." << std::endl;
        abort();
    }
}


TrainingData::TrainingData(const std::string filename) {
    _trainingDataFile.open(filename.c_str());
}

unsigned TrainingData::getNextInputs(std::vector<double>& inputVals) {
    inputVals.clear();
    std::string line;
    std::getline(_trainingDataFile, line);
    std::stringstream ss(line);
    std::string label;
    ss >> label;
    if (label.compare("in:") == 0) {
        double oneVal;
        while (ss >> oneVal) {
            inputVals.push_back(oneVal);
        }
    }
    return inputVals.size();
}

unsigned  TrainingData::getTargetOutputs(std::vector<double>& targetOutputsVals) {
    targetOutputsVals.clear();
    std::string line;
    std::getline(_trainingDataFile, line);
    std::stringstream ss(line);
    std::string label;
    ss >> label;
    if (label.compare("out:") == 0) {
        double oneVal;
        while (ss >> oneVal) {
            targetOutputsVals.push_back(oneVal);
        }
    }
    return targetOutputsVals.size();
}
