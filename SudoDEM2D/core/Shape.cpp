#include <sudodem/core/Shape.hpp>

REGISTER_INDEX_COUNTER_CPP(Shape)

void Shape::pyRegisterClass(pybind11::module_ _module)
{
    checkPyClassRegistersItself("Shape");
    pybind11::class_<Shape, Serializable, std::shared_ptr<Shape>> _classObj(_module, "Shape", "Geometry of a body");
    _classObj.def(pybind11::init<>());
    // Register attributes
    _classObj.def_readwrite("color", &Shape::color, "Color for rendering (normalized RGB).");
    _classObj.def_readwrite("wire", &Shape::wire, "Whether this Shape is rendered using color surfaces, or only wireframe (can still be overridden by global config of the renderer).");
    _classObj.def_readwrite("highlight", &Shape::highlight, "Whether this Shape will be highlighted when rendered.");
    // Python-specific properties
    _classObj.def_property_readonly("dispIndex", [](std::shared_ptr<Shape> s){ return Indexable_getClassIndex(s); }, "Return class index of this instance.");
    _classObj.def("dispHierarchy", [](std::shared_ptr<Shape> s, bool names=true){ return Indexable_getClassIndices(s, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
}