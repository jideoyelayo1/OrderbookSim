/*!
 * @file LevelInfo.h
 * @brief Defines the LevelInfo structure and related types for managing price levels in an order book.
 *
 * This file contains the LevelInfo struct, which represents a price level with its associated
 * quantity, as well as the LevelInfos alias for managing multiple levels.
 */

#pragma once

#include "Usings.hpp"
#include <vector>

/*!
 * @struct LevelInfo
 * @brief Represents a price level in an order book, including its price and quantity.
 *
 * The LevelInfo struct is used to store information about individual price levels within
 * an order book, including the price of the level and the associated quantity.
 */
struct LevelInfo {
    Price _price;       ///< The price at this level in the order book.
    Quantity _quantity; ///< The quantity available at this price level.
};

/*!
 * @typedef LevelInfos
 * @brief A vector of LevelInfo objects for managing multiple price levels.
 *
 * LevelInfos is used to store a collection of price levels, allowing easy access
 * and manipulation of multiple levels within an order book.
 */
using LevelInfos = std::vector<LevelInfo>;
