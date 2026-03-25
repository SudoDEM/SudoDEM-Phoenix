#pragma once
#include <sudodem/core/GlobalEngine.hpp>
class BoundaryController: public GlobalEngine{
public:
	virtual void action() override {
		{ throw std::runtime_error("BoundaryController must not be used in simulations directly (BoundaryController::action called)."); }
	}
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<BoundaryController, GlobalEngine, std::shared_ptr<BoundaryController>> _classObj(_module, "BoundaryController", "Base for engines controlling boundary conditions of simulations. Not to be used directly.");
		_classObj.def(pybind11::init<>());
	}
};
REGISTER_SERIALIZABLE_BASE(BoundaryController, GlobalEngine);
