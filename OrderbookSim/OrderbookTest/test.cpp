/*!
 * @file main_test.cpp
 * @brief Contains unit tests for the Orderbook using Google Test framework.
 *
 * This file defines the ActionType enum, the Information struct, and the InputHandler class,
 * which are used to parse and handle test data, as well as the main test suite for the Orderbook.
 */

#include "pch.h"

#pragma region includes
#include "../Orderbook.hpp"
#include "../OrderbookLevelInfos.hpp"
#include "../Order.hpp"
#include "../OrderModify.hpp"
#pragma endregion

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
    ActionType _type;    ///< The type of action (Add, Cancel, Modify)
    OrderType _orderType;///< The type of order (e.g., GoodTillCancel, Market)
    Side _side;          ///< The side of the order (Buy or Sell)
    Price _price;        ///< The price of the order
    Quantity _quantity;  ///< The quantity of the order
    OrderId _orderId;    ///< The unique ID of the order
};

using Informations = std::vector<Information>;

/*!
 * @struct Result
 * @brief Holds expected results for order book counts used for assertions.
 */
struct Result {
    std::size_t _allCount; ///< Total count of orders in the order book
    std::size_t _bidCount; ///< Count of bid orders in the order book
    std::size_t _askCount; ///< Count of ask orders in the order book
};

using Results = std::vector<Result>;

/*!
 * @class InputHandler
 * @brief Parses test input files to extract actions and expected results.
 *
 * The InputHandler class provides functionality for parsing actions and results from
 * test files, which are used to drive tests for the Orderbook functionality.
 */
struct InputHandler
{
private:
    std::uint32_t ToNumber(const std::string_view& str) const
    {
        std::int64_t value{};
        std::from_chars(str.data(), str.data() + str.size(), value);
        if (value < 0)
            throw std::logic_error("Value is below zero.");
        return static_cast<std::uint32_t>(value);
    }

    bool TryParseResult(const std::string_view& str, Result& result) const
    {
        if (str.at(0) != 'R')
            return false;

        auto values = Split(str, ' ');
        result._allCount = ToNumber(values[1]);
        result._bidCount = ToNumber(values[2]);
        result._askCount = ToNumber(values[3]);

        return true;
    }

    bool TryParseInformation(const std::string_view& str, Information& action) const
    {
        auto value = str.at(0);
        auto values = Split(str, ' ');
        if (value == 'A')
        {
            action._type = ActionType::Add;
            action._side = ParseSide(values[1]);
            action._orderType = ParseOrderType(values[2]);
            action._price = ParsePrice(values[3]);
            action._quantity = ParseQuantity(values[4]);
            action._orderId = ParseOrderId(values[5]);
        }
        else if (value == 'M')
        {
            action._type = ActionType::Modify;
            action._orderId = ParseOrderId(values[1]);
            action._side = ParseSide(values[2]);
            action._price = ParsePrice(values[3]);
            action._quantity = ParseQuantity(values[4]);
        }
        else if (value == 'C')
        {
            action._type = ActionType::Cancel;
            action._orderId = ParseOrderId(values[1]);
        }
        else return false;

        return true;
    }

    std::vector<std::string_view> Split(const std::string_view& str, char delimeter) const
    {
        std::vector<std::string_view> columns;
        columns.reserve(5);
        std::size_t start_index{}, end_index{};
        while ((end_index = str.find(delimeter, start_index)) && end_index != std::string::npos)
        {
            auto distance = end_index - start_index;
            auto column = str.substr(start_index, distance);
            start_index = end_index + 1;
            columns.push_back(column);
        }
        columns.push_back(str.substr(start_index));
        return columns;
    }

    Side ParseSide(const std::string_view& str) const
    {
        if (str == "B")
            return Side::Buy;
        else if (str == "S")
            return Side::Sell;
        else throw std::logic_error("Unknown Side");
    }

    OrderType ParseOrderType(const std::string_view& str) const
    {
        if (str == "FillAndKill")
            return OrderType::FillAndKill;
        else if (str == "GoodTillCancel")
            return OrderType::GoodTillCancel;
        else if (str == "GoodForDay")
            return OrderType::GoodForDay;
        else if (str == "FillOrKill")
            return OrderType::FillOrKill;
        else if (str == "Market")
            return OrderType::Market;
        else throw std::logic_error("Unknown OrderType");
    }

