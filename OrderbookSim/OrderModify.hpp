/*!
 * @file OrderModify.hpp
 * @brief Defines the OrderModify class, which encapsulates modifications to an order's details.
 *
 * The OrderModify class stores updated information for an order, such as order ID, price, side,
 * and quantity. It also includes a method to convert these modifications into a new order.
 */

#pragma once

#include "Order.hpp"
#include <memory>

/*!
 * @class OrderModify
 * @brief Represents a modification to an existing order.
 *
 * The OrderModify class is used to store and apply modifications to an order.
 * It includes details such as the modified order's ID, price, side (buy/sell),
 * and quantity. The class also provides a method to convert these modifications
 * into a new Order object.
 */
class OrderModify {
private:
    OrderId _orderId;      ///< The unique identifier of the order to be modified.
    Price _price;          ///< The new price for the modified order.
    Side _side;            ///< The side (buy or sell) for the modified order.
    Quantity _quantity;    ///< The new quantity for the modified order.

public:
    /*!
     * @brief Constructs an OrderModify object with the specified parameters.
     * 
     * @param orderId The unique identifier of the order to modify.
     * @param side The side (buy or sell) of the order.
     * @param price The updated price of the order.
     * @param quantity The updated quantity of the order.
     */
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
        : _orderId{ orderId }
        , _price{ price }
        , _side{ side }
        , _quantity{ quantity }
    { }

    /*!
     * @brief Gets the order ID of the modified order.
     * @return The OrderId of the modified order.
     */
    OrderId getOrderId() const { return _orderId; }

    /*!
     * @brief Gets the price of the modified order.
     * @return The updated Price of the modified order.
     */
    Price getPrice() const { return _price; }

    /*!
     * @brief Gets the side (buy or sell) of the modified order.
     * @return The updated Side of the modified order.
     */
    Side getSide() const { return _side; }

    /*!
     * @brief Gets the quantity of the modified order.
     * @return The updated Quantity of the modified order.
     */
    Quantity getQuantity() const { return _quantity; }

    /*!
     * @brief Converts the order modification into a shared pointer to a new Order object.
     * 
     * Creates a new OrderPtr from the modification details using the specified order type.
     *
     * @param type The type of the new order to create (e.g., market, limit).
     * @return A shared pointer (OrderPtr) to a newly created Order with the modification details.
     */
    OrderPtr toOrderPtr(OrderType type) const {
        return std::make_shared<Order>(type, getOrderId(), getSide(), getPrice(), getQuantity());
    }
};
