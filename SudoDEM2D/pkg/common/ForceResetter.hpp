#pragma once

#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Scene.hpp>

class Scene;
class ForceResetter: public GlobalEngine{
	public:
		virtual void action() override {
			scene->forces.reset(scene->iter);
			if(scene->trackEnergy) scene->energy->resetResettables();
		}
		
		ForceResetter() {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<ForceResetter, GlobalEngine, std::shared_ptr<ForceResetter>> _classObj(_module, "ForceResetter", "Reset all forces stored in Scene::forces (``O.forces`` in python). Typically, this is the first engine to be run at every step. In addition, reset those energies that should be reset, if energy tracing is enabled.");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(ForceResetter, GlobalEngine);