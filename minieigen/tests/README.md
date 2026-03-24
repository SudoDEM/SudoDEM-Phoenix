# minieigen Type Converter Tests

This directory contains unit tests for the minieigen type converters, specifically testing the conversion between Python lists/tuples and Eigen Vector types.

## Test Files

- `test_converters.py` - Python unit tests for Vector2 and Vector3 conversions
- `test_converters.cpp` - C++ test module for pybind11 type converters
- `CMakeLists.txt` - Build configuration for C++ tests

## Running Tests

### Python Tests

To run the Python tests:

```bash
cd /Users/sway/Documents/gitrepos/SudoDEM-Phoenix/minieigen/tests
python3 test_converters.py
```

Make sure minieigen is built first:

```bash
cd /Users/sway/Documents/gitrepos/SudoDEM-Phoenix/minieigen/build
make
```

### C++ Tests

To build and run the C++ tests:

```bash
cd /Users/sway/Documents/gitrepos/SudoDEM-Phoenix/minieigen/tests
mkdir -p build && cd build
cmake ..
make
python3 -c "import test_converters; test_converters.test_vector2([1.0, 2.0])"
```

## What is Tested

### Vector2 Conversions
- Creating Vector2 from Python list: `Vector2([1.0, 2.0])`
- Creating Vector2 from Python tuple: `Vector2((3.0, 4.0))`
- Vector operations after conversion (addition, dot product)

### Vector3 Conversions
- Creating Vector3 from Python list: `Vector3([1.0, 2.0, 3.0])`
- Creating Vector3 from Python tuple: `Vector3((4.0, 5.0, 6.0))`
- Vector operations after conversion (cross product)

### Error Handling
- Invalid conversions (wrong size lists) raise appropriate errors

## Implementation Details

The type converters are implemented in `minieigen/src/converters.hpp` using pybind11's custom type caster mechanism:

```cpp
namespace pybind11 { namespace detail {
    template<> struct type_caster<Vector2r> { ... };  // underlying Eigen type
    template<> struct type_caster<Vector3r> { ... };  // underlying Eigen type
}}
```

This allows automatic conversion when:
- Passing Python lists/tuples to C++ functions expecting Vector2/Vector3
- Returning Vector2/Vector3 from C++ to Python (converted to tuples)
