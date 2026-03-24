/*************************************************************************
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include <sudodem/core/Engine.hpp>

class GlobalEngine: public Engine{
	public :
		virtual ~GlobalEngine() {};
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("GlobalEngine");
			pybind11::class_<GlobalEngine, Engine, std::shared_ptr<GlobalEngine>> _classObj(_module, "GlobalEngine", "Engine that will generally affect the whole simulation (contrary to PartialEngine).");
			_classObj.def(pybind11::init<>());
		}
	SUDODEM_CLASS_BASE_DOC(GlobalEngine,Engine,"Engine that will generally affect the whole simulation (contrary to PartialEngine).");
};
REGISTER_SERIALIZABLE_BASE(GlobalEngine, Engine);


