/*!
 * @file Constants.h
 * @brief Defines constants used across the application.
 *
 * This file provides constant values that can be used to represent specific states,
 * such as an invalid price in financial calculations.
 */

#pragma once

#include <limits>  // For std::numeric_limits

/*!
 * @struct Constants
 * @brief Holds application-wide constants for common values.
 *
 * The Constants struct provides a centralized location for defining important
 * constant values, such as `InvalidPrice`, which can represent an undefined or
 * invalid price in calculations.
 */
struct Constants {
    /*!
     * @brief Represents an invalid price.
     *
     * This constant is used to signify an undefined or invalid price in scenarios
     * where a valid price cannot be provided. It utilizes `quiet_NaN()` to indicate
     * a non-numeric value.
     */
    static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
};
