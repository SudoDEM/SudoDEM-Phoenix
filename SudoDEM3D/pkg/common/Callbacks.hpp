// 2010 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once

#include<sudodem/lib/base/Logging.hpp>
#include<sudodem/lib/serialization/Serializable.hpp>

class Interaction;
class Body;
class Scene;

class IntrCallback: public Serializable{
	public:
	virtual ~IntrCallback() {}; // vtable
	typedef void(*FuncPtr)(IntrCallback*,Interaction*);
	// should be set at every step by InteractionLoop
	Scene* scene;
	/*
	At the beginning of each timestep, perform initialization and
	return pointer to the static member function that does the actual work.
	Returned value might be NULL, in which case the callback will be deactivated during that timestep.
	*/
	virtual FuncPtr stepInit() { throw std::runtime_error("Called IntrCallback::stepInit() of the base class?"); }
	
	IntrCallback() : scene(nullptr) {}
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE(IntrCallback);

#ifdef SUDODEM_BODY_CALLBACKS
	class BodyCallback: public Serializable{
		public:
		virtual ~BodyCallback() {}; // vtable
		typedef void(*FuncPtr)(BodyCallback*,Body*);
		// set at every step, before stepInit() is called
		Scene* scene;
		virtual FuncPtr stepInit() override { throw std::runtime_error("Called BodyCallback::stepInit() of the base class?"); }
		
		BodyCallback() : scene(nullptr) {}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	};
	REGISTER_SERIALIZABLE(BodyCallback);
#endif