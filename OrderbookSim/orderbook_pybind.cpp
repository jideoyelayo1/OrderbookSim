#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Orderbook.h"


namespace py = pybind11;

PYBIND11_MODULE(orderbook, handle) {
	handle.doc() = "this is the module docstring";
	py::class_<Orderbook>(handle, "Orderbook")
		.def(py::init<>());

}




