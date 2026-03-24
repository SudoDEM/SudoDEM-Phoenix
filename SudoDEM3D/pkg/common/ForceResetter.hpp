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
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ForceResetter, GlobalEngine);