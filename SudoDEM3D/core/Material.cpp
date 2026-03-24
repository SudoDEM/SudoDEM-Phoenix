#include<stdexcept>
#include<sudodem/core/Material.hpp>
#include<sudodem/core/Scene.hpp>

REGISTER_INDEX_COUNTER_CPP(Material)

const shared_ptr<Material> Material::byId(int id, Scene* w_){
	Scene* w=w_?w_:Omega::instance().getScene().get();
	assert(id>=0 && (size_t)id<w->materials.size());
	assert(w->materials[id]->id == id);
	return w->materials[id];
}

const shared_ptr<Material> Material::byLabel(const std::string& label, Scene* w_){
	Scene* w=w_?w_:Omega::instance().getScene().get();
	for(const shared_ptr<Material>& m : w->materials){
		if(m->label == label) return m;
	}
	throw std::runtime_error(("No material labeled `"+label+"'.").c_str());
}

const int Material::byLabelIndex(const std::string& label, Scene* w_){
	Scene* w=w_?w_:Omega::instance().getScene().get(); size_t iMax=w->materials.size();
	for(size_t i=0; i<iMax; i++){
		if(w->materials[i]->label==label) return i;
	}
	throw std::runtime_error(("No material labeled `"+label+"'.").c_str());
}

void Material::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Material");
	pybind11::class_<Material, Serializable, std::shared_ptr<Material>> _classObj(_module, "Material", "Material properties of a :yref:`body<Body>`.");
	_classObj.def(pybind11::init<>());
	// Register attributes
	_classObj.def_readonly("id", &Material::id, "Numeric id of this material; is non-negative only if this Material is shared (i.e. in O.materials), -1 otherwise. This value is set automatically when the material is inserted to the simulation via :yref:`O.materials.append<MaterialContainer.append>`. (This id was necessary since before Cereal serialization was used, shared pointers were not tracked properly; it might disappear in the future)");
	_classObj.def_readwrite("label", &Material::label, "Textual identifier for this material; can be used for shared materials lookup in :yref:`MaterialContainer`.");
	_classObj.def_readwrite("density", &Material::density, "Density of the material [kg/m³]");
	// Python-specific properties
	_classObj.def("newAssocState", &Material::newAssocState, "Return new :yref:`State` instance, which is associated with this :yref:`Material`. Some materials have special requirement on :yref:`Body::state` type and calling this function when the body is created will ensure that they match. (This is done automatically if you use utils.disk, … functions from python).");
	_classObj.def_property_readonly("dispIndex", [](std::shared_ptr<Material> m){ return Indexable_getClassIndex(m); }, "Return class index of this instance.");
	_classObj.def("dispHierarchy", [](std::shared_ptr<Material> m, bool names=true){ return Indexable_getClassIndices(m, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
}