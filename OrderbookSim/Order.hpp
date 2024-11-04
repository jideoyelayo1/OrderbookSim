/*!
 * @file Order.hpp
 * @brief Defines the Order class and related types for managing orders in a trading system.
 *
 * This file contains the Order class, which represents a trading order with attributes such
 * as order type, ID, side, price, and quantity. It also provides functionality to manage
 * and update the state of an order.
 */

#pragma once

#include "OrderType.hpp"
#include "Side.hpp"
#include <cstdint>
#include <stdexcept>
#include <format>
#include <list>
#include <memory>

#include "Usings.hpp"
#include "Constants.hpp"

/*!
 * @class Order
 * @brief Represents a trading order with attributes and functionality to manage its state.
 *
 * The Order class stores details of an order such as its type, ID, side (buy/sell),
 * price, and quantities (initial, remaining, and filled). It provides methods for
 * retrieving these details, filling the order, and adjusting its state.
 */
class Order {
private:
    OrderType _type;       ///< Type of the order (e.g., Market, GoodTillCancel).
    OrderId _id;           ///< Unique identifier for the order.
    Side _side;            ///< Side of the order (Buy or Sell).
    Price _price;          ///< Price of the order.
    Quantity _quantity;    ///< Current quantity of the order.
    Quantity _initialQty;  ///< Initial quantity of the order at creation.
    Quantity _remainingQty;///< Remaining quantity of the order that is yet to be filled.

public:
    /*!
     * @brief Constructs an order with specified type, ID, side, price, and quantity.
     *
     * Initializes an order with the specified attributes and sets the initial and
     * remaining quantities to the specified quantity.
     *
     * @param type The type of the order (e.g., Market, GoodTillCancel).
     * @param id The unique identifier for the order.
     * @param side The side of the order (Buy or Sell).
     * @param price The price of the order.
     * @param qty The quantity of the order.
     */
    Order(OrderType type, OrderId id, Side side, Price price, Quantity qty)
        : _type{ type }, _id{ id }, _side{ side }, _price{ price }, _quantity{ qty }, _initialQty{ qty }, _remainingQty{ qty } {}

    /*!
     * @brief Constructs a market order with specified ID, side, and quantity.
     *
     * This constructor initializes a market order with the specified ID, side, and
     * quantity, setting the price to an invalid state.
     *
     * @param orderId The unique identifier for the order.
     * @param side The side of the order (Buy or Sell).
     * @param quantity The quantity of the order.
     */
    Order(OrderId orderId, Side side, Quantity quantity)
        : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity) {}

    /*!
     * @brief Retrieves the unique ID of the order.
     * @return The ID of the order.
     */
    OrderId getOrderId() const;

    /*!
     * @brief Retrieves the type of the order (e.g., Market, GoodTillCancel).
     * @return The type of the order.
     */
    OrderType getOrderType() const;

    /*!
     * @brief Retrieves the side of the order (Buy or Sell).
     * @return The side of the order.
     */
    Side getSide() const;

    /*!
     * @brief Retrieves the price of the order.
     * @return The price of the order.
     */
    Price getPrice() const;

    /*!
     * @brief Retrieves the initial quantity of the order.
     *
     * This represents the original quantity specified when the order was created.
     *
     * @return The initial quantity of the order.
     */
    Quantity getInitialQty() const;

    /*!
     * @brief Retrieves the current quantity of the order.
     *
     * This value may change if the order is partially filled.
     *
     * @return The current quantity of the order.
     */
    Quantity getQty() const;

    /*!
     * @brief Retrieves the remaining quantity of the order.
     *
     * This represents the unfilled portion of the order.
     *
     * @return The remaining quantity of the order.
     */
    Quantity getRemainingQty() const;

    /*!
     * @brief Retrieves the filled quantity of the order.
     *
     * This represents the difference between the initial quantity and the remaining quantity.
     *
     * @return The quantity of the order that has been filled.
     */
    Quantity getFilledQty() const;

    /*!
     * @brief Checks if the order is fully filled.
     * @return True if the order is fully filled, false otherwise.
     */
    bool isFilled() const;

    /*!
     * @brief Fills the order by a specified quantity.
     *
     * Reduces the remaining quantity by the specified amount. If the quantity exceeds
     * the remaining quantity, an exception is thrown.
     *
     * @param qty The quantity by which to fill the order.
     * @throws std::logic_error if the specified quantity exceeds the remaining quantity.
     */
    void Fill(Quantity qty);

    /*!
     * @brief Converts a market order to a Good Till Cancel order with a specified price.
     *
     * Changes the order type to GoodTillCancel and sets the price. Only market orders
     * can be converted; otherwise, an exception is thrown.
     *
     * @param price The price to set for the Good Till Cancel order.
     * @throws std::logic_error if the order is not a market order.
     */
    void toGoodTillCancel(Price price);
};

/*!
 * @typedef OrderPtr
 * @brief A shared pointer to an Order object.
 */
using OrderPtr = std::shared_ptr<Order>;

/*!
 * @typedef OrderPtrs
 * @brief A list of shared pointers to Order objects.
 *
 * Used for managing collections of orders. A vector may be more efficient,
 * and future upgrades could explore vector usage.
 */
using OrderPtrs = std::list<OrderPtr>;
