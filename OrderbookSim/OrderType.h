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