#pragma once

#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/PeriodicEngines.hpp>
#include<sudodem/gui/OpenGLManager.hpp>


class SnapshotEngine: public PeriodicEngine {
	public:
		string format;
		string fileBase;
		int counter;
		bool ignoreErrors;
		vector<string> snapshots;
		int msecSleep;
		Real deadTimeout;
		string plot;
		
		virtual void action();
		
		// Explicit constructor with initial values
		SnapshotEngine(): format("PNG"), fileBase(""), counter(0), ignoreErrors(true), msecSleep(0), deadTimeout(3.) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(PeriodicEngine, format, fileBase, counter, ignoreErrors, snapshots, msecSleep, deadTimeout, plot);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("SnapshotEngine");
		pybind11::class_<SnapshotEngine, PeriodicEngine, std::shared_ptr<SnapshotEngine>> _classObj(_module, "SnapshotEngine", "Periodically save snapshots of GLView(s) as .png files.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("format", &SnapshotEngine::format, "Format of snapshots (one of JPEG, PNG, EPS, PS, PPM, BMP)");
		_classObj.def_readwrite("fileBase", &SnapshotEngine::fileBase, "Basename for snapshots");
		_classObj.def_readwrite("counter", &SnapshotEngine::counter, "Number that will be appended to fileBase when the next snapshot is saved (incremented at every save). |yupdate|");
		_classObj.def_readwrite("ignoreErrors", &SnapshotEngine::ignoreErrors, "Only report errors instead of throwing exceptions, in case of timeouts.");
		_classObj.def_readwrite("snapshots", &SnapshotEngine::snapshots, "Files that have been created so far");
		_classObj.def_readwrite("msecSleep", &SnapshotEngine::msecSleep, "number of msec to sleep after snapshot (to prevent 3d hw problems) [ms]");
		_classObj.def_readwrite("deadTimeout", &SnapshotEngine::deadTimeout, "Timeout for 3d operations (opening new view, saving snapshot); after timing out, throw exception (or only report error if *ignoreErrors*) and make myself :yref:`dead<Engine.dead>`. [s]");
		_classObj.def_readwrite("plot", &SnapshotEngine::plot, "Name of field in :yref:`sudodem.plot.imgData` to which taken snapshots will be appended automatically.");
	}
};

REGISTER_SERIALIZABLE_BASE(SnapshotEngine, PeriodicEngine);