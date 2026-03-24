// 2016 Hong Kong University of Science and Technology and South China University of Technology
// Authors: Chiara Modenese, Wei Zhou, Shiwei Zhao

#include<sudodem/pkg/dem/PolySuperellipsoid.hpp>

void PolySuperellipsoid::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolySuperellipsoid");
	pybind11::class_<PolySuperellipsoid, Shape, std::shared_ptr<PolySuperellipsoid>> _classObj(_module, "PolySuperellipsoid", "PolySuperellipsoid shape");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("rxyz", &PolySuperellipsoid::rxyz, "half-axis lengths");
	_classObj.def_readwrite("rxyz_ref", &PolySuperellipsoid::rxyz_ref, "reference half-axis lengths");
	_classObj.def_readwrite("eps", &PolySuperellipsoid::eps, "exponents");
	_classObj.def_readwrite("m_GL_slices", &PolySuperellipsoid::m_GL_slices, "GL slices");
	_classObj.def("Initial",&PolySuperellipsoid::Initial,"Initialization");
	_classObj.def("getVolume",&PolySuperellipsoid::getVolume,"return particle's volume");
	_classObj.def("getMassCenter",&PolySuperellipsoid::getMassCenter,"mass center at the local coordinate system with a fixed origin at the geometry center");
	_classObj.def("getrxyz",&PolySuperellipsoid::getrxyz,"return particle's rxyz");
	_classObj.def("geteps",&PolySuperellipsoid::geteps,"return particle's eps1 and eps2");
	_classObj.def("getInertia",&PolySuperellipsoid::getInertia,"return particle's inertia tensor");
	_classObj.def("getOri",&PolySuperellipsoid::getOrientation,"return particle's orientation");
	_classObj.def("getCentroid",&PolySuperellipsoid::getPosition,"return particle's centroid");
}

void PolySuperellipsoidGeom::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolySuperellipsoidGeom");
	pybind11::class_<PolySuperellipsoidGeom, IGeom, std::shared_ptr<PolySuperellipsoidGeom>> _classObj(_module, "PolySuperellipsoidGeom", "Geometry of interaction between PolySuperellipsoid");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("PenetrationDepth", &PolySuperellipsoidGeom::PenetrationDepth, "PenetrationDepth");
	_classObj.def_readwrite("contactAngle", &PolySuperellipsoidGeom::contactAngle, "Normal direction");
	_classObj.def_readwrite("contactPoint", &PolySuperellipsoidGeom::contactPoint, "Contact point");
	_classObj.def_readwrite("shearInc", &PolySuperellipsoidGeom::shearInc, "Shear displacement increment");
	_classObj.def_readwrite("relativeVn", &PolySuperellipsoidGeom::relativeVn, "relative velocity in the normal");
	_classObj.def_readwrite("relativeVs", &PolySuperellipsoidGeom::relativeVs, "relative velocity in the tangential");
	_classObj.def_readwrite("normal", &PolySuperellipsoidGeom::normal, "Contact normal");
	_classObj.def_readwrite("twist_axis", &PolySuperellipsoidGeom::twist_axis, "Twist axis");
	_classObj.def_readwrite("point1", &PolySuperellipsoidGeom::point1, "point 1");
	_classObj.def_readwrite("point2", &PolySuperellipsoidGeom::point2, "point 2");
	_classObj.def_readwrite("isShearNew", &PolySuperellipsoidGeom::isShearNew, "isShearNew");
	_classObj.def_readwrite("orthonormal_axis", &PolySuperellipsoidGeom::orthonormal_axis, "orthonormal_axis");
}

void Bo1_PolySuperellipsoid_Aabb::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Bo1_PolySuperellipsoid_Aabb");
	pybind11::class_<Bo1_PolySuperellipsoid_Aabb, BoundFunctor, std::shared_ptr<Bo1_PolySuperellipsoid_Aabb>> _classObj(_module, "Bo1_PolySuperellipsoid_Aabb", "Create Aabb from PolySuperellipsoid");
	_classObj.def(pybind11::init<>());
}

void PolySuperellipsoidMat::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolySuperellipsoidMat");
	pybind11::class_<PolySuperellipsoidMat, Material, std::shared_ptr<PolySuperellipsoidMat>> _classObj(_module, "PolySuperellipsoidMat", "Elastic material with Coulomb friction");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("Kn", &PolySuperellipsoidMat::Kn, "Normal stiffness");
	_classObj.def_readwrite("Ks", &PolySuperellipsoidMat::Ks, "Shear stiffness");
	_classObj.def_readwrite("frictionAngle", &PolySuperellipsoidMat::frictionAngle, "Contact friction angle");
	_classObj.def_readwrite("betan", &PolySuperellipsoidMat::betan, "Normal Damping Ratio");
	_classObj.def_readwrite("betas", &PolySuperellipsoidMat::betas, "Shear Damping Ratio");
	_classObj.def_readwrite("IsSplitable", &PolySuperellipsoidMat::IsSplitable, "To be splitted");
	_classObj.def_readwrite("strength", &PolySuperellipsoidMat::strength, "Stress at which polyhedra breaks");
}

void PolySuperellipsoidPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolySuperellipsoidPhys");
	pybind11::class_<PolySuperellipsoidPhys, FrictPhys, std::shared_ptr<PolySuperellipsoidPhys>> _classObj(_module, "PolySuperellipsoidPhys", "Simple elastic material with friction for volumetric constitutive laws");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("normalViscous", &PolySuperellipsoidPhys::normalViscous, "Normal viscous component");
	_classObj.def_readwrite("shearViscous", &PolySuperellipsoidPhys::shearViscous, "Shear viscous component");
	_classObj.def_readwrite("betan", &PolySuperellipsoidPhys::betan, "Normal Damping Ratio");
	_classObj.def_readwrite("betas", &PolySuperellipsoidPhys::betas, "Shear Damping Ratio");
}

void Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys");
	pybind11::class_<Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys, IPhysFunctor, std::shared_ptr<Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys>> _classObj(_module, "Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys", "Material to physics functor for PolySuperellipsoid");
	_classObj.def(pybind11::init<>());
}

void PolySuperellipsoidLaw::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("PolySuperellipsoidLaw");
	pybind11::class_<PolySuperellipsoidLaw, LawFunctor, std::shared_ptr<PolySuperellipsoidLaw>> _classObj(_module, "PolySuperellipsoidLaw", "Calculate physical response of PolySuperellipsoid");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("shearForce", &PolySuperellipsoidLaw::shearForce, "Shear force from last step");
	_classObj.def_readwrite("traceEnergy", &PolySuperellipsoidLaw::traceEnergy, "Define the total energy dissipated in plastic slips");
	_classObj.def("elasticEnergy", &PolySuperellipsoidLaw::elasticEnergy, "Compute and return the total elastic energy");
	_classObj.def("getPlasticDissipation", &PolySuperellipsoidLaw::getPlasticDissipation, "Total energy dissipated in plastic slips");
	_classObj.def("initPlasticDissipation", &PolySuperellipsoidLaw::initPlasticDissipation, "Initialize cummulated plastic dissipation");
}
