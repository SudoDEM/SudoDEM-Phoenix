#include<sudodem/lib/smoothing/WeightedAverage2d.hpp>
#include<pybind11/pybind11.h>
#include<pybind11/stl.h>

/* Tell whether point is inside polygon
 *
 * See _utils.cpp: pointInsidePolygon for docs and license.
 */
bool pyGaussAverage::pointInsidePolygon(const Vector2r& pt, const vector<Vector2r>& vertices){
	int i /*current node*/, j/*previous node*/; bool inside=false; int rows=(int)vertices.size();
	const Real& testx=pt[0],testy=pt[1];
	for(i=0,j=rows-1; i<rows; j=i++){
		const Real& vx_i=vertices[i][0], vy_i=vertices[i][1], vx_j=vertices[j][0], vy_j=vertices[j][1];
		if (((vy_i>testy)!=(vy_j>testy)) && (testx < (vx_j-vx_i) * (testy-vy_i) / (vy_j-vy_i) + vx_i) ) inside=!inside;
	}
	return inside;
}

void pybind_init_WeightedAverage2d(pybind11::module& m){
	m.doc() = "Smoothing (2d gauss-weighted average) for postprocessing scalars in 2d.";
	pybind11::class_<pyGaussAverage>(m, "GaussAverage")
		.def(pybind11::init([](pybind11::tuple min, pybind11::tuple max, pybind11::tuple nCells, Real stDev, Real relThreshold){
			return new pyGaussAverage(min, max, nCells, stDev, relThreshold);
		}),
			pybind11::arg("min"), pybind11::arg("max"), pybind11::arg("nCells"),
			pybind11::arg("stDev"), pybind11::arg("relThreshold")=3.0,
			"Create empty container for data, which can be added using add and later retrieved using avg.")
		.def("add",&pyGaussAverage::addPt)
		.def("avg",&pyGaussAverage::avg)
		.def("avgPerUnitArea",&pyGaussAverage::avgPerUnitArea)
		.def("cellNum",&pyGaussAverage::cellNum)
		.def("cellSum",&pyGaussAverage::cellSum)
		.def("cellAvg",&pyGaussAverage::cellAvg)
		.def_property("stDev",&pyGaussAverage::stDev_get,&pyGaussAverage::stDev_set)
		.def_property("relThreshold",&pyGaussAverage::relThreshold_get,&pyGaussAverage::relThreshold_set)
		.def_property("clips",&pyGaussAverage::clips_get,&pyGaussAverage::clips_set)
		.def_property("data",&pyGaussAverage::data_get,nullptr)
		.def_property("aabb",&pyGaussAverage::aabb_get,nullptr)
		.def_property("nCells",&pyGaussAverage::nCells_get,nullptr)
		.def_property_readonly("cellArea",&pyGaussAverage::cellArea)
		.def_property_readonly("cellDim",&pyGaussAverage::cellDim)
	;
}

