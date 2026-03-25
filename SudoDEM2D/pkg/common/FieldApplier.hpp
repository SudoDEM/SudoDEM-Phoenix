#pragma once
#include <sudodem/core/GlobalEngine.hpp>
#include <stdexcept>

class FieldApplier: public GlobalEngine{
	virtual void action() override {
		throw std::runtime_error("FieldApplier must not be used in simulations directly (FieldApplier::action called).");
	}
	
public:
	int fieldWorkIx;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<FieldApplier, GlobalEngine, std::shared_ptr<FieldApplier>> _classObj(_module, "FieldApplier", "Base for engines applying force files on particles. Not to be used directly.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("fieldWorkIx", &FieldApplier::fieldWorkIx, "Index for the work done by this field, if tracking energies.");
		_classObj.def("action", &FieldApplier::action);
	}
};
REGISTER_SERIALIZABLE_BASE(FieldApplier, GlobalEngine);

