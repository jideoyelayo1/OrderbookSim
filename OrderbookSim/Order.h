#pragma once
#include "OrderType.h"
#include "Side.h"
#include <cstdint>
#include <stdexcept>
#include <format>
#include <list>

#include "Usings.h"
#include "Constants.h"



class Order {
private:
    // Member variables
    OrderType _type;
    OrderId _id;
    Side _side;
    Price _price;
    Quantity _quantity;
    Quantity _initialQty;
    Quantity _remainingQty;
public:
    // Constructor and member functions declarations
    Order(OrderType type, OrderId id, Side side, Price price, Quantity qty)
        : _type{ type }, _id{ id }, _side{ side }, _price{ price }, _quantity{ qty }, _initialQty{ qty }, _remainingQty{ qty } {} // setter in c++
    Order(OrderId orderId, Side side, Quantity quantity)
        : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
    {}

    // getter in c++
    OrderId getOrderId() const;
    OrderType getOrderType() const;
    Side getSide() const;
    Price getPrice() const;
    Quantity getInitialQty() const; // may not be used 
    Quantity getQty() const;
    Quantity getRemainingQty() const;
    Quantity getFilledQty() const;
    bool isFilled() const;
    void Fill(Quantity qty);
    void toGoodTillCancel(Price price);

};

using OrderPtr = std::shared_ptr<Order>;
using OrderPtrs = std::list<OrderPtr>; // a vector is better and show upgrade with vector done

