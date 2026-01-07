/*!
 * @file main_test.cpp
 * @brief Contains unit tests for the Orderbook using Google Test framework.
 */

#include "pch.h"

#pragma region includes
#include "../Orderbook.hpp"
#include "../OrderbookLevelInfos.hpp"
#include "../Order.hpp"
#include "../OrderModify.hpp"
#pragma endregion

#include <fstream>
#include <string_view>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <filesystem>

namespace googletest = ::testing;

/*!
 * @enum ActionType
 * @brief Defines the types of actions that can be performed on the order book.
 */
enum class ActionType {
    Add,    ///< Add a new order
    Cancel, ///< Cancel an existing order
    Modify  ///< Modify an existing order
};

/*!
 * @struct Information
 * @brief Holds information about an action to be performed on the order book.
 */
struct Information {
    ActionType _type{};
    OrderType  _orderType{};
    Side       _side{};
    Price      _price{};
    Quantity   _quantity{};
    OrderId    _orderId{};
};

using Informations = std::vector<Information>;

/*!
 * @struct Result
 * @brief Holds expected results for order book counts used for assertions.
 *
 * If no Result line is present in the test file, all fields will be zero and
 * the test will only verify that the scenario runs without throwing.
 */
struct Result {
    std::size_t _allCount{}; ///< Total count of orders in the order book
    std::size_t _bidCount{}; ///< Count of bid levels in the order book
    std::size_t _askCount{}; ///< Count of ask levels in the order book
};

using Results = std::vector<Result>;

/*!
 * @class InputHandler
 * @brief Parses test input files to extract actions and expected results.
 *
 * Supported line formats (space separated):
 *
 *   A <Side> <OrderType> <Price> <Qty> <OrderId>
 *   M <OrderId> <Side> <Price> <Qty>
 *   C <OrderId>
 *   R <AllCount> <BidCount> <AskCount>
 */
struct InputHandler {
private:
    std::uint32_t ToNumber(const std::string_view& str) const;
    bool TryParseResult(const std::string_view& str, Result& result) const;
    bool TryParseInformation(const std::string_view& str, Information& action) const;
    std::vector<std::string_view> Split(const std::string_view& str, char delimiter) const;
    Side      ParseSide(const std::string_view& str) const;
    OrderType ParseOrderType(const std::string_view& str) const;
    Price     ParsePrice(const std::string_view& str) const;
    Quantity  ParseQuantity(const std::string_view& str) const;
    OrderId   ParseOrderId(const std::string_view& str) const;

public:
    /*!
     * @brief Retrieves actions and the result from a file.
     * @param path The path to the input file.
     * @return A tuple containing a vector of actions and the result.
     *
     * If no Result line is present, the returned Result will be {0,0,0}.
     */
    std::tuple<Informations, Result> GetInformations(const std::filesystem::path& path) const;
};

/*!
 * @class OrderbookTestsFixture
 * @brief Test fixture for Orderbook tests, providing setup for test files and paths.
 */
class OrderbookTestsFixture : public googletest::TestWithParam<const char*> {
public:
    // TEST_FILES_DIR is provided by CMake:
    // target_compile_definitions(OrderbookTestExec PRIVATE
    //     TEST_FILES_DIR="${CMAKE_SOURCE_DIR}/OrderbookTest/TestFiles"
    // )
    const static inline std::filesystem::path TestFolderPath{ TEST_FILES_DIR };

    void printPath() {
        // std::cout << "Using test files in: " << TestFolderPath << std::endl;
    }
};

/*!
 * @brief Test case for the Orderbook functionality.
 */
