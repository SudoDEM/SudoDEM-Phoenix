# 1. Define source files
SET(DCSRC
    "double-conversion/bignum.cc"
    "double-conversion/bignum-dtoa.cc"
    "double-conversion/cached-powers.cc"
    "double-conversion/diy-fp.cc"
    "double-conversion/double-conversion.cc"
    "double-conversion/fast-dtoa.cc"
    "double-conversion/fixed-dtoa.cc"
    "double-conversion/strtod.cc"
)

# Explicitly list source files for predictability (preferred over GLOB)
FILE(GLOB_RECURSE SRC_minieigen "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 2. Create the library
# Python modules should typically be 'MODULE' (loadable at runtime)
ADD_LIBRARY(minieigen MODULE ${SRC_minieigen} ${DCSRC})

# 3. Target properties
SET_TARGET_PROPERTIES(minieigen PROPERTIES
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    INSTALL_RPATH "@loader_path;@loader_path/../;"
)


# 4. Link with Python (pybind11 is header-only)
TARGET_LINK_LIBRARIES(minieigen PRIVATE
    pybind11::module
)

# 5. Target Include Directories
TARGET_INCLUDE_DIRECTORIES(minieigen PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${EIGEN3_INCLUDE_DIR}"
    "${PYBIND11_INCLUDE_DIR}"
)

# 6. Install to the directory defined in the top-level script
INSTALL(TARGETS minieigen DESTINATION ${CMAKE_SOURCE_DIR}/lib)
