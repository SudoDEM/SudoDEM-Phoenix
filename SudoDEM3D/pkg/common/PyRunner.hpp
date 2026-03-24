// 2008 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/PeriodicEngines.hpp>
#include<sudodem/lib/pyutil/gil.hpp>

class PyRunner: public PeriodicEngine {
	public :
		string command;
		
		/* virtual bool isActivated: not overridden, PeriodicEngine handles that */
		virtual void action() override { if(command.size()>0) pyRunString(command); }
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PyRunner, PeriodicEngine);
