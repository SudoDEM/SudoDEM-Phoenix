
#include<sudodem/core/Body.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/InteractionContainer.hpp>

//! This could be -1 if id_t is re-typedef'ed as `int'

const shared_ptr<Body>& Body::byId(Body::id_t _id, Scene* rb){return (*((rb?rb:Omega::instance().getScene().get())->bodies))[_id];}
const shared_ptr<Body>& Body::byId(Body::id_t _id, shared_ptr<Scene> rb){return (*(rb->bodies))[_id];}

// return list of interactions of this particle
pybind11::list Body::py_intrs(){
  pybind11::list ret;
	for(Body::MapId2IntrT::iterator it=this->intrs.begin(),end=this->intrs.end(); it!=end; ++it) {  //Iterate over all bodie's interactions
		if(!(*it).second->isReal()) continue;
		ret.append((*it).second);
	}
	return ret;
}

// return list of interactions of this particle
unsigned int Body::coordNumber(){
	unsigned int intrSize = 0;
	for(Body::MapId2IntrT::iterator it=this->intrs.begin(),end=this->intrs.end(); it!=end; ++it) {  //Iterate over all bodie's interactions
		if(!(*it).second->isReal()) continue;
		intrSize++;
	}
	return intrSize;
}


bool Body::maskOk(int mask) const { return (mask==0 || ((groupMask & mask) != 0)); }
bool Body::maskCompatible(int mask) const { return (groupMask & mask) != 0; }
#ifdef SUDODEM_MASK_ARBITRARY
bool Body::maskOk(const mask_t& mask) const { return (mask==0 || ((groupMask & mask) != 0)); }
bool Body::maskCompatible(const mask_t& mask) const { return (groupMask & mask) != 0; }
#endif

void Body::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Body");
	pybind11::class_<Body, Serializable, std::shared_ptr<Body>> _classObj(_module, "Body", "A particle, basic element of simulation; interacts with other bodies.");
	_classObj.def(pybind11::init<>());
	// Register attributes
	_classObj.def_readonly("id", &Body::id, "Unique id of this body.");
	_classObj.def_readwrite("groupMask", &Body::groupMask, "Bitmask for determining interactions.");
	_classObj.def_readonly("flags", &Body::flags, "Bits of various body-related flags. *Do not access directly*. In c++, use isDynamic/setDynamic, isBounded/setBounded, isAspherical/setAspherical. In python, use :yref:`Body.dynamic`, :yref:`Body.bounded`, :yref:`Body.aspherical`.");
	_classObj.def_readwrite("material", &Body::material, ":yref:`Material` instance associated with this body.");
	_classObj.def_readwrite("state", &Body::state, "Physical :yref:`state<State>`.");
	_classObj.def_readwrite("shape", &Body::shape, "Geometrical :yref:`Shape`.");
	_classObj.def_readwrite("bound", &Body::bound, ":yref:`Bound`, approximating volume for the purposes of collision detection.");
	_classObj.def_readonly("clumpId", &Body::clumpId, "Id of clump this body makes part of; invalid number if not part of clump; see :yref:`Body::isStandalone`, :yref:`Body::isClump`, :yref:`Body::isClumpMember` properties. \n\nNot meant to be modified directly from Python, use :yref:`O.bodies.appendClumped<BodyContainer.appendClumped>` instead.");
	_classObj.def_readwrite("chain", &Body::chain, "Id of chain to which the body belongs.");
	_classObj.def_readwrite("iterBorn", &Body::iterBorn, "Step number at which the body was added to simulation.");
	_classObj.def_readwrite("timeBorn", &Body::timeBorn, "Time at which the body was added to simulation.");
	// Python-specific properties
	_classObj.def_readwrite("mat", &Body::material, "Shorthand for :yref:`Body::material`");
	_classObj.def_property("dynamic", &Body::isDynamic, &Body::setDynamic, "Whether this body will be moved by forces. (In c++, use ``Body::isDynamic``/``Body::setDynamic``) :ydefault:`true`");
	_classObj.def_property("bounded", &Body::isBounded, &Body::setBounded, "Whether this body should have :yref:`Body.bound` created. Note that bodies without a :yref:`bound <Body.bound>` do not participate in collision detection. (In c++, use ``Body::isBounded``/``Body::setBounded``) :ydefault:`true`");
	_classObj.def_property("aspherical", &Body::isAspherical, &Body::setAspherical, "Whether this body has different inertia along principal axes; :yref:`NewtonIntegrator` makes use of this flag to call rotation integration routine for aspherical bodies, which is more expensive. :ydefault:`false`");
	_classObj.def_readwrite("mask", &Body::groupMask, "Shorthand for :yref:`Body::groupMask`");
	_classObj.def_property_readonly("isStandalone", &Body::isStandalone, "True if this body is neither clump, nor clump member; false otherwise.");
	_classObj.def_property_readonly("isClumpMember", &Body::isClumpMember, "True if this body is clump member, false otherwise.");
	_classObj.def_property_readonly("isClump", &Body::isClump, "True if this body is clump itself, false otherwise.");
	_classObj.def_readonly("iterBorn", &Body::iterBorn, "Returns step number at which the body was added to simulation.");
	_classObj.def_readonly("timeBorn", &Body::timeBorn, "Returns time at which the body was added to simulation.");
	_classObj.def_readwrite("chain", &Body::chain, "Returns Id of chain to which the body belongs.");
	_classObj.def("intrs", &Body::py_intrs, "Return all interactions in which this body participates.");
#ifdef SUDODEM_SPH
	_classObj.def_readwrite("rho", &Body::rho, "Returns the current density (only for SPH-model).");
	_classObj.def_readwrite("rho0", &Body::rho0, "Returns the rest density (only for SPH-model).");
	_classObj.def_readwrite("press", &Body::press, "Returns the pressure (only for SPH-model).");
#endif
}



