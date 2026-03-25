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
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<ParallelEngine, Engine, std::shared_ptr<ParallelEngine>> _classObj(_module, "ParallelEngine", "Engine for running other Engine in parallel.");
			_classObj.def(pybind11::init<>());
			_classObj.def(pybind11::init([](const pybind11::list& slaves2){
				shared_ptr<ParallelEngine> instance(new ParallelEngine);
				instance->slaves_set(slaves2);
				return instance;
			}), "Construct from (possibly nested) list of slaves.");
			_classObj.def_property("slaves", &ParallelEngine::slaves_get, &ParallelEngine::slaves_set, "List of lists of Engines; each top-level group will be run in parallel with other groups, while Engines inside each group will be run sequentially, in given order.");
		}
};
REGISTER_SERIALIZABLE_BASE(ParallelEngine, Engine);


