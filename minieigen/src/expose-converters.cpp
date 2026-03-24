#include"converters.hpp"

void expose_converters(pybind11::module_& m){
	// pybind11/eigen.h handles automatic conversions for vectors and matrices
	// We only need to handle special cases
	
	// TODO: custom_alignedBoxNr_from_seq is not working with pybind11
	// AlignedBox types are not used in Python code, so commented out for now
	// custom_alignedBoxNr_from_seq<2>(m);
	// custom_alignedBoxNr_from_seq<3>(m);
	custom_Quaternionr_from_axisAngle_or_angleAxis(m);
	
	// Note: pybind11/eigen.h automatically handles:
	// - VectorXr, Vector6r, Vector3r, Vector2r
	// - Vector6i, Vector3i, Vector2i
	// - MatrixXr, Matrix6r, Matrix3r, Matrix2r
	// - Complex variants
}