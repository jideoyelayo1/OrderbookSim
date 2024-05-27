#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "orderbook.h"

namespace py = pybind11;

PYBIND11_MODULE(orderbook, handle) {
        handle.doc() = "Orderbook"; // optional module docstring
        py::class_<Orderbook>(handle, "Orderbook")
        .def(py::init<>())
        .def("addOrder", &Orderbook::addOrderViaPython)
        .def("loadTioJson", &Orderbook::saveToJson);
}
/*
*/


