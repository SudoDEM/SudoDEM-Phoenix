// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#include<sudodem/core/State.hpp>

CREATE_LOGGER(State);

void State::setDOFfromVector3r(Vector3r disp_rot){
	blockedDOFs=((disp_rot[0]==1.0)?DOF_X :0)|((disp_rot[1]==1.0)?DOF_Y :0)|((disp_rot[2]==1.0)?DOF_RZ :0);
}

std::string State::blockedDOFs_vec_get() const {
	std::string ret;
	#define _SET_DOF(DOF_ANY,ch) if((blockedDOFs & State::DOF_ANY)!=0) ret.push_back(ch);
	_SET_DOF(DOF_X,'x'); _SET_DOF(DOF_Y,'y'); _SET_DOF(DOF_RZ,'Z');
	#undef _SET_DOF
	return ret;
}

void State::blockedDOFs_vec_set(const std::string& dofs){
	blockedDOFs=0;
	for(char c : dofs){
		#define _GET_DOF(DOF_ANY,ch) if(c==ch) { blockedDOFs|=State::DOF_ANY; continue; }
		_GET_DOF(DOF_X,'x'); _GET_DOF(DOF_Y,'y'); _GET_DOF(DOF_RZ,'Z');
		#undef _GET_DOF
		throw std::invalid_argument("Invalid  DOF specification `"+std::to_string(c)+"' in '"+dofs+"', characters must be ∈{x,y,Z}.");
	}
}

void State::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("State");
	pybind11::class_<State, Serializable, std::shared_ptr<State>> _classObj(_module, "State", "State of a body (spatial configuration, internal variables).");
	_classObj.def(pybind11::init<>());
	// Register attributes
	_classObj.def_readwrite("se2", &State::se2, "Position and orientation as one object.");
	_classObj.def_readwrite("vel", &State::vel, "Current linear velocity.");
	_classObj.def_readwrite("mass", &State::mass, "Mass of this body");
	_classObj.def_readwrite("angVel", &State::angVel, "Current angular velocity");
	_classObj.def_readwrite("angMom", &State::angMom, "Current angular momentum");
	_classObj.def_readwrite("inertia", &State::inertia, "Inertia of associated body, in local coordinate system.");
	_classObj.def_readwrite("refPos", &State::refPos, "Reference position");
	_classObj.def_readwrite("refOri", &State::refOri, "Reference orientation");
	_classObj.def_readwrite("blockedDOFs", &State::blockedDOFs, "[Will be overridden]");
	_classObj.def_readwrite("isDamped", &State::isDamped, "Damping in :yref:`Newtonintegrator` can be deactivated for individual particles by setting this variable to FALSE. E.g. damping is inappropriate for particles in free flight under gravity but it might still be applicable to other particles in the same simulation.");
	_classObj.def_readwrite("densityScaling", &State::densityScaling, "|yupdate| see :yref:`GlobalStiffnessTimeStepper::targetDt`.");
	// Python-specific properties
	_classObj.def_property_readonly("dispIndex", [](std::shared_ptr<State> s){ return Indexable_getClassIndex(s); }, "Return class index of this instance.");
	_classObj.def("dispHierarchy", [](std::shared_ptr<State> s, bool names=true){ return Indexable_getClassIndices(s, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
	_classObj.def_property("blockedDOFs", &State::blockedDOFs_vec_get, &State::blockedDOFs_vec_set, "Degress of freedom where linear/angular velocity will be always constant (equal to zero, or to an user-defined value), regardless of applied force/torque. String that may contain 'xyzXYZ' (translations and rotations).");
	_classObj.def_property("pos", &State::pos_get, &State::pos_set, "Current position.");
	_classObj.def_property("ori", &State::ori_get, &State::ori_set, "Current orientation.");
	_classObj.def("displ", &State::displ, "Displacement from :yref:`reference position<State.refPos>` (:yref:`pos<State.pos>` - :yref:`refPos<State.refPos>`)");
	_classObj.def("rot", &State::rot, "Rotation from :yref:`reference orientation<State.refOri>` (as rotation vector)");
}

REGISTER_INDEX_COUNTER_CPP(State)