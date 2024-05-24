#pragma once

#include <vector>

#include "OrderDetail.h"

class OrderDetailHistory
{
private:
	std::vector<OrderDetail> buyHistory;
	std::vector<OrderDetail> sellHistory;
	void addOrderToBuyHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
		OrderDetail newOrderDetail = OrderDetail(type, id, side, price, qty);
		buyHistory.push_back(newOrderDetail);
	}
	void addOrderToSellHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
		OrderDetail newOrderDetail = OrderDetail(type, id, side, price, qty);
		sellHistory.push_back(newOrderDetail);
	}
public:
	OrderDetailHistory() {};
	void addOrderToHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
		if (side == Side::Buy) addOrderToBuyHistory(type, id, side, price, qty);
		else if (side == Side::Sell) addOrderToSellHistory(type, id, side, price, qty);
	}
};

