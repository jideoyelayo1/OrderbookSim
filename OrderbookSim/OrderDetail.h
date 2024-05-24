
#pragma once

#include <chrono>

#include "OrderType.h"
#include "Side.h"
#include "Usings.h"
#include "Constants.h"

using timeChrono = std::chrono::system_clock::time_point;

class OrderDetail
{
private:
	// buy or sell
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
    } // setter in c++
    
    time_t getTime() {
        return std::chrono::system_clock::to_time_t(_time);
    }


};