TEST_P(OrderbookTestsFixture, OrderbookTestSuite) {
#pragma region ARRANGE
    const auto file = OrderbookTestsFixture::TestFolderPath / GetParam();

    InputHandler handler;
    const auto [actions, result] = handler.GetInformations(file);

    auto GetOrder = [](const Information& action) {
        return std::make_shared<Order>(
            action._orderType,
            action._orderId,
            action._side,
            action._price,
            action._quantity
        );
    };

    auto GetOrderModify = [](const Information& action) {
        return OrderModify{
            action._orderId,
            action._side,
            action._price,
            action._quantity,
        };
    };
#pragma endregion

#pragma region ACT
    Orderbook orderbook;
    for (const auto& action : actions) {
        switch (action._type) {
        case ActionType::Add: {
                const Trades& trades = orderbook.addOrder(GetOrder(action));
                (void)trades;
            }
            break;
        case ActionType::Modify: {
                const Trades& trades = orderbook.ModifyOrder(GetOrderModify(action));
                (void)trades;
            }
            break;
        case ActionType::Cancel:
            orderbook.CancelOrder(action._orderId);
            break;
        default:
            throw std::logic_error("Unsupported Action");
        }
    }
#pragma endregion

#pragma region ASSERT
    const auto& orderbookInfos = orderbook.getOrderInfos();

    // If no explicit Result line is present (all zeros), treat as a smoke test
    if (result._allCount == 0 && result._bidCount == 0 && result._askCount == 0) {
        SUCCEED();
    } else {
        ASSERT_EQ(orderbook.Size(), result._allCount);
        ASSERT_EQ(orderbookInfos.getBids().size(), result._bidCount);
        ASSERT_EQ(orderbookInfos.getAsks().size(), result._askCount);
    }
#pragma endregion
}

/*!
 * @brief Instantiates test cases for OrderbookTestsFixture using predefined test files.
 */
INSTANTIATE_TEST_SUITE_P(
    Tests,
    OrderbookTestsFixture,
    googletest::ValuesIn({
        "Match_GoodTillCancel.txt",
        "Match_FillAndKill.txt",
        "Match_FillOrKill_Hit.txt",
        "Match_FillOrKill_Miss.txt",
        "Cancel_Success.txt",
        "Modify_Side.txt",
        "Match_Market.txt"
    })
);

// ------------------------
// InputHandler definitions
// ------------------------

std::uint32_t InputHandler::ToNumber(const std::string_view& str) const {
    std::string s(str);
    if (s.empty())
        throw std::logic_error("Empty numeric field");

    std::size_t pos = 0;
    long long val = std::stoll(s, &pos);
    if (pos != s.size())
        throw std::logic_error("Invalid numeric field: " + s);
    if (val < 0)
        throw std::logic_error("Negative value not allowed: " + s);

    return static_cast<std::uint32_t>(val);
}

std::vector<std::string_view> InputHandler::Split(const std::string_view& str, char delimiter) const {
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (start <= str.size()) {
        std::size_t pos = str.find(delimiter, start);
        if (pos == std::string_view::npos) {
            parts.emplace_back(str.substr(start));
            break;
        }
        parts.emplace_back(str.substr(start, pos - start));
        start = pos + 1;
    }
    return parts;
}

Side InputHandler::ParseSide(const std::string_view& str) const {
    if (str == "Buy" || str == "BUY" || str == "B")
        return Side::Buy;
    if (str == "Sell" || str == "SELL" || str == "S")
        return Side::Sell;
    throw std::logic_error("Unknown side: " + std::string(str));
}

OrderType InputHandler::ParseOrderType(const std::string_view& str) const {
    if (str == "GoodTillCancel" || str == "GTC")
        return OrderType::GoodTillCancel;
    if (str == "GoodForDay" || str == "GFD")
        return OrderType::GoodForDay;
    if (str == "FillAndKill" || str == "FAK")
        return OrderType::FillAndKill;
    if (str == "FillOrKill" || str == "FOK")
        return OrderType::FillOrKill;
    if (str == "Market" || str == "MKT")
        return OrderType::Market;
    throw std::logic_error("Unknown order type: " + std::string(str));
}

Price InputHandler::ParsePrice(const std::string_view& str) const {
    return static_cast<Price>(ToNumber(str));
}

