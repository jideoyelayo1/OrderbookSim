#pragma once
#include <string>
enum class OrderType {
    GoodTillCancel,
    FillAndKill,
    Market,
    GoodForDay,
    FillOrKill
};

inline std::string orderTypeToString(OrderType type) {
    switch (type) {
    case OrderType::GoodTillCancel: return "GoodTillCancel";
    case OrderType::FillAndKill: return "FillAndKill";
    case OrderType::Market: return "Market";
    case OrderType::GoodForDay: return "GoodForDay";
    case OrderType::FillOrKill: return "FillOrKill";
    default: return "Unknown";
    }
}


inline OrderType intToOrdertType(int type) {
    switch (type) {
    case 1: return OrderType::GoodTillCancel;
    case 2: return OrderType::FillAndKill;
    case 3: return OrderType::Market;
    case 4: return OrderType::GoodForDay;
    case 5: return OrderType::FillOrKill;
    default: return OrderType::GoodTillCancel;
    }
}