/*
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "orderbook.h"

namespace py = pybind11;

PYBIND11_MODULE(orderbook, m) {
    orderbook.doc() = "Orderbook Class"; // optional module docstring
    py::class_<Orderbook>(handle, "Orderbook")
        .def(py::init<>())
        .def("addOrder", &Orderbook::addOrder)
        .def("ModifyOrder", &Orderbook::ModifyOrder)
        .def("CancelOrder", &Orderbook::CancelOrder);

}

*/
