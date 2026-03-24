// 2010 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/pkg/common/Callbacks.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

class SumIntrForcesCb: public IntrCallback{
	public:
		OpenMPAccumulator<int> numIntr;
		OpenMPAccumulator<Real> force;
		
		static void go(IntrCallback*,Interaction*);
		virtual IntrCallback::FuncPtr stepInit();
		
		// Explicit constructor with initial values
		SumIntrForcesCb() {}
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(SumIntrForcesCb, IntrCallback);

#ifdef SUDODEM_BODY_CALLBACK
class SumBodyForcesCb: public BodyCallback{
	Scene* scene;
	public:
		OpenMPAccumulator<int> numBodies;
		OpenMPAccumulator<Real> force;
		
		static void go(BodyCallback*,Body*);
		virtual BodyCallback::FuncPtr stepInit();
		
		// Explicit constructor with initial values
		SumBodyForcesCb(): scene(nullptr) {}
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(SumBodyForcesCb, BodyCallback);
#endif