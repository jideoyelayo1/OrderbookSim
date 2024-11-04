/*!
 * @file OrderType.hpp
 * @brief Defines the OrderType enum and related utility functions for handling different order types.
 *
 * The OrderType enum represents various types of orders in a trading system, and utility functions are
 * provided to convert between the enum and string representations or integer values.
 */

#pragma once
#include <string>

/*!
 * @enum OrderType
 * @brief Specifies the different types of orders that can be used in a trading system.
 *
 * Each type of order has specific rules for how it should be handled in the order book.
 */
enum class OrderType {
    GoodTillCancel,   /*!< Stays active unless filled or explicitly canceled by the trader. */
    FillAndKill,      /*!< Attempts to fill as much as possible immediately and cancels the remaining part. */
    Market,           /*!< Executes at the best available current price. */
    GoodForDay,       /*!< Remains active until the end of the trading day, then cancels automatically if unfilled. */
    FillOrKill        /*!< Either fully fills or cancels, with no partial filling allowed. */
};

/*!
 * @brief Converts an OrderType to its corresponding string representation.
 *
 * This function provides a string representation of each order type for easier display and logging.
 *
 * @param type The OrderType to convert to a string.
 * @return A string representing the order type (e.g., "GoodTillCancel", "Market").
 */
inline std::string orderTypeToString(OrderType type) {
    switch (type) {
    case OrderType::GoodTillCancel: return "GoodTillCancel"; 
    case OrderType::FillAndKill: return "FillAndKill";
    case OrderType::Market: return "Market";
    case OrderType::GoodForDay: return "GoodForDay";
    case OrderType::FillOrKill: return "FillOrKill";
    default: return "Unknown";
    }
}

/*!
 * @brief Converts an integer value to the corresponding OrderType.
 *
 * This function maps integer values to OrderType values, allowing for easy conversions from user inputs
 * or data sources where order types are represented by integers.
 *
 * @param type An integer representing the order type.
 * @return The corresponding OrderType. If the integer does not match any order type, defaults to GoodTillCancel.
 */
inline OrderType intToOrdertType(int type) {
    switch (type) {
    case 1: return OrderType::GoodTillCancel;
    case 2: return OrderType::FillAndKill;
    case 3: return OrderType::Market;
    case 4: return OrderType::GoodForDay;
    case 5: return OrderType::FillOrKill;
    default: return OrderType::GoodTillCancel;
    }
}
