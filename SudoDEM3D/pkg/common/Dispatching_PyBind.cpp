// Python bindings for Dispatching classes

#include <sudodem/pkg/common/Dispatching.hpp>

void BoundFunctor::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("BoundFunctor");
	pybind11::class_<BoundFunctor, Functor, std::shared_ptr<BoundFunctor>> _classObj(_module, "BoundFunctor", "Functor for creating/updating :yref:`Body::bound`.");
	_classObj.def(pybind11::init<>());
}

void IGeomFunctor::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("IGeomFunctor");
	pybind11::class_<IGeomFunctor, Functor, std::shared_ptr<IGeomFunctor>> _classObj(_module, "IGeomFunctor", "Functor for creating/updating :yref:`Interaction::geom` objects.");
	_classObj.def(pybind11::init<>());
	_classObj.def("preStep", &IGeomFunctor::preStep, "called before every step once, from InteractionLoop (used to set Scene::flags & Scene::LOCAL_COORDS)");
}

void IPhysFunctor::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("IPhysFunctor");
	pybind11::class_<IPhysFunctor, Functor, std::shared_ptr<IPhysFunctor>> _classObj(_module, "IPhysFunctor", "Functor for creating/updating :yref:`Interaction::phys` objects.");
	_classObj.def(pybind11::init<>());
}

void LawFunctor::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("LawFunctor");
	pybind11::class_<LawFunctor, Functor, std::shared_ptr<LawFunctor>> _classObj(_module, "LawFunctor", "Functor for applying constitutive laws on :yref:`interactions<Interaction>`.");
	_classObj.def(pybind11::init<>());
	_classObj.def("preStep", &LawFunctor::preStep, "called before every step once, from InteractionLoop (used to set Scene::flags & Scene::COMPRESSION_NEGATIVE)");
	_classObj.def("addForce", &LawFunctor::addForce, "Convenience functions to get forces/torques quickly.");
	_classObj.def("addTorque", &LawFunctor::addTorque, "Convenience functions to get forces/torques quickly.");
	_classObj.def("applyForceAtContactPoint", &LawFunctor::applyForceAtContactPoint, "Convenience function to apply force and torque from one force at contact point.");
}

void BoundDispatcher::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("BoundDispatcher");
	pybind11::class_<BoundDispatcher, Dispatcher, std::shared_ptr<BoundDispatcher>> _classObj(_module, "BoundDispatcher", "Dispatcher calling :yref:`functors<BoundFunctor>` based on received argument type(s).");
	_classObj.def(pybind11::init<>());
	_classObj.def("add", static_cast<void (BoundDispatcher::*)(shared_ptr<BoundFunctor>)>(&BoundDispatcher::add), "Add functor to dispatcher.");
	_classObj.def_property("functors", &BoundDispatcher::functors_get, &BoundDispatcher::functors_set, "Functors associated with this dispatcher.");
	_classObj.def("dispMatrix", &BoundDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
	_classObj.def("dispFunctor", &BoundDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
	_classObj.def_readwrite("activated", &BoundDispatcher::activated, "Whether the engine is activated (only should be changed by the collider)");
	_classObj.def_readwrite("sweepDist", &BoundDispatcher::sweepDist, "Distance by which enlarge all bounding boxes, to prevent collider from being run at every step (only should be changed by the collider).");
	_classObj.def_readwrite("minSweepDistFactor", &BoundDispatcher::minSweepDistFactor, "Minimal distance by which enlarge all bounding boxes; superseeds computed value of sweepDist when lower that (minSweepDistFactor x sweepDist). Updated by the collider.");
	_classObj.def_readwrite("updatingDispFactor", &BoundDispatcher::updatingDispFactor, "see :yref:`InsertionSortCollider::updatingDispFactor`");
	_classObj.def_readwrite("targetInterv", &BoundDispatcher::targetInterv, "see :yref:`InsertionSortCollider::targetInterv`");
}

void IGeomDispatcher::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("IGeomDispatcher");
	pybind11::class_<IGeomDispatcher, Dispatcher, std::shared_ptr<IGeomDispatcher>> _classObj(_module, "IGeomDispatcher", "Dispatcher calling :yref:`functors<IGeomFunctor>` based on received argument type(s).");
	_classObj.def(pybind11::init<>());
	_classObj.def("add", static_cast<void (IGeomDispatcher::*)(shared_ptr<IGeomFunctor>)>(&IGeomDispatcher::add), "Add functor to dispatcher.");
	_classObj.def_property("functors", &IGeomDispatcher::functors_get, &IGeomDispatcher::functors_set, "Functors associated with this dispatcher.");
	_classObj.def("dispMatrix", &IGeomDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
	_classObj.def("dispFunctor", &IGeomDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
}

void IPhysDispatcher::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("IPhysDispatcher");
	pybind11::class_<IPhysDispatcher, Dispatcher, std::shared_ptr<IPhysDispatcher>> _classObj(_module, "IPhysDispatcher", "Dispatcher calling :yref:`functors<IPhysFunctor>` based on received argument type(s).");
	_classObj.def(pybind11::init<>());
	_classObj.def("add", static_cast<void (IPhysDispatcher::*)(shared_ptr<IPhysFunctor>)>(&IPhysDispatcher::add), "Add functor to dispatcher.");
	_classObj.def_property("functors", &IPhysDispatcher::functors_get, &IPhysDispatcher::functors_set, "Functors associated with this dispatcher.");
	_classObj.def("dispMatrix", &IPhysDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
	_classObj.def("dispFunctor", &IPhysDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
}

void LawDispatcher::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("LawDispatcher");
	pybind11::class_<LawDispatcher, Dispatcher, std::shared_ptr<LawDispatcher>> _classObj(_module, "LawDispatcher", "Dispatcher calling :yref:`functors<LawFunctor>` based on received argument type(s).");
	_classObj.def(pybind11::init<>());
	_classObj.def("add", static_cast<void (LawDispatcher::*)(shared_ptr<LawFunctor>)>(&LawDispatcher::add), "Add functor to dispatcher.");
	_classObj.def_property("functors", &LawDispatcher::functors_get, &LawDispatcher::functors_set, "Functors associated with this dispatcher.");
	_classObj.def("dispMatrix", &LawDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
	_classObj.def("dispFunctor", &LawDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
}
