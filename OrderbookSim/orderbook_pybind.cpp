/*!
 * @file orderbook_module.cpp
 * @brief Pybind11 bindings for the Orderbook class to be used in Python.
 *
 * This file contains the Pybind11 module definition for exposing the Orderbook class
 * to Python, allowing Python code to interact with and manipulate Orderbook objects.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Orderbook.hpp"

namespace py = pybind11;

/*!
 * @brief Defines the Pybind11 module for the Orderbook class.
 *
 * This module allows Python to interact with the Orderbook class, providing
 * methods to add orders and save the order book to JSON.
 *
 * @param orderbook The name of the module in Python.
 * @param handle The module handle that is used to add classes and functions.
 */
PYBIND11_MODULE(orderbook, handle) {
    handle.doc() = "Orderbook"; ///< Optional module docstring.

    py::class_<Orderbook>(handle, "Orderbook")
        .def(py::init<>()) ///< Default constructor for Orderbook.
        .def("addOrder", &Orderbook::addOrderViaPython) ///< Adds an order to the order book.
        .def("loadToJson", &Orderbook::saveToJson); ///< Saves the order book to a JSON file.
}
