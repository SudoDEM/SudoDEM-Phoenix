#include<sudodem/pkg/fem/FEContainer.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/InteractionContainer.hpp>
#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif



//! This could be -1 if id_t is re-typedef'ed as `int'

const shared_ptr<Element>& Element::byId(Element::id_t _id, Scene* rb){return (*((rb?rb:Omega::instance().getScene().get())->elements))[_id];}
const shared_ptr<Element>& Element::byId(Element::id_t _id, shared_ptr<Scene> rb){return (*(rb->elements))[_id];}


CREATE_LOGGER(ElementContainer);

void FEContainer::clear(){
	element.clear();
}

Element::id_t FEContainer::insert(shared_ptr<Element>& b){
	const shared_ptr<Scene>& scene=Omega::instance().getScene();
	b->id=element.size();
	scene->doSort = true;
	element.push_back(b);
	// Notify ForceContainer about new id
	//scene->elementforces.addMaxId(b->id);
	return b->id;
}

bool FEContainer::erase(Element::id_t id){//default is false (as before)
	if(!element[id]) return false;
	const shared_ptr<Element>& b=Element::byId(id);
	element[id].reset();
	return true;
}

void Element::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Element");
	pybind11::class_<Element, std::shared_ptr<Element>> _classObj(_module, "Element", "A general FE element.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("id", &Element::id, "Unique id of this Element.");
	_classObj.def_readwrite("pos", &Element::pos, "Position.");
	_classObj.def_readwrite("ori", &Element::ori, "Orientation of this element.");
}

