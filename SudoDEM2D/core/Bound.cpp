#include <sudodem/core/Bound.hpp>

REGISTER_INDEX_COUNTER_CPP(Bound)

void Bound::pyRegisterClass(pybind11::module_ _module) 
{
    checkPyClassRegistersItself("Bound");
    pybind11::class_<Bound, Serializable, std::shared_ptr<Bound>> _classObj(_module, "Bound", "Object bounding part of space taken by associated body; might be larger, used to optimalize collision detection");
    _classObj.def(pybind11::init<>());
    // Register attributes (readonly)
    _classObj.def_readonly("lastUpdateIter", &Bound::lastUpdateIter, "record iteration of last reference position update |yupdate|");
    _classObj.def_readonly("refPos", &Bound::refPos, "Reference position, updated at current body position each time the bound dispatcher update bounds |yupdate|");
    _classObj.def_readonly("sweepLength", &Bound::sweepLength, "The length used to increase the bounding boxe size, can be adjusted on the basis of previous displacement if :yref:`BoundDispatcher::targetInterv`>0. |yupdate|");
    _classObj.def_readwrite("color", &Bound::color, "Color for rendering this object");
    _classObj.def_readonly("min", &Bound::min, "Lower corner of box containing this bound (and the :yref:`Body` as well)");
    _classObj.def_readonly("max", &Bound::max, "Upper corner of box containing this bound (and the :yref:`Body` as well)");
    // Python-specific properties
    _classObj.def_property_readonly("dispIndex", [](std::shared_ptr<Bound> b){ return Indexable_getClassIndex(b); }, "Return class index of this instance.");
    _classObj.def("dispHierarchy", [](std::shared_ptr<Bound> b, bool names=true){ return Indexable_getClassIndices(b, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
}