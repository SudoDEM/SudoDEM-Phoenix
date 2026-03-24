#pragma once
#include"common.hpp"

// pybind11/eigen.h provides automatic conversions for most Eigen types
// We only need to handle special cases

// Custom converter for AlignedBox from sequence
template<int Dim>
void custom_alignedBoxNr_from_seq(pybind11::module_& m){
	using AlignedBoxNr = Eigen::AlignedBox<Real, Dim>;
	
	pybind11::implicitly_convertible<pybind11::tuple, AlignedBoxNr>();
}

// Quaternion from axis-angle or angle-axis
void custom_Quaternionr_from_axisAngle_or_angleAxis(pybind11::module_& m){
	// pybind11/eigen.h handles basic conversions
	// We can add additional constructors if needed
}

// No-op for complex types if not defined
#ifndef _COMPLEX_SUPPORT
	inline void expose_complex(pybind11::module_& m){}
#endif