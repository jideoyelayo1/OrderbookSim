#pragma once
#include "TradeInfo.hpp"

#include <vector>

class Trade {
private:
    TradeInfo _bidTrade, _askTrade;
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : _bidTrade{ bidTrade }, _askTrade{ askTrade }
    {}

    const TradeInfo& getBidTrade() const;
    const TradeInfo& getAskTrade() const;

};
using Trades = std::vector<Trade>;
