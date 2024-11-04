/*!
 * @file MatchedOrderDetail.hpp
 * @brief Defines the MatchedOrderDetail class for storing details of matched orders.
 *
 * This file contains the MatchedOrderDetail class, which holds information about a matched
 * order, including its price, quantity, and timestamp. It provides methods to retrieve these details.
 */

#pragma once

#include <chrono>
#include <sstream>  // For std::ostringstream
#include <iomanip>  // For std::put_time

#include "OrderType.hpp"
#include "Side.hpp"
#include "Usings.hpp"
#include "Constants.hpp"

using timeChrono = std::chrono::system_clock::time_point;

/*!
 * @class MatchedOrderDetail
 * @brief Stores details of a matched order, including price, quantity, and time.
 *
 * The MatchedOrderDetail class represents an individual matched order in a trading system.
 * It contains the order's price, quantity, and timestamp, and provides accessors for
 * retrieving each of these details.
 */
class MatchedOrderDetail {
private:
    Price _price;       ///< The price at which the order was matched.
    Quantity _quantity; ///< The quantity of the matched order.
    timeChrono _time;   ///< The timestamp of when the order was matched.

public:
    /*!
     * @brief Constructs a MatchedOrderDetail with specified price and quantity.
     *
     * Initializes the matched order with the given price and quantity, and sets
     * the timestamp to the current system time.
     *
     * @param price The price at which the order was matched.
     * @param qty The quantity of the matched order.
     */
    MatchedOrderDetail(Price price, Quantity qty) : _price{ price }, _quantity{ qty } {
        _time = std::chrono::system_clock::now();
    }

    /*!
     * @brief Retrieves the price of the matched order.
     * @return The price at which the order was matched.
     */
    Price getPrice() const { return _price; }

    /*!
     * @brief Retrieves the quantity of the matched order.
     * @return The quantity of the matched order.
     */
    Quantity getQuantity() const { return _quantity; }

    /*!
     * @brief Retrieves the timestamp of when the order was matched as a formatted string.
     *
     * Converts the internal timestamp to a string in the format "YYYY-MM-DD HH:MM:SS".
     *
     * @return A string representation of the timestamp in local time.
     */
    std::string getTime() const {
        std::time_t timeT = std::chrono::system_clock::to_time_t(_time);
        std::tm tm;
        localtime_s(&tm, &timeT);  // Use localtime_s instead of localtime for thread safety
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};
