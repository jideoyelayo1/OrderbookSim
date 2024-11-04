/*!
 * @file TrainingData.hpp
 * @brief Declares the TrainingData class, which is responsible for reading and parsing training data from a file.
 *
 * The TrainingData class provides methods to extract neural network topology, input values, and target output values
 * from a structured training data file for training neural networks.
 */

#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

/*!
 * @class TrainingData
 * @brief Manages reading and parsing of training data for neural network training.
 *
 * This class provides methods to read topology information, input values, and target output values
 * from a structured file. It assumes the file follows a specific format.
 */
class TrainingData {
public:
    /*!
     * @brief Constructs a TrainingData object and opens the specified training data file.
     * 
     * @param filename The path to the training data file.
     */
    TrainingData(const std::string filename);

    /*!
     * @brief Checks if the end of the training data file has been reached.
     * 
     * @return True if end of file is reached, otherwise false.
     */
    bool isEof(void) { return _trainingDataFile.eof(); }

    /*!
     * @brief Reads the network topology from the training data file.
     * 
     * The topology is represented by a line starting with "topology:" followed by integers indicating
     * the number of neurons in each layer.
     *
     * @param[out] topology A vector to store the number of neurons in each layer.
     */
    void getTopology(std::vector<unsigned>& topology);

    /*!
     * @brief Reads the next set of input values from the training data file.
     * 
     * The input line in the file should start with "in:" followed by a sequence of input values.
     *
     * @param[out] inputVals A vector to store the input values.
     * @return The number of input values read.
     */
    unsigned getNextInputs(std::vector<double>& inputVals);

    /*!
     * @brief Reads the next set of target output values from the training data file.
     * 
     * The target output line in the file should start with "out:" followed by a sequence of target values.
     *
     * @param[out] targetOutputsVals A vector to store the target output values.
     * @return The number of target output values read.
     */
    unsigned getTargetOutputs(std::vector<double>& targetOutputsVals);

private:
    std::ifstream _trainingDataFile; /*!< Input file stream for reading training data. */
};
