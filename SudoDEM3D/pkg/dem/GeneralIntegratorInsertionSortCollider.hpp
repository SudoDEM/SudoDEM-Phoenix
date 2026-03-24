// 2014 Burak ER <burak.er@btu.edu.tr>

#pragma once
#include<sudodem/pkg/common/InsertionSortCollider.hpp>


/*!
	Adaptive Integration Sort Collider:

	Changing the Integrator dependence from Newton Integrator to Arbitrary Integrators. Arbitrary integrators should use the Integrator interface.

*/



#ifdef ISC_TIMING
	#define ISC_CHECKPOINT(cpt) timingDeltas->checkpoint(cpt)
#else
	#define ISC_CHECKPOINT(cpt)
#endif

class Integrator;

class GeneralIntegratorInsertionSortCollider: public InsertionSortCollider{
	shared_ptr<Integrator> integrator;

	public:

	// Explicit constructor with initial values
	GeneralIntegratorInsertionSortCollider() {}
	
	virtual bool isActivated();
	virtual void action();
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(InsertionSortCollider, integrator);
	
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GeneralIntegratorInsertionSortCollider, InsertionSortCollider);