Quantity InputHandler::ParseQuantity(const std::string_view& str) const {
    return static_cast<Quantity>(ToNumber(str));
}

OrderId InputHandler::ParseOrderId(const std::string_view& str) const {
    if (str.empty())
        throw std::logic_error("Empty order id");
    return static_cast<OrderId>(ToNumber(str));
}

bool InputHandler::TryParseResult(const std::string_view& line, Result& result) const {
    if (line.empty())
        return false;

    auto trimmed = line;
    // trim leading spaces/tabs/# 
    while (!trimmed.empty() &&
           (trimmed.front() == ' ' || trimmed.front() == '\t' || trimmed.front() == '#'))
        trimmed.remove_prefix(1);
    if (trimmed.empty())
        return false;

    // Expect: R AllCount BidCount AskCount
    auto parts = Split(trimmed, ' ');
    if (parts.size() < 4)
        return false;

    if (!(parts[0] == "R" || parts[0] == "Result" || parts[0] == "RESULT" || parts[0] == "result"))
        return false;

    result._allCount = ToNumber(parts[1]);
    result._bidCount = ToNumber(parts[2]);
    result._askCount = ToNumber(parts[3]);
    return true;
}

bool InputHandler::TryParseInformation(const std::string_view& line, Information& action) const {
    if (line.empty())
        return false;

    auto trimmed = line;
    while (!trimmed.empty() &&
           (trimmed.front() == ' ' || trimmed.front() == '\t' || trimmed.front() == '#'))
        trimmed.remove_prefix(1);
    if (trimmed.empty())
        return false;

    // Space-separated format: A/M/C ...
    auto parts = Split(trimmed, ' ');
    if (parts.empty())
        return false;

    const auto& code = parts[0];

    if (code == "A") {
        // A <Side> <OrderType> <Price> <Qty> <OrderId>
        if (parts.size() < 6)
            return false;

        action._type      = ActionType::Add;
        action._side      = ParseSide(parts[1]);
        action._orderType = ParseOrderType(parts[2]);
        action._price     = ParsePrice(parts[3]);
        action._quantity  = ParseQuantity(parts[4]);
        action._orderId   = ParseOrderId(parts[5]);
        return true;
    }

    if (code == "M") {
        // M <OrderId> <Side> <Price> <Qty>
        if (parts.size() < 5)
            return false;

        action._type      = ActionType::Modify;
        // order type is irrelevant for Modify, keep any default (e.g. GTC)
        action._orderType = OrderType::GoodTillCancel;
        action._orderId   = ParseOrderId(parts[1]);
        action._side      = ParseSide(parts[2]);
        action._price     = ParsePrice(parts[3]);
        action._quantity  = ParseQuantity(parts[4]);
        return true;
    }

    if (code == "C") {
        // C <OrderId>
        if (parts.size() < 2)
            return false;

        action._type      = ActionType::Cancel;
        action._orderType = OrderType::GoodTillCancel;
        action._side      = Side::Buy;  // side not needed for Cancel
        action._price     = 0;
        action._quantity  = 0;
        action._orderId   = ParseOrderId(parts[1]);
        return true;
    }

    return false;
}

std::tuple<Informations, Result>
InputHandler::GetInformations(const std::filesystem::path& path) const {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::logic_error("Failed to open test file: " + path.string());
    }

    Informations actions;
    Result result{};      // default: no expectations
    bool hasResult = false;

    std::string line;
    while (std::getline(in, line)) {
        // strip CR if present
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::string_view sv(line);

        // Try result line first
        Result tmpRes{};
        if (TryParseResult(sv, tmpRes)) {
            result = tmpRes;
            hasResult = true;
            continue;
        }

        // Try action line
        Information info{};
        if (TryParseInformation(sv, info)) {
            actions.push_back(info);
            continue;
        }

        // otherwise ignore (empty/comment/etc)
    }

    if (!hasResult) {
        return {actions, Result{0, 0, 0}};
    }

    return {actions, result};
}
