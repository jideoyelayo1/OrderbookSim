cmake_minimum_required(VERSION 3.25)
project(OrderbookSim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Order book, history and neural network; shared by the simulator, tests, benchmark and Python module
set(SOURCES
    OrderDetailHistory.cpp
    OrderModify.cpp
    MatchedOrderDetails.cpp
    Orderbook.cpp
    OrderbookLevelInfos.cpp
    NeuralNetwork.cpp
    Neuron.cpp
    Order.cpp
    OrderDetail.cpp
    Trade.cpp
    TrainingData.cpp
    Net.cpp
)

find_package(Threads REQUIRED)
add_library(orderbook_core STATIC ${SOURCES})
target_include_directories(orderbook_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orderbook_core PUBLIC Threads::Threads)
set_target_properties(orderbook_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(OrderbookSimExec OrderbookSim.cpp)
target_link_libraries(OrderbookSimExec PRIVATE orderbook_core)

# Python extension module; needs the pybind11 submodule checked out
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/CMakeLists.txt)
    add_subdirectory(pybind11)
    pybind11_add_module(orderbook orderbook_pybind.cpp)
    target_link_libraries(orderbook PRIVATE orderbook_core)
endif()

# Tests: GoogleTest is fetched at configure time (pinned by hash)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    URL_HASH SHA256=1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(OrderbookTests
    OrderbookTest/test.cpp
)
target_include_directories(OrderbookTests PRIVATE OrderbookTest)
target_link_libraries(OrderbookTests PRIVATE orderbook_core GTest::gtest_main)
include(GoogleTest)
# test.cpp reads TestFiles/ relative to the working directory; the timeout turns a deadlock into a failure
gtest_discover_tests(OrderbookTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/OrderbookTest
    PROPERTIES TIMEOUT 20
)

