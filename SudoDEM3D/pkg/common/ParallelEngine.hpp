#pragma once
#include<sudodem/core/GlobalEngine.hpp>

class ParallelEngine: public Engine {
	public:
		typedef vector<vector<shared_ptr<Engine> > > slaveContainer;
		slaveContainer slaves;
		int ompThreads;
		
		virtual void action() override;
		virtual bool isActivated() override{return true;}
	// py access
		pybind11::list slaves_get();
		void slaves_set(const pybind11::list& slaves);
		
		ParallelEngine() : ompThreads(2) {}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ParallelEngine, Engine);


