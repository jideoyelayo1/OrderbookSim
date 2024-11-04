/*!
 * @file pch.h
 * @brief Precompiled header file for commonly used libraries and headers.
 *
 * This header file is used to speed up compilation by precompiling
 * frequently included standard libraries and third-party headers.
 */

#pragma once  ///< Ensures the file is only included once during compilation

/// Google Test framework header for unit testing
#include "gtest/gtest.h"

/// Standard library headers
#include <filesystem>  ///< Classes and functions for filesystem operations
#include <iostream>    ///< Input and output stream operations
#include <fstream>     ///< File input and output stream operations
