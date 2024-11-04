/*!
 * @file OrderbookLevelInfos.cpp
 * @brief Implements the OrderbookLevelInfos class, providing access to bid and ask levels.
 *
 * This file contains the implementation of the OrderbookLevelInfos class, which
 * provides methods to retrieve the bid and ask levels within the orderbook.
 */

#include "OrderbookLevelInfos.hpp"

/*!
 * @brief Retrieves the bid levels in the orderbook.
 *
 * @return A constant reference to the bid LevelInfos.
 */
const LevelInfos& OrderbookLevelInfos::getBids() const {
    return _bids;
}

/*!
 * @brief Retrieves the ask levels in the orderbook.
 *
 * @return A constant reference to the ask LevelInfos.
 */
const LevelInfos& OrderbookLevelInfos::getAsks() const {
    return _asks;
}
