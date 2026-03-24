/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once
#include <vector>
#include<sudodem/core/Engine.hpp>
#include<sudodem/core/Body.hpp>
#include<pybind11/pybind11.h>

class PartialEngine: public Engine{
	public:
		virtual ~PartialEngine() {};

		// Member variables for serialization
		std::vector<int> ids;

	public:
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("PartialEngine");
			pybind11::class_<PartialEngine, Engine, std::shared_ptr<PartialEngine>> _classObj(_module, "PartialEngine", "Engine affecting only particular bodies in the simulation, defined by *ids*.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("ids", &PartialEngine::ids, ":yref:`Ids<Body::id>` of bodies affected by this PartialEngine.");
		}
};
REGISTER_SERIALIZABLE_BASE(PartialEngine, Engine);