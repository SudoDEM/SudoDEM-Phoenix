// functions defined in the respective .cpp files
void expose_matrices(pybind11::module_& m);
void expose_vectors(pybind11::module_& m);
void expose_boxes(pybind11::module_& m);
void expose_quaternion(pybind11::module_& m);
void expose_complex(pybind11::module_& m); // does nothing if _COMPLEX_SUPPORT is not #defined
void expose_converters(pybind11::module_& m);
