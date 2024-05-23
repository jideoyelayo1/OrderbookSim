#include "OrderModify.h"
#include "Order.h"

/*

OrderId OrderModify::getOrderId() const { return _id; }
Side OrderModify::getSide() const { return _side; }
Price OrderModify::getPrice() const { return _price; }
Quantity OrderModify::getQty() const { return _qty; }

OrderPtr OrderModify::toOrderPtr(OrderType type) const {
	return std::make_shared<Order>(type, getOrderId(), getSide(), getPrice(), getQty());
}
*/