    Price ParsePrice(const std::string_view& str) const
    {
        if (str.empty())
            throw std::logic_error("Unknown Price");

        return ToNumber(str);
    }

    Quantity ParseQuantity(const std::string_view& str) const
    {
        if (str.empty())
            throw std::logic_error("Unknown Quantity");

        return ToNumber(str);
    }

    OrderId ParseOrderId(const std::string_view& str) const
    {
        if (str.empty())
            throw std::logic_error("Empty OrderId");

        return static_cast<OrderId>(ToNumber(str));
    }

public:
    std::tuple<Informations, Result> GetInformations(const std::filesystem::path& path) const
    {
        Informations actions;
        actions.reserve(1'000);

        std::string line;
        std::ifstream file{ path };
        while (std::getline(file, line))
        {
            if (line.empty())
                break;

            const bool isResult = line.at(0) == 'R';
            const bool isAction = !isResult;

            if (isAction)
            {
                Information action;

                auto isValid = TryParseInformation(line, action);
                if (!isValid)
                    continue;

                actions.push_back(action);
            }
            else
            {
                if (!file.eof())
                    throw std::logic_error("Result should only be specified at the end.");

                Result result;

                auto isValid = TryParseResult(line, result);
                if (!isValid)
                    continue;

                return { actions, result };
            }

        }

        throw std::logic_error("No result specified.");
    }
};

/*!
 * @class OrderbookTestsFixture
 * @brief Test fixture for Orderbook tests, providing setup for test files and paths.
 */
class OrderbookTestsFixture : public googletest::TestWithParam<const char*> {
private:
    const static inline std::filesystem::path Root{ std::filesystem::current_path() }; ///< Root directory path
    const static inline std::filesystem::path TestFolder{ "TestFiles" }; ///< Test files folder

public:
    const static inline std::filesystem::path TestFolderPath{ Root / TestFolder }; ///< Full path to test files folder

    /*!
     * @brief Prints the contents of the TestFolderPath directory.
     */
    void printPath() {
        std::cout << "THIS DIRECTORY" << std::endl;
        if (std::filesystem::exists(TestFolderPath) && std::filesystem::is_directory(TestFolderPath)) {
            // Iterate over the directory and print each entry
            for (const auto& entry : std::filesystem::directory_iterator(TestFolderPath)) {
                std::cout << entry.path() << std::endl;
            }
        }
        else {
            std::cout << "Directory does not exist or is not a directory." << std::endl;
        }
    }
};

/*!
 * @brief Test case for the Orderbook functionality.
 * 
 * This test case uses parameterized inputs to verify various Orderbook behaviors,
 * including adding, modifying, and canceling orders.
 */
TEST_P(OrderbookTestsFixture, OrderbookTestSuite) {
    // Arrange
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
            action._quantity);
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

    // Act
#pragma region ACT
    Orderbook orderbook;
    for (const auto& action : actions) {
        switch (action._type) {
        case ActionType::Add:
            { const Trades& trades = orderbook.addOrder(GetOrder(action)); }
            break;
        case ActionType::Modify:
            { const Trades& trades = orderbook.ModifyOrder(GetOrderModify(action)); }
            break;
        case ActionType::Cancel:
            { orderbook.CancelOrder(action._orderId); }
            break;
        default:
            throw std::logic_error("Unsupported Action");
        }
    }
#pragma endregion

    // Assert
#pragma region ASSERT
    const auto& orderbookInfos = orderbook.getOrderInfos();
    ASSERT_EQ(orderbook.Size(), result._allCount);
    ASSERT_EQ(orderbookInfos.getBids().size(), result._bidCount);
    ASSERT_EQ(orderbookInfos.getAsks().size(), result._askCount);
#pragma endregion
}

/*!
 * @brief Instantiates test cases for OrderbookTestsFixture using predefined test files.
 */
INSTANTIATE_TEST_CASE_P(Tests, OrderbookTestsFixture, googletest::ValuesIn({
    "Match_GoodTillCancel.txt",
    "Match_FillAndKill.txt",
    "Match_FillOrKill_Hit.txt",
    "Match_FillOrKill_Miss.txt",
    "Cancel_Success.txt",
    "Modify_Side.txt",
    "Match_Market.txt"
}));
