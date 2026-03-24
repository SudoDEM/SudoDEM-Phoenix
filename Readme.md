# SudoDEM

A Discrete Element Code for Non-spherical Particles

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

## Overview

**SudoDEM** is a specialized open-source Discrete Element Method (DEM) code designed for modeling non-spherical particles. It inherits the basic framework from [YADE](https://yade-dem.org) and extends it to support various convex particle shapes.

### Key Features

- **2D Particles**: Disks, super-ellipses
- **3D Particles**: Polyhedrons, super-ellipsoids, poly-superellipsoids, cylinders, cones
- **Clump Technique**: Construct concave shapes by grouping convex particles
- **GPU Acceleration**: CUDA support for high-performance computing
- **Python Scripting**: Flexible simulation control via Python interface
- **OpenMP Parallelization**: Multi-threaded execution support

## Project Structure

```
SudoDEM-Phoenix/
├── SudoDEM2D/          # 2D DEM simulation engine
├── SudoDEM3D/          # 3D DEM simulation engine
├── examples/           # Example simulation scripts
│   ├── SudoDEM2D/     # 2D examples
│   └── SudoDEM3D/     # 3D examples
├── minieigen/          # Linear algebra library bindings
├── thirdparty/         # Third-party dependencies
│   ├── cereal/        # Serialization library
│   ├── eigen-3.3.5/   # C++ template library for linear algebra
│   └── pybind11/      # Python bindings for C++
└── sudodemcfg.h       # Version configuration
```

## Requirements

### Prerequisites

- **CMake**: 3.10+ (3.16+ for SudoDEM3D)
- **Python**: 3.13+
- **C++ Compiler**: C++17 compatible (GCC, Clang, or MSVC)
- **OpenMP**: For parallelization
- **CUDA**: (Optional) For GPU acceleration

### Dependencies

The following dependencies are included in the `thirdparty` directory:
- [Eigen](http://eigen.tuxfamily.org/) - Linear algebra library
- [pybind11](https://github.com/pybind/pybind11) - Python bindings
- [cereal](https://github.com/USCiLab/cereal) - Serialization library

## Building

### SudoDEM2D

```bash
cd SudoDEM2D
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### SudoDEM3D

```bash
cd SudoDEM3D
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Build Options

- `DEBUG=ON`: Enable debug build with verbose output
- `ENABLE_CUDA=ON`: Enable CUDA support (SudoDEM3D)
- `USE_QT5=ON`: Enable Qt5 GUI support

## Examples

### 2D Examples (`examples/SudoDEM2D/`)

| Script | Description |
|--------|-------------|
| `packingEllipses.py` | Ellipse packing simulation |
| `periBiax_disk.py` | Biaxial test with disks |
| `periConsol_disk.py` | Consolidation test with disks |
| `periConsol_ell.py` | Consolidation test with ellipses |
| `periSimpleShear_disk.py` | Simple shear test with disks |
| `testBC.py` | Boundary condition tests |

### 3D Examples (`examples/SudoDEM3D/`)

| Script | Description |
|--------|-------------|
| `cubesliding.py` | Cube sliding simulation |
| `example0.py` | Basic usage example |
| `example1.py` | Particle generation |
| `example2_consol.py` | Consolidation test |
| `example2_shear.py` | Shear test |
| `example3.py` | Advanced features |
| `example4.py` | Complex simulation |

### Running Examples

```bash
# 2D example
python3 examples/SudoDEM2D/packingEllipses.py

# 3D example
python3 examples/SudoDEM3D/example0.py
```

## Documentation

- **Quick Guide**: See `QuickGuideToSudoDEM.pdf` for detailed usage instructions
- **Website**: https://sudodem.github.io
- **Forum**: https://sudoforum.discourse.group

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Citation

If you use SudoDEM in your research, please cite:

```
SudoDEM: A Discrete Element Code for Non-spherical Particles
https://sudodem.github.io
```

## Authors

- **Shiwei Zhao** - Hong Kong University of Science and Technology, Wuhan University
- **Hao Chen** - Hong Kong University of Science and Technology
## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.0 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Acknowledgments

- Built upon the [YADE](https://yade-dem.org) framework
- Thanks to all contributors and users of the SudoDEM community