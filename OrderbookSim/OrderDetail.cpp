#include "OrderDetail.hpp"

std::string OrderDetail::getOrderType() const { return orderTypeToString(_type); }
OrderType OrderDetail::OrderType() const { return _type; }
OrderId OrderDetail::getOrderId() const { return _id; }
std::string OrderDetail::getSide() const { return sideToString(_side); }
Side OrderDetail::Side() const { return _side; }
Price OrderDetail::getPrice() const { return _price; }
Quantity OrderDetail::getQuantity() const { return _quantity; }
std::string OrderDetail::getTime() const {
    std::time_t timeT = std::chrono::system_clock::to_time_t(_time);
    std::tm tm;
    localtime_s(&tm, &timeT);  // Use localtime_s instead of localtime
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
void OrderDetail::setQuantity(Quantity newValue) {
    _quantity = newValue;
    // if (_quantity <= 0) delete this; // would be unsafe
}