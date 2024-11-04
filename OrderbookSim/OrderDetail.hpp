/*!
 * @file OrderDetail.hpp
 * @brief Defines the OrderDetail class for storing detailed information about individual orders.
 *
 * This file contains the OrderDetail class, which provides attributes and methods
 * for managing and accessing details of a single order. Each order includes information
 * such as type, side, price, quantity, and a timestamp of creation.
 */

#pragma once

#include <chrono>
#include "OrderType.hpp"
#include "Side.hpp"
#include "Usings.hpp"
#include "Constants.hpp"

using timeChrono = std::chrono::system_clock::time_point;

/*!
 * @class OrderDetail
 * @brief Stores details of an individual order in the orderbook.
 *
 * The OrderDetail class contains information about an order, including its type,
 * side (buy/sell), price, quantity, and creation timestamp. It provides functions
 * to retrieve and set these values.
 */
class OrderDetail {
private:
    OrderType _type;       ///< Type of the order (e.g., Good Till Cancel, Fill and Kill).
    OrderId _id;           ///< Unique identifier of the order.
    Side _side;            ///< Side of the order (Buy or Sell).
    Price _price;          ///< Price of the order.
    Quantity _quantity;    ///< Quantity of the order.
    timeChrono _time;      ///< Timestamp when the order was created.

public:
    /*!
     * @brief Constructs an OrderDetail with specified parameters.
     *
     * Initializes an OrderDetail instance with order type, ID, side, price, and quantity.
     * The creation timestamp is set to the current time.
     *
     * @param type The type of the order.
     * @param id The unique identifier of the order.
     * @param side The side of the order (Buy or Sell).
     * @param price The price of the order.
     * @param qty The quantity of the order.
     */
    OrderDetail(OrderType type, OrderId id, Side side, Price price, Quantity qty)
        : _type{ type }, _id{ id }, _side{ side }, _price{ price }, _quantity{ qty } {
        _time = std::chrono::system_clock::now();
    }

    /*!
     * @brief Default constructor.
     */
    OrderDetail() = default;

    /*!
     * @brief Retrieves the order type as a string.
     *
     * @return A string representing the type of the order.
     */
    std::string getOrderType() const;

    /*!
     * @brief Retrieves the order type as an enum value.
     *
     * @return The OrderType of the order.
     */
    OrderType OrderType() const;

    /*!
     * @brief Retrieves the unique identifier of the order.
     *
     * @return The OrderId of the order.
     */
    OrderId getOrderId() const;

    /*!
     * @brief Retrieves the side (buy/sell) of the order as a string.
     *
     * @return A string representing the side of the order.
     */
    std::string getSide() const;

    /*!
     * @brief Retrieves the side (buy/sell) of the order as an enum value.
     *
     * @return The Side of the order.
     */
    Side Side() const;

    /*!
     * @brief Retrieves the price of the order.
     *
     * @return The Price of the order.
     */
    Price getPrice() const;

    /*!
     * @brief Retrieves the quantity of the order.
     *
     * @return The Quantity of the order.
     */
    Quantity getQuantity() const;

    /*!
     * @brief Retrieves the timestamp of the order in a string format.
     *
     * @return A string representing the time the order was created, formatted as "YYYY-MM-DD HH:MM:SS".
     */
    std::string getTime() const;

    /*!
     * @brief Sets a new quantity for the order.
     *
     * Updates the quantity of the order with a new value.
     *
     * @param newValue The new quantity to set.
     */
    void setQuantity(Quantity newValue);
};
