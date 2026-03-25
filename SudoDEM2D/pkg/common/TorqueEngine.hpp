/*************************************************************************
*  Copyright (C) 2008 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include <sudodem/core/PartialEngine.hpp>
#include <sudodem/core/Scene.hpp>

class TorqueEngine: public PartialEngine{
	public:
		Real moment;
		
		virtual void action() override {
			for(const Body::id_t id : ids){
				scene->forces.addTorque(id,moment);
			}
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<TorqueEngine, PartialEngine, std::shared_ptr<TorqueEngine>> _classObj(_module, "TorqueEngine", "Apply given torque (momentum) value at every subscribed particle, at every step.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("moment", &TorqueEngine::moment, "Torque value to be applied.");
		}
};
REGISTER_SERIALIZABLE_BASE(TorqueEngine, PartialEngine);
