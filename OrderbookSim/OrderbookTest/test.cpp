/*!
 * @file main_test.cpp
 * @brief Contains unit tests for the Orderbook using Google Test framework.
 *
 * This file defines the ActionType enum, the Information struct, and the InputHandler class,
 * which are used to parse and handle test data, as well as the main test suite for the Orderbook.
 */

#include "pch.h"

#pragma region includes // solves linker issues
#include "../Orderbook.cpp"
#include "../Orderbook.h"
#include "../OrderbookLevelInfos.h"
#include "../OrderbookLevelInfos.cpp"
#include "../Order.h"
#include "../Order.cpp"
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
struct InputHandler {
private:
    /*!
     * @brief Converts a string view to a number.
     * @param str The string view to convert.
     * @return The numeric value.
     * @throws std::logic_error if the value is below zero.
     */
    std::uint32_t ToNumber(const std::string_view& str) const;

    /*!
     * @brief Tries to parse a result line.
     * @param str The string to parse.
     * @param result The result structure to populate.
     * @return True if parsing was successful, false otherwise.
     */
    bool TryParseResult(const std::string_view& str, Result& result) const;

    /*!
     * @brief Tries to parse an information line for an action.
     * @param str The string to parse.
     * @param action The action structure to populate.
     * @return True if parsing was successful, false otherwise.
     */
    bool TryParseInformation(const std::string_view& str, Information& action) const;

    /*!
     * @brief Splits a string view into parts based on a delimiter.
     * @param str The string to split.
     * @param delimiter The delimiter character.
     * @return A vector of string views representing the split parts.
     */
    std::vector<std::string_view> Split(const std::string_view& str, char delimiter) const;

    /*!
     * @brief Parses a side string into a Side enum value.
     * @param str The string to parse.
     * @return The parsed Side value.
     * @throws std::logic_error if the side is unknown.
     */
    Side ParseSide(const std::string_view& str) const;

    /*!
     * @brief Parses an order type string into an OrderType enum value.
     * @param str The string to parse.
     * @return The parsed OrderType value.
     * @throws std::logic_error if the order type is unknown.
     */
    OrderType ParseOrderType(const std::string_view& str) const;

    /*!
     * @brief Parses a price string into a Price value.
     * @param str The string to parse.
     * @return The parsed Price value.
     * @throws std::logic_error if the price is unknown.
     */
    Price ParsePrice(const std::string_view& str) const;

    /*!
     * @brief Parses a quantity string into a Quantity value.
     * @param str The string to parse.
     * @return The parsed Quantity value.
     * @throws std::logic_error if the quantity is unknown.
     */
    Quantity ParseQuantity(const std::string_view& str) const;

    /*!
     * @brief Parses an order ID string into an OrderId value.
     * @param str The string to parse.
     * @return The parsed OrderId value.
     * @throws std::logic_error if the order ID is empty.
     */
    OrderId ParseOrderId(const std::string_view& str) const;

public:
    /*!
     * @brief Retrieves actions and the result from a file.
     * @param path The path to the input file.
     * @return A tuple containing a vector of actions and the result.
     * @throws std::logic_error if no result is specified in the file.
     */
    std::tuple<Informations, Result> GetInformations(const std::filesystem::path& path) const;
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
    void printPath();
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
