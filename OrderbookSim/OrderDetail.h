#pragma once

#include <chrono>
//#include <nlohmann/json.hpp>
#include "OrderType.h"
#include "Side.h"
#include "Usings.h"
#include "Constants.h"

using timeChrono = std::chrono::system_clock::time_point;

class OrderDetail {
private:
    OrderType _type;
    OrderId _id;
    Side _side;
    Price _price;
    Quantity _quantity;
    timeChrono _time;

public:

    OrderDetail(OrderType type, OrderId id, Side side, Price price, Quantity qty)
        : _type{ type }, _id{ id }, _side{ side }, _price{ price }, _quantity{ qty } {
        _time = std::chrono::system_clock::now();
    }

    OrderDetail() = default;

    std::string getOrderType() const;
    OrderType OrderType() const;
    OrderId getOrderId() const;
    std::string getSide() const;
    Side Side() const;
    Price getPrice() const;
    Quantity getQuantity() const;
    std::string getTime() const;

    void setQuantity(Quantity newValue);


};
