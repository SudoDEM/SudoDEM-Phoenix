// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
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
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Recorder, PeriodicEngine);
