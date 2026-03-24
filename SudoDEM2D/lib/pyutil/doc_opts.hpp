#pragma once

// macro to set the same docstring generation options in all modules
// pybind11 uses different approach for docstring options
#define SUDODEM_SET_DOCSTRING_OPTS pybind11::options options; options.disable_function_signatures();

