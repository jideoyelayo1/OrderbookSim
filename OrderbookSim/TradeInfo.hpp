/*!
 * @file TradeInfo.hpp
 * @brief Defines the TradeInfo struct, which holds essential information for a trade.
 *
 * The TradeInfo struct represents the basic details of a trade, including order ID, price, and quantity.
 * This information is used in trading systems to capture the key elements of each transaction.
 */

#pragma once
#include "Usings.hpp"

/*!
 * @struct TradeInfo
 * @brief Holds essential information about a trade, including order ID, price, and quantity.
 *
 * TradeInfo encapsulates the core details of a trade and is typically used within a Trade object
 * to represent either the bid or ask side of a transaction.
 */
struct TradeInfo {
    OrderId _orderId;    /*!< The unique identifier of the order involved in the trade. */
    Price _price;        /*!< The price at which the trade was executed. */
    Quantity _quantity;  /*!< The quantity of assets involved in the trade. */
};
