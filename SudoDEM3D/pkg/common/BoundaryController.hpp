#pragma once
#include<sudodem/core/GlobalEngine.hpp>
class BoundaryController: public GlobalEngine{
public:
	virtual void action() override {
		{ throw std::runtime_error("BoundaryController must not be used in simulations directly (BoundaryController::action called)."); }
	}
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(BoundaryController, GlobalEngine);
