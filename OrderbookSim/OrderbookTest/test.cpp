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

enum class ActionType
{
    Add,
    Cancel,
    Modify,
};

struct Information
{
    ActionType _type;
    OrderType _orderType;
    Side _side;
    Price _price;
    Quantity _quantity;
    OrderId _orderId;
};

using Informations = std::vector<Information>;

struct Result
{
    std::size_t _allCount;
    std::size_t _bidCount;
    std::size_t _askCount;
};

using Results = std::vector<Result>;

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


class OrderbookTestsFixture : public googletest::TestWithParam<const char*>
{
private:
    const static inline std::filesystem::path Root{ std::filesystem::current_path() };
    const static inline std::filesystem::path TestFolder{ "TestFiles" };
public:
    const static inline std::filesystem::path TestFolderPath{ Root / TestFolder };
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

TEST_P(OrderbookTestsFixture, OrderbookTestSuite)
{
    //OrderbookTestsFixture::printPath();
    // Arrange
#pragma region ARRANGE
    const auto file = OrderbookTestsFixture::TestFolderPath / GetParam();

    InputHandler handler;
    const auto [actions, result] = handler.GetInformations(file);

    auto GetOrder = [](const Information& action)
        {
            return std::make_shared<Order>(
                action._orderType,
                action._orderId,
                action._side,
                action._price,
                action._quantity);
        };

    auto GetOrderModify = [](const Information& action)
        {
            return OrderModify
            {
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
    for (const auto& action : actions)
    {
        switch (action._type)
        {
        case ActionType::Add:
        {const Trades& trades = orderbook.addOrder(GetOrder(action));}
        break;
        case ActionType::Modify:
        {const Trades& trades = orderbook.ModifyOrder(GetOrderModify(action));}
        break;
        case ActionType::Cancel:
        {orderbook.CancelOrder(action._orderId);}
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

INSTANTIATE_TEST_CASE_P(Tests, OrderbookTestsFixture, googletest::ValuesIn({
    "Match_GoodTillCancel.txt",
    "Match_FillAndKill.txt",
    "Match_FillOrKill_Hit.txt",
    "Match_FillOrKill_Miss.txt",
    "Cancel_Success.txt",
    "Modify_Side.txt",
    "Match_Market.txt"
    }));

