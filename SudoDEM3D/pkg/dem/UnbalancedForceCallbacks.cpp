#include<sudodem/pkg/dem/UnbalancedForceCallbacks.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/core/Scene.hpp>

IntrCallback::FuncPtr SumIntrForcesCb::stepInit(){
	// if(scene->iter%100 != 0) return NULL;

	cerr<<"("<<(Real)force<<","<<(int)numIntr<<")";
	// reset accumulators
	force.reset(); numIntr.reset();
	// return function pointer
	return &SumIntrForcesCb::go;
}

void SumIntrForcesCb::go(IntrCallback* _self, Interaction* i){
	SumIntrForcesCb* self=static_cast<SumIntrForcesCb*>(_self);
	NormShearPhys* nsp=SUDODEM_CAST<NormShearPhys*>(i->phys.get());
	assert(nsp!=NULL); // only effective in debug mode
	Vector3r f=nsp->normalForce+nsp->shearForce;
	if(f==Vector3r::Zero()) return;
	self->numIntr+=1;
	self->force+=f.norm();
	//cerr<<"[cb#"<<i->getId1()<<"+"<<i->getId2()<<"]";
}

#ifdef SUDODEM_BODY_CALLBACK
	BodyCallback::FuncPtr SumBodyForcesCb::stepInit(){
		cerr<<"{"<<(Real)force<<","<<(int)numBodies<<",this="<<this<<",scene="<<scene<<",forces="<<&(scene->forces)<<"}";
		force.reset(); numBodies.reset(); // reset accumulators
		return &SumBodyForcesCb::go;
	}
	void SumBodyForcesCb::go(BodyCallback* _self, Body* b){
		if(b->state->blockedDOFs==State::DOF_ALL) return;
		SumBodyForcesCb* self=static_cast<SumBodyForcesCb*>(_self);
	#ifdef SUDODEM_OPENMP
		cerr<<"["<<omp_get_thread_num()<<",#"<<b->id<<",scene="<<self->scene<<"]";
	#endif
		cerr<<"[force="<<self->scene->forces.getForce(b->id)<<"]";
		self->numBodies+=1;
		//self->scene->forces.sync();
		self->force+=self->scene->forces.getForce(b->id).norm();
	}
#endif

// pyRegisterClass implementation moved from UnbalancedForceCallbacks.hpp
void SumIntrForcesCb::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("SumIntrForcesCb");
	pybind11::class_<SumIntrForcesCb, IntrCallback, std::shared_ptr<SumIntrForcesCb>> _classObj(_module, "SumIntrForcesCb", "Callback summing magnitudes of forces over all interactions.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readonly("numIntr", &SumIntrForcesCb::numIntr, "Number of interactions");
	_classObj.def_readonly("force", &SumIntrForcesCb::force, "Sum of force magnitudes");
}

#ifdef SUDODEM_BODY_CALLBACK
// pyRegisterClass implementation moved from UnbalancedForceCallbacks.hpp
void SumBodyForcesCb::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("SumBodyForcesCb");
	pybind11::class_<SumBodyForcesCb, BodyCallback, std::shared_ptr<SumBodyForcesCb>> _classObj(_module, "SumBodyForcesCb", "Callback summing magnitudes of resultant forces over dynamic bodies.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readonly("numBodies", &SumBodyForcesCb::numBodies, "Number of bodies");
	_classObj.def_readonly("force", &SumBodyForcesCb::force, "Sum of force magnitudes");
}
#endif
