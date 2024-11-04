/*!
 * @file OrderModify.cpp
 * @brief Implements the OrderModify class, which provides a way to modify orders and convert modifications into new orders.
 *
 * This file contains the implementation of the OrderModify class functions, including getters for order details
 * and a method to create a new OrderPtr from the modification.
 */

#include "OrderModify.hpp"
#include "Order.hpp"

/*

OrderId OrderModify::getOrderId() const { return _id; }
Side OrderModify::getSide() const { return _side; }
Price OrderModify::getPrice() const { return _price; }
Quantity OrderModify::getQty() const { return _qty; }

OrderPtr OrderModify::toOrderPtr(OrderType type) const {
	return std::make_shared<Order>(type, getOrderId(), getSide(), getPrice(), getQty());
}
*/