#include<sudodem/pkg/fem/Node.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/InteractionContainer.hpp>
#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif



//! This could be -1 if id_t is re-typedef'ed as `int'

const shared_ptr<Node>& Node::byId(Node::id_t _id, Scene* rb){return (*((rb?rb:Omega::instance().getScene().get())->nodes))[_id];}
const shared_ptr<Node>& Node::byId(Node::id_t _id, shared_ptr<Scene> rb){return (*(rb->nodes))[_id];}

bool Node::maskOk(int mask) const { return (mask==0 || ((groupMask & mask) != 0)); }
bool Node::maskCompatible(int mask) const { return (groupMask & mask) != 0; }
#ifdef SUDODEM_MASK_ARBITRARY
bool Node::maskOk(const mask_t& mask) const { return (mask==0 || ((groupMask & mask) != 0)); }
bool Node::maskCompatible(const mask_t& mask) const { return (groupMask & mask) != 0; }
#endif


void Node::setDOFfromVector3r(Vector3r disp,Vector3r rot){
	blockedDOFs=((disp[0]==1.0)?DOF_X :0)|((disp[1]==1.0)?DOF_Y :0)|((disp[2]==1.0)?DOF_Z :0)|
		((rot [0]==1.0)?DOF_RX:0)|((rot [1]==1.0)?DOF_RY:0)|((rot [2]==1.0)?DOF_RZ:0);
}

std::string Node::blockedDOFs_vec_get() const {
	std::string ret;
	#define _SET_DOF(DOF_ANY,ch) if((blockedDOFs & Node::DOF_ANY)!=0) ret.push_back(ch);
	_SET_DOF(DOF_X,'x'); _SET_DOF(DOF_Y,'y'); _SET_DOF(DOF_Z,'z'); _SET_DOF(DOF_RX,'X'); _SET_DOF(DOF_RY,'Y'); _SET_DOF(DOF_RZ,'Z');
	#undef _SET_DOF
	return ret;
}

void Node::blockedDOFs_vec_set(const std::string& dofs){
	blockedDOFs=0;
		for(char c : dofs){
			#define _GET_DOF(DOF_ANY,ch) if(c==ch) { blockedDOFs|=Node::DOF_ANY; continue; }
			_GET_DOF(DOF_X,'x'); _GET_DOF(DOF_Y,'y'); _GET_DOF(DOF_Z,'z'); _GET_DOF(DOF_RX,'X'); _GET_DOF(DOF_RY,'Y'); _GET_DOF(DOF_RZ,'Z');
			#undef _GET_DOF
			throw std::invalid_argument("Invalid  DOF specification `"+std::string(1,c)+"' in '"+dofs+"', characters must be ∈{x,y,z,X,Y,Z}.");
	}
}


CREATE_LOGGER(NodeContainer);

void NodeContainer::clear(){
	node.clear();
}

Node::id_t NodeContainer::insert(shared_ptr<Node>& b){
	const shared_ptr<Scene>& scene=Omega::instance().getScene();
	b->iterBorn=scene->iter;
	b->timeBorn=scene->time;
	b->id=node.size();
	scene->doSort = true;
	node.push_back(b);
	// Notify ForceContainer about new id
	scene->nodeforces.addMaxId(b->id);
	return b->id;
}

bool NodeContainer::erase(Node::id_t id){//default is false (as before)
	if(!node[id]) return false;
	const shared_ptr<Node>& b=Node::byId(id);
	node[id].reset();
	return true;
}

void Node::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Node");
	pybind11::class_<Node, std::shared_ptr<Node>> _classObj(_module, "Node", "A node for fem elements; no interaction with other Nodes.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("id", &Node::id, "Unique id of this Node.");
	_classObj.def_readwrite("groupMask", &Node::groupMask, "Bitmask for determining interactions.");
	_classObj.def_readwrite("flags", &Node::flags, "Bits of various Node-related flags.");
	_classObj.def_readwrite("pos", &Node::pos, "Position.");
	_classObj.def_readwrite("vel", &Node::vel, "Current linear velocity.");
	_classObj.def_readwrite("angVel", &Node::angVel, "Current angular velocity.");
	_classObj.def_readwrite("inertia", &Node::inertia, "Inertia of associated body, in local coordinate system.");
	_classObj.def_readwrite("mass", &Node::mass, "Mass of this Node");
	_classObj.def_readwrite("refPos", &Node::refPos, "Reference position");
	_classObj.def_readwrite("ori", &Node::ori, "Orientation of this node.");
	_classObj.def_readwrite("blockedDOFs", &Node::blockedDOFs, "Blocked degrees of freedom.");
	_classObj.def_readwrite("isDamped", &Node::isDamped, "Damping enabled.");
	_classObj.def_readwrite("iterBorn", &Node::iterBorn, "Step number at which the Node was added to simulation.");
	_classObj.def_readwrite("timeBorn", &Node::timeBorn, "Time at which the Node was added to simulation.");
	_classObj.def_readwrite("mask", &Node::groupMask, "Shorthand for groupMask");
	_classObj.def_property("blockedDOFs", &Node::blockedDOFs_vec_get, &Node::blockedDOFs_vec_set, "Degrees of freedom where linear/angular velocity will be always constant.");
	_classObj.def_property("pos", &Node::pos_get, &Node::pos_set, "Current position.");
	_classObj.def("displ", &Node::displ, "Displacement from reference position.");
}

