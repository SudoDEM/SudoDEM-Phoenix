#include<sudodem/pkg/dem/DemXDofGeom.hpp>

// pyRegisterClass implementation moved from DemXDofGeom.hpp
void GenericSpheresContact::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("GenericSpheresContact");
	pybind11::class_<GenericSpheresContact, IGeom, std::shared_ptr<GenericSpheresContact>> _classObj(_module, "GenericSpheresContact", "Class uniting ScGeom and L3Geom");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("normal", &GenericSpheresContact::normal, "Unit vector oriented along the interaction");
	_classObj.def_readwrite("contactPoint", &GenericSpheresContact::contactPoint, "some reference point for the interaction");
	_classObj.def_readwrite("refR1", &GenericSpheresContact::refR1, "Reference radius of particle #1");
	_classObj.def_readwrite("refR2", &GenericSpheresContact::refR2, "Reference radius of particle #2");
}

REGISTER_CLASS_INDEX_CPP(GenericSpheresContact,IGeom)
