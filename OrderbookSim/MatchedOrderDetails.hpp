#pragma once

#include <chrono>
#include <sstream>  // For std::ostringstream
#include <iomanip>  // For std::put_time

#include "OrderType.hpp"
#include "Side.hpp"
#include "Usings.hpp"
#include "Constants.hpp"

using timeChrono = std::chrono::system_clock::time_point;



class MatchedOrderDetail
{
private:
	Price _price;
	Quantity _quantity;
	timeChrono _time;

public:
	MatchedOrderDetail(Price price, Quantity qty): _price { price }, _quantity{ qty }{ _time = std::chrono::system_clock::now(); };

    Price getPrice() const { return _price; }
    Quantity getQuantity() const { return _quantity; }
    std::string getTime() const {
        std::time_t timeT = std::chrono::system_clock::to_time_t(_time);
        std::tm tm;
        localtime_s(&tm, &timeT);  // Use localtime_s instead of localtime
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

