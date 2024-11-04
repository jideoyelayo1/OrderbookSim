/*!
 * @file Trade.cpp
 * @brief Implements the Trade class, which represents a completed trade with bid and ask details.
 *
 * This file contains the implementation of the Trade class's member functions, providing
 * access to the bid and ask details of a trade.
 */

#include "Trade.hpp"

/*!
 * @brief Retrieves the bid-side trade information of this Trade.
 *
 * The TradeInfo returned contains details about the bid side of the trade, including
 * order ID, price, and quantity.
 *
 * @return A constant reference to the TradeInfo associated with the bid side of the trade.
 */
const TradeInfo& Trade::getBidTrade() const { 
    return _bidTrade; 
}

/*!
 * @brief Retrieves the ask-side trade information of this Trade.
 *
 * The TradeInfo returned contains details about the ask side of the trade, including
 * order ID, price, and quantity.
 *
 * @return A constant reference to the TradeInfo associated with the ask side of the trade.
 */
const TradeInfo& Trade::getAskTrade() const { 
    return _askTrade; 
}
