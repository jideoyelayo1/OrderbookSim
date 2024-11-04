/*!
 * @file TrainingData.cpp
 * @brief Implements the TrainingData class, which handles reading and parsing training data for neural network training.
 *
 * This file contains methods to extract network topology, input values, and target output values from
 * a training data file.
 */

#include "TrainingData.hpp"
#include <iostream>
#include <sstream>
#include <string>

/*!
 * @brief Reads the topology of the neural network from the training data file.
 *
 * The topology line in the training data file should start with "topology:" followed by
 * the number of neurons in each layer. This method reads the topology information and
 * populates the provided vector.
 *
 * @param[out] topology A vector to store the number of neurons in each layer of the network.
 */
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

/*!
 * @brief Constructs a TrainingData object and opens the specified training data file.
 * 
 * @param filename The path to the training data file.
 */
TrainingData::TrainingData(const std::string filename) {
    _trainingDataFile.open(filename.c_str());
}

/*!
 * @brief Reads the next set of input values from the training data file.
 *
 * This method reads a line from the training file that starts with "in:" followed by
 * input values, and populates the provided vector with those values.
 *
 * @param[out] inputVals A vector to store the input values.
 * @return The number of input values read.
 */
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

/*!
 * @brief Reads the next set of target output values from the training data file.
 *
 * This method reads a line from the training file that starts with "out:" followed by
 * target output values, and populates the provided vector with those values.
 *
 * @param[out] targetOutputsVals A vector to store the target output values.
 * @return The number of target output values read.
 */
unsigned TrainingData::getTargetOutputs(std::vector<double>& targetOutputsVals) {
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
