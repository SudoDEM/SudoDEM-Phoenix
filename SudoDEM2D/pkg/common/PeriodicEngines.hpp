// 2008, 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once

#include<time.h>
#include <chrono>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Scene.hpp>

class PeriodicEngine:  public GlobalEngine {
	public:
		static Real getClock() {
			using namespace std::chrono;
			return static_cast<Real>(
				duration_cast<duration<Real>>(
					system_clock::now().time_since_epoch()
				).count()
				);
		}
		virtual ~PeriodicEngine() {}; // vtable
		
		Real virtPeriod;
		Real realPeriod;
		long iterPeriod;
		long nDo;
		bool initRun;
		Real virtLast;
		Real realLast;
		long iterLast;
		long nDone;

		virtual bool isActivated() override{
			const Real& virtNow=scene->time;
			Real realNow=getClock();
			const long& iterNow=scene->iter;
			if (iterNow<iterLast) nDone=0;//handle O.resetTime(), all counters will be initialized again
			if((nDo<0 || nDone<nDo) &&
				((virtPeriod>0 && virtNow-virtLast>=virtPeriod) ||
				 (realPeriod>0 && realNow-realLast>=realPeriod) ||
				 (iterPeriod>0 && iterNow-iterLast>=iterPeriod))){
				realLast=realNow; virtLast=virtNow; iterLast=iterNow; nDone++;
				return true;
			}
			if(nDone==0){
				realLast=realNow; virtLast=virtNow; iterLast=iterNow; nDone++;
				if(initRun) return true;
				return false;
			}
			return false;
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<PeriodicEngine, GlobalEngine, std::shared_ptr<PeriodicEngine>> _classObj(_module, "PeriodicEngine", 
				"Run Engine::action with given fixed periodicity real time (=wall clock time, computation time), \
				virtual time (simulation time), iteration number), by setting any of those criteria \
				(virtPeriod, realPeriod, iterPeriod) to a positive value. They are all negative (inactive)\
				by default.\n\n\
				\
				The number of times this engine is activated can be limited by setting nDo>0. If the number of activations \
				will have been already reached, no action will be called even if an active period has elapsed. \n\n\
				\
				If initRun is set (false by default), the engine will run when called for the first time; otherwise it will only \
				start counting period (realLast etc interal variables) from that point, but without actually running, and will run \
				only once a period has elapsed since the initial run. \n\n\
				\
				This class should not be used directly; rather, derive your own engine which you want to be run periodically. \n\n\
				\
				Derived engines should override Engine::action(), which will be called periodically. If the derived Engine \
				overrides also Engine::isActivated, it should also take in account return value from PeriodicEngine::isActivated, \
				since otherwise the periodicity will not be functional. \n\n\
				\
				Example with :yref:`PyRunner`, which derives from PeriodicEngine; likely to be encountered in python scripts:: \n\n\
				\
					PyRunner(realPeriod=5,iterPeriod=10000,command='print O.iter')	\n\n\
				\
				will print iteration number every 10000 iterations or every 5 seconds of wall clock time, whiever comes first since it was \
				last run.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("virtPeriod", &PeriodicEngine::virtPeriod, "Periodicity criterion using virtual (simulation) time (deactivated if <= 0)");
			_classObj.def_readwrite("realPeriod", &PeriodicEngine::realPeriod, "Periodicity criterion using real (wall clock, computation, human) time (deactivated if <=0)");
			_classObj.def_readwrite("iterPeriod", &PeriodicEngine::iterPeriod, "Periodicity criterion using step number (deactivated if <= 0)");
			_classObj.def_readwrite("nDo", &PeriodicEngine::nDo, "Limit number of executions by this number (deactivated if negative)");
			_classObj.def_readwrite("initRun", &PeriodicEngine::initRun, "Run the first time we are called as well.");
			_classObj.def_readwrite("virtLast", &PeriodicEngine::virtLast, "Tracks virtual time of last run |yupdate|.");
			_classObj.def_readwrite("realLast", &PeriodicEngine::realLast, "Tracks real time of last run |yupdate|.");
			_classObj.def_readwrite("iterLast", &PeriodicEngine::iterLast, "Tracks step number of last run |yupdate|.");
			_classObj.def_readwrite("nDone", &PeriodicEngine::nDone, "Track number of executions (cummulative) |yupdate|.");
			_classObj.def("isActivated", &PeriodicEngine::isActivated);
			_classObj.def("getClock", &PeriodicEngine::getClock);
		}
};
REGISTER_SERIALIZABLE_BASE(PeriodicEngine, GlobalEngine);
