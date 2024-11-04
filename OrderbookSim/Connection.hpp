/*!
 * @file Connection.h
 * @brief Defines the Connection structure used in neural network implementations.
 *
 * The Connection struct represents a link between two neurons, holding
 * information about the weight of the connection and the weight's change (delta).
 */

#pragma once

/*!
 * @struct Connection
 * @brief Represents a connection between two neurons in a neural network.
 *
 * Each Connection stores the weight of the link between neurons and the 
 * delta weight, which indicates how much the weight has changed during training.
 */
struct Connection {
    double weight;       ///< The weight of the connection.
    double deltaWeight;  ///< The change in weight during the last update.
};
