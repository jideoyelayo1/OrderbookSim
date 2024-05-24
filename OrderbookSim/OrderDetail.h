#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
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

    time_t getTime() const {
        return std::chrono::system_clock::to_time_t(_time);
    }

    // Define how to serialize OrderDetail to JSON
    friend void to_json(nlohmann::json& j, const OrderDetail& o) {
        j = nlohmann::json{
            {"type", o._type},
            {"id", o._id},
            {"side", o._side},
            {"price", o._price},
            {"quantity", o._quantity},
            {"time", o.getTime()}
        };
    }

    // Define how to deserialize OrderDetail from JSON
    friend void from_json(const nlohmann::json& j, OrderDetail& o) {
        o._type = j.at("type").get<OrderType>();
        o._id = j.at("id").get<OrderId>();
        o._side = j.at("side").get<Side>();
        o._price = j.at("price").get<Price>();
        o._quantity = j.at("quantity").get<Quantity>();
        time_t time = j.at("time").get<time_t>();
        o._time = std::chrono::system_clock::from_time_t(time);
    }
};
