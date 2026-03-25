/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include <list>
#include <vector>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/core/GlobalEngine.hpp>
#include <sudodem/core/Scene.hpp>

class Body;

class TimeStepper: public GlobalEngine{
	public:
		bool active = true;
		unsigned int timeStepUpdateInterval = 1;

		virtual void computeTimeStep(Scene* ) { throw; };
		virtual bool isActivated() override {return (active && (scene->iter % timeStepUpdateInterval == 0));};
		virtual void action() override { computeTimeStep(scene);} ;
		void setActive(bool a, int nb=-1) {active = a; if (nb>0) {timeStepUpdateInterval = (unsigned int)nb;}}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("TimeStepper");
			pybind11::class_<TimeStepper, GlobalEngine, std::shared_ptr<TimeStepper>> _classObj(_module, "TimeStepper", "Engine defining time-step (fundamental class)");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("active", &TimeStepper::active, "is the engine active?");
			_classObj.def_readwrite("timeStepUpdateInterval", &TimeStepper::timeStepUpdateInterval, "dt update interval");
		}
};

REGISTER_SERIALIZABLE_BASE(TimeStepper, GlobalEngine);