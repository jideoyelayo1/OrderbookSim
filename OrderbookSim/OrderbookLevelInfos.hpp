#pragma once

#include "LevelInfo.hpp"
#


class OrderbookLevelInfos {
private:
    LevelInfos _bids; LevelInfos _asks;
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : _bids{ bids }, _asks{ asks } {} // setter in c++

    const LevelInfos& getBids() const;
    const LevelInfos& getAsks() const;
};
