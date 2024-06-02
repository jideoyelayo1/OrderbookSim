#pragma once

#include <numeric>
#include <vector>
#include <cstdint>   // Add this line for int32_t, uint32_t, uint64_t
#include <vector>    // Add this line for std::vector


using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;
using OrderIds = std::vector<OrderId>;