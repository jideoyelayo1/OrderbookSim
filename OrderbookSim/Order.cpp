#include "Order.hpp"

OrderId Order::getOrderId() const { return _id; }
OrderType Order::getOrderType() const { return _type; }
Side Order::getSide() const { return _side; }
Price Order::getPrice() const { return _price; }
Quantity Order::getInitialQty() const { return _initialQty; } // may not be used 
Quantity Order::getQty() const { return _quantity; }
Quantity Order::getRemainingQty() const { return _remainingQty; }
Quantity Order::getFilledQty() const { return _initialQty - _remainingQty; }
bool Order::isFilled() const { return getRemainingQty() == 0; }
void Order::Fill(Quantity qty) {
    if (qty > getRemainingQty()) {
        throw std::logic_error(std::format("Order ({}) cannot be filled for more than remaining qty ({})", getOrderId(), getRemainingQty()));
    }
    _remainingQty -= qty;
}
void Order::toGoodTillCancel(Price price) {
    if (getOrderType() != OrderType::Market) throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", getOrderId()));
    _price = price;
    _type = OrderType::GoodTillCancel;
}