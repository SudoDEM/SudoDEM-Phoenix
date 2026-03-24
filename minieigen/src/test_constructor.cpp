#include <Eigen/Dense>
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

namespace py = pybind11;

PYBIND11_MODULE(test, m) {
    // Test with eigen.h but using lambda-based constructor
    py::class_<Eigen::Vector3d>(m, "Vector3", "Vector3 class")
        .def(py::init([](double x, double y, double z) {
            Eigen::Vector3d v;
            v[0] = x; v[1] = y; v[2] = z;
            return v;
        }), py::arg("x"), py::arg("y"), py::arg("z"))
        .def_property_readonly("x", [](const Eigen::Vector3d& v) { return v[0]; })
        .def_property_readonly("y", [](const Eigen::Vector3d& v) { return v[1]; })
        .def_property_readonly("z", [](const Eigen::Vector3d& v) { return v[2]; })
        .def("__repr__", [](const Eigen::Vector3d& v) {
            return "[" + std::to_string(v[0]) + " " + std::to_string(v[1]) + " " + std::to_string(v[2]) + "]";
        });
}
