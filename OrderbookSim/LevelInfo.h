#pragma once

#include "Usings.h"
#include <vector>

struct LevelInfo {
    Price _price;
    Quantity _quantity;
};

using LevelInfos = std::vector<LevelInfo>;
