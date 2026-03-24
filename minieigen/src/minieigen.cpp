// 2009-2012 © Václav Šmilauer <eu@doxos.eu>
// licensed under the Lesser General Public License version 3 (LGPLv3)

/* TODO:
	* Figure out if aligned types can be wrapped	(we failed at this previously; unaligned types force the c++ part not to align, making code perhaps less efficient numerically)
	* Add converters from 1-column MatrixX to VectorX so that matrix eqs work as expected
	* Figure out if integer types are ints or longs
*/


#include"common.hpp"
#include"expose.hpp"
#include"visitors.hpp"

PYBIND11_MODULE(minieigen, m){
	m.doc()="miniEigen is wrapper for a small part of the `Eigen <http://eigen.tuxfamily.org>`_ library. Refer to its documentation for details. All classes in this module support pickling.";

	expose_converters(m); // in expose-converters.cpp

	expose_vectors(m);
	expose_matrices(m); // must come after vectors
	expose_complex(m);
	expose_quaternion(m);
	expose_boxes(m);

	m.def("float2str",&doubleToShortest,pybind11::arg("f")=0,pybind11::arg("pad")=0,"Return the shortest string representation of *f* which will is equal to *f* when converted back to float. This function is only useful in Python prior to 3.0; starting from that version, standard string conversion does just that.");

	#ifdef EIGEN_DONT_ALIGN
		m.attr("vectorize")=false;
	#else
		m.attr("vectorize")=true;
	#endif
}








