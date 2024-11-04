/*!
 * @file OrderDetail.cpp
 * @brief Implements the OrderDetail class, providing details of an individual order.
 *
 * This file contains the implementation of the OrderDetail class, which includes
 * methods for retrieving various attributes of an order, such as type, side, price,
 * quantity, and timestamp. It also provides methods to convert these attributes to
 * string representations.
 */

#include "OrderDetail.hpp"
#include <iomanip> // for std::put_time
#include <sstream> // for std::ostringstream

/*!
 * @brief Retrieves the order type as a string.
 *
 * @return A string representing the type of the order.
 */
std::string OrderDetail::getOrderType() const {
    return orderTypeToString(_type);
}

/*!
 * @brief Retrieves the order type as an enum value.
 *
 * @return The OrderType of the order.
 */
OrderType OrderDetail::OrderType() const {
    return _type;
}

/*!
 * @brief Retrieves the unique identifier for the order.
 *
 * @return The OrderId of the order.
 */
OrderId OrderDetail::getOrderId() const {
    return _id;
}

/*!
 * @brief Retrieves the side (buy/sell) of the order as a string.
 *
 * @return A string representing the side of the order.
 */
std::string OrderDetail::getSide() const {
    return sideToString(_side);
}

/*!
 * @brief Retrieves the side (buy/sell) of the order as an enum value.
 *
 * @return The Side of the order.
 */
Side OrderDetail::Side() const {
    return _side;
}

/*!
 * @brief Retrieves the price of the order.
 *
 * @return The Price of the order.
 */
Price OrderDetail::getPrice() const {
    return _price;
}

/*!
 * @brief Retrieves the quantity of the order.
 *
 * @return The Quantity of the order.
 */
Quantity OrderDetail::getQuantity() const {
    return _quantity;
}

/*!
 * @brief Retrieves the timestamp of the order in a string format.
 *
 * @return A string representing the time the order was created, formatted as "YYYY-MM-DD HH:MM:SS".
 */
std::string OrderDetail::getTime() const {
    std::time_t timeT = std::chrono::system_clock::to_time_t(_time);
    std::tm tm;
    localtime_s(&tm, &timeT);  // Use localtime_s for thread safety
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/*!
 * @brief Sets a new quantity for the order.
 *
 * Updates the quantity of the order with a new value. 
 *
 * @param newValue The new quantity to set.
 */
void OrderDetail::setQuantity(Quantity newValue) {
    _quantity = newValue;
    // if (_quantity <= 0) delete this; // would be unsafe
}
