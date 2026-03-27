CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(minieigen CXX)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/Zc:preprocessor)
  add_compile_options(/bigobj)
  add_compile_options(/utf-8)
else(MSVC)
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O2 -fPIC")

endif()

# 1. Find the active Python
if(WIN32)
  set(Python_EXECUTABLE "${CONDA_PREFIX_CMAKE}/python.exe" CACHE FILEPATH "Python executable" FORCE)
else()
  set(Python_EXECUTABLE "${CONDA_PREFIX_CMAKE}/bin/python" CACHE FILEPATH "Python executable" FORCE)
endif()
find_package(Python 3.13 REQUIRED COMPONENTS Interpreter Development Development.Module)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/pybind11" "${CMAKE_CURRENT_BINARY_DIR}/pybind11")
endif()

file(TO_CMAKE_PATH "$ENV{CONDA_PREFIX}" CONDA_PREFIX_CMAKE)
list(PREPEND CMAKE_PREFIX_PATH "${CONDA_PREFIX_CMAKE}")
MESSAGE(STATUS "Miniconda env: ${CONDA_PREFIX_CMAKE}")


# 2. Set pybind11 path (header-only library)
SET(PYBIND11_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/pybind11/include" CACHE PATH "Path to pybind11")

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cMake")

# Compiler Optimization Flags
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
IF (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ftrack-macro-expansion=0 -save-temps")
ENDIF()

IF ("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ftemplate-depth-512")
ENDIF()

ADD_DEFINITIONS("-DEIGEN_DONT_VECTORIZE -DEIGEN_DONT_ALIGN -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")

# RPATH Settings
SET(CMAKE_INSTALL_RPATH "@loader_path;")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# 3. Set Eigen3 paths (allow override from command line)
if(NOT EIGEN3_INCLUDE_DIR)
  set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/eigen-3.3.5" CACHE PATH "Path to Eigen3")
endif()

# 4. Output configurations for debugging
MESSAGE(STATUS "Found Python Header: ${Python_INCLUDE_DIRS}")
MESSAGE(STATUS "Found pybind11: ${PYBIND11_INCLUDE_DIR}")
MESSAGE(STATUS "Found Eigen: ${EIGEN3_INCLUDE_DIR}")

# 5. Include directories and build
INCLUDE_DIRECTORIES(${Python_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PYBIND11_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})


ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/src")
