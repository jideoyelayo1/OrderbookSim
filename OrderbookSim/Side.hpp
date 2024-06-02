#pragma once
#include <string>
enum class Side {
    Buy,
    Sell
};

inline std::string sideToString(Side side) {
    switch (side) {
    case Side::Buy: return "Buy";
    case Side::Sell: return "Sell";
    default: return "Unknown";
    }
}