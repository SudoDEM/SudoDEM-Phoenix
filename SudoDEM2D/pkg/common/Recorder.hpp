// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/pkg/common/PeriodicEngines.hpp>
class Recorder: public PeriodicEngine{
	void openAndCheck() {
		assert(!out.is_open());

		std::string fileTemp = file;
		if (addIterNum) fileTemp+="-" + std::to_string(scene->iter);

		if(fileTemp.empty()) throw ios_base::failure(__FILE__ ": Empty filename.");
		out.open(fileTemp.c_str(), truncate ? fstream::trunc : fstream::app);
		if(!out.good()) throw ios_base::failure(__FILE__ ": I/O error opening file `"+fileTemp+"'.");
	}
	protected:
		//! stream object that derived engines should write to
		std::ofstream out;
	public:
		std::string file;
		bool truncate;
		bool addIterNum;
		
		virtual ~Recorder() {};
		virtual bool isActivated() override {
			if(PeriodicEngine::isActivated()){
				if(!out.is_open()) openAndCheck();
				return true;
			}
			return false;
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Recorder, PeriodicEngine, std::shared_ptr<Recorder>> _classObj(_module, "Recorder", "Engine periodically storing some data to (one) external file. In addition PeriodicEngine, it handles opening the file as needed. See :yref:`PeriodicEngine` for controlling periodicity.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("file", &Recorder::file, "Name of file to save to; must not be empty.");
			_classObj.def_readwrite("truncate", &Recorder::truncate, "Whether to delete current file contents, if any, when opening (false by default)");
			_classObj.def_readwrite("addIterNum", &Recorder::addIterNum, "Adds an iteration number to the file name, when the file was created. Useful for creating new files at each call (false by default)");
		}
};
REGISTER_SERIALIZABLE_BASE(Recorder, PeriodicEngine);
