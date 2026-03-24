// 2008, 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once

#include<time.h>
//CH_BUG_FIX
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
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PeriodicEngine, GlobalEngine);
