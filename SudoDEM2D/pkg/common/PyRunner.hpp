// 2008 © Václav Šmilauer <eudoxos@arcig.cz>
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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<PyRunner, PeriodicEngine, std::shared_ptr<PyRunner>> _classObj(_module, "PyRunner", "Execute a python command periodically, with defined (and adjustable) periodicity. See :yref:`PeriodicEngine` documentation for details.");
			_classObj.def(pybind11::init<>());
			_classObj.def(pybind11::init([](const std::string& command, Real virtPeriod, Real realPeriod, long iterPeriod, long nDo, bool initRun, const std::string& label, bool dead) {
				auto runner = std::make_shared<PyRunner>();
				runner->command = command;
				runner->virtPeriod = virtPeriod;
				runner->realPeriod = realPeriod;
				runner->iterPeriod = iterPeriod;
				runner->nDo = nDo;
				runner->initRun = initRun;
				runner->label = label;
				runner->dead = dead;
				return runner;
			}), pybind11::arg("command") = "", pybind11::arg("virtPeriod") = 0, pybind11::arg("realPeriod") = 0, pybind11::arg("iterPeriod") = 0, pybind11::arg("nDo") = -1, pybind11::arg("initRun") = false, pybind11::arg("label") = "", pybind11::arg("dead") = false);
			_classObj.def_readwrite("command", &PyRunner::command, "Command to be run by python interpreter. Not run if empty.");
		}
};
REGISTER_SERIALIZABLE_BASE(PyRunner, PeriodicEngine);
