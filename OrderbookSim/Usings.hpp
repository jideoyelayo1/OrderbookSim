/*!
 * @file Usings.hpp
 * @brief Defines type aliases commonly used in the trading system.
 *
 * This file provides convenient type definitions for key data types such as Price, Quantity, and OrderId.
 * These aliases improve code readability and maintainability throughout the codebase.
 */

#pragma once

#include <numeric>
#include <vector>
#include <cstdint>   // Provides fixed-width integer types like int32_t, uint32_t, uint64_t

/*!
 * @typedef Price
 * @brief Represents the price of an order or trade.
 *
 * Defined as a 32-bit signed integer. Price values can be positive or negative.
 */
using Price = std::int32_t;

/*!
 * @typedef Quantity
 * @brief Represents the quantity or volume of assets in an order or trade.
 *
 * Defined as a 32-bit unsigned integer, as quantities cannot be negative.
 */
using Quantity = std::uint32_t;

/*!
 * @typedef OrderId
 * @brief Represents a unique identifier for an order.
 *
 * Defined as a 64-bit unsigned integer, allowing for a large range of unique identifiers.
 */
using OrderId = std::uint64_t;

/*!
 * @typedef OrderIds
 * @brief A collection of unique order identifiers.
 *
 * Represents a vector of OrderId values, useful for managing groups of orders.
 */
using OrderIds = std::vector<OrderId>;
