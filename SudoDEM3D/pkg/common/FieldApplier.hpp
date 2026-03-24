#pragma once
#include <sudodem/core/GlobalEngine.hpp>
#include <stdexcept>

class FieldApplier: public GlobalEngine{
	virtual void action() override {
		throw std::runtime_error("FieldApplier must not be used in simulations directly (FieldApplier::action called).");
	}
	
public:
	int fieldWorkIx;
	
	// Override pyRegisterClass to register FieldApplier name instead of GlobalEngine
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(FieldApplier, GlobalEngine);

