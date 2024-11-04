/*!
 * @file OrderbookLevelInfos.hpp
 * @brief Defines the OrderbookLevelInfos class for managing bid and ask levels in an orderbook.
 *
 * This file contains the OrderbookLevelInfos class, which provides a structure to store
 * and retrieve the bid and ask levels within an orderbook.
 */

#pragma once

#include "LevelInfo.hpp"

/*!
 * @class OrderbookLevelInfos
 * @brief Holds the bid and ask levels for an orderbook.
 *
 * The OrderbookLevelInfos class stores bid and ask levels represented by LevelInfos objects.
 * It provides methods to access these levels, which can be useful for orderbook analysis and
 * data dissemination.
 */
class OrderbookLevelInfos {
private:
    LevelInfos _bids; ///< The bid levels in the orderbook.
    LevelInfos _asks; ///< The ask levels in the orderbook.

public:
    /*!
     * @brief Constructs an OrderbookLevelInfos object with specified bid and ask levels.
     *
     * @param bids The bid levels to initialize.
     * @param asks The ask levels to initialize.
     */
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : _bids{ bids }, _asks{ asks } {}

    /*!
     * @brief Retrieves the bid levels in the orderbook.
     *
     * @return A constant reference to the bid LevelInfos.
     */
    const LevelInfos& getBids() const;

    /*!
     * @brief Retrieves the ask levels in the orderbook.
     *
     * @return A constant reference to the ask LevelInfos.
     */
    const LevelInfos& getAsks() const;
};
