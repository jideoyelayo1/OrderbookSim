/*!
 * @file Side.hpp
 * @brief Defines the Side enum for representing order sides (buy or sell) and utility functions.
 *
 * The Side enum is used to distinguish between buy and sell orders, and includes a utility function
 * for converting the enum to a string representation.
 */

#pragma once
#include <string>

/*!
 * @enum Side
 * @brief Represents the side of an order in a trading system (either buy or sell).
 */
enum class Side {
    Buy,  /*!< Represents a buy order. */
    Sell  /*!< Represents a sell order. */
};

/*!
 * @brief Converts a Side value to its corresponding string representation.
 *
 * This function provides a string representation of each order side for display or logging.
 *
 * @param side The Side value to convert to a string.
 * @return A string representing the side of the order (e.g., "Buy" or "Sell").
 */
inline std::string sideToString(Side side) {
    switch (side) {
    case Side::Buy: return "Buy";
    case Side::Sell: return "Sell";
    default: return "Unknown";
    }
}
