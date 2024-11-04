/*!
 * @file Trade.hpp
 * @brief Defines the Trade class, representing a completed trade with bid and ask details.
 *
 * The Trade class encapsulates information about a trade, including the details of the bid and ask sides.
 * It also defines a type alias, Trades, for a collection of Trade objects.
 */

#pragma once
#include "TradeInfo.hpp"
#include <vector>

/*!
 * @class Trade
 * @brief Represents a completed trade, containing information about the bid and ask sides.
 *
 * A Trade object contains bid and ask details, encapsulated in TradeInfo objects. It provides
 * methods to access these details, facilitating the retrieval of trade information in a trading system.
 */
class Trade {
private:
    TradeInfo _bidTrade;  /*!< Details of the bid-side trade, including order ID, price, and quantity. */
    TradeInfo _askTrade;  /*!< Details of the ask-side trade, including order ID, price, and quantity. */

public:
    /*!
     * @brief Constructs a Trade object with specified bid and ask trade details.
     * 
     * @param bidTrade The TradeInfo for the bid side of the trade.
     * @param askTrade The TradeInfo for the ask side of the trade.
     */
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : _bidTrade{ bidTrade }, _askTrade{ askTrade }
    {}

    /*!
     * @brief Retrieves the bid-side trade information of this Trade.
     *
     * The TradeInfo returned contains details about the bid side of the trade, including
     * order ID, price, and quantity.
     *
     * @return A constant reference to the TradeInfo associated with the bid side of the trade.
     */
    const TradeInfo& getBidTrade() const;

    /*!
     * @brief Retrieves the ask-side trade information of this Trade.
     *
     * The TradeInfo returned contains details about the ask side of the trade, including
     * order ID, price, and quantity.
     *
     * @return A constant reference to the TradeInfo associated with the ask side of the trade.
     */
    const TradeInfo& getAskTrade() const;
};

/*!
 * @typedef Trades
 * @brief A type alias for a collection of Trade objects.
 *
 * This alias represents a vector of Trade objects, useful for handling multiple trades
 * in a trading system.
 */
using Trades = std::vector<Trade>;
