/*!
 * @file Order.cpp
 * @brief Implements the Order class for managing order details in a trading system.
 *
 * This file contains the implementation of the Order class, including functions
 * for retrieving order properties, checking order status, and updating order quantities.
 */

#include "Order.hpp"

/*!
 * @brief Retrieves the unique ID of the order.
 * @return The ID of the order.
 */
OrderId Order::getOrderId() const { return _id; }

/*!
 * @brief Retrieves the type of the order (e.g., Market, GoodTillCancel).
 * @return The type of the order.
 */
OrderType Order::getOrderType() const { return _type; }

/*!
 * @brief Retrieves the side of the order (Buy or Sell).
 * @return The side of the order.
 */
Side Order::getSide() const { return _side; }

/*!
 * @brief Retrieves the price of the order.
 * @return The price of the order.
 */
Price Order::getPrice() const { return _price; }

/*!
 * @brief Retrieves the initial quantity of the order.
 *
 * This represents the original quantity specified when the order was created.
 *
 * @return The initial quantity of the order.
 */
Quantity Order::getInitialQty() const { return _initialQty; }

/*!
 * @brief Retrieves the current quantity of the order.
 *
 * This value may change if the order is partially filled.
 *
 * @return The current quantity of the order.
 */
Quantity Order::getQty() const { return _quantity; }

/*!
 * @brief Retrieves the remaining quantity of the order.
 *
 * This represents the unfilled portion of the order.
 *
 * @return The remaining quantity of the order.
 */
Quantity Order::getRemainingQty() const { return _remainingQty; }

/*!
 * @brief Retrieves the filled quantity of the order.
 *
 * This represents the difference between the initial quantity and the remaining quantity.
 *
 * @return The quantity of the order that has been filled.
 */
Quantity Order::getFilledQty() const { return _initialQty - _remainingQty; }

/*!
 * @brief Checks if the order is fully filled.
 * @return True if the order is fully filled, false otherwise.
 */
bool Order::isFilled() const { return getRemainingQty() == 0; }

/*!
 * @brief Fills the order by a specified quantity.
 *
 * Reduces the remaining quantity by the specified amount. If the quantity exceeds
 * the remaining quantity, an exception is thrown.
 *
 * @param qty The quantity by which to fill the order.
 * @throws std::logic_error if the specified quantity exceeds the remaining quantity.
 */
void Order::Fill(Quantity qty) {
    if (qty > getRemainingQty()) {
        throw std::logic_error(std::format("Order ({}) cannot be filled for more than remaining qty ({})", getOrderId(), getRemainingQty()));
    }
    _remainingQty -= qty;
}

/*!
 * @brief Converts a market order to a Good Till Cancel order with a specified price.
 *
 * Changes the order type to GoodTillCancel and sets the price. Only market orders
 * can be converted; otherwise, an exception is thrown.
 *
 * @param price The price to set for the Good Till Cancel order.
 * @throws std::logic_error if the order is not a market order.
 */
void Order::toGoodTillCancel(Price price) {
    if (getOrderType() != OrderType::Market) {
        throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", getOrderId()));
    }
    _price = price;
    _type = OrderType::GoodTillCancel;
}
