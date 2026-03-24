/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Timing.hpp>
#include<sudodem/lib/base/Logging.hpp>

class Body;
class Scene;

CREATE_LOGGER(Engine);

class Engine: public Serializable{
	public:
		// pointer to the simulation, set at every step by Scene::moveToNextTimeStep
		Scene* scene;
		//! high-level profiling information; not serializable
		TimingInfo timingInfo;
		//! precise profiling information (timing of fragments of the engine)
		shared_ptr<TimingDeltas> timingDeltas;
		virtual ~Engine() {};

		virtual bool isActivated() { return true; };
		virtual void action() {
			LOG_FATAL("Engine "<<getClassName()<<" calling virtual method Engine::action(). Please submit bug report at http://bugs.launchpad.net/sudodem.");
			throw std::logic_error("Engine::action() called.");
		}
	public:
		// py access funcs
		TimingInfo::delta get_execTime() const {return timingInfo.nsec;};
		void set_execTime(TimingInfo::delta d){ timingInfo.nsec=d;}
		long get_execCount() const {return timingInfo.nExec;};
		void set_execCount(long d){ timingInfo.nExec=d;}
		void call(){ scene=Omega::instance().getScene().get(); action(); };

	DECLARE_LOGGER;

	// Attribute declarations for serialization
	bool dead; int ompThreads; string label;

	Engine() : dead(false), ompThreads(-1), label(""), scene(Omega::instance().getScene().get()) {
		#ifdef USE_TIMING_DELTAS
			timingDeltas=shared_ptr<TimingDeltas>(new TimingDeltas);
		#endif
	}

	// Cereal serialization
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int version) {
		ar(cereal::base_class<Serializable>(this));
		ar(CEREAL_NVP(dead));
		ar(CEREAL_NVP(ompThreads));
		ar(CEREAL_NVP(label));
		// Note: scene, timingInfo, timingDeltas are not serialized (runtime state)
	}

	virtual void pyRegisterClass(pybind11::module_ _module) {
		checkPyClassRegistersItself("Engine");
		pybind11::class_<Engine, Serializable, std::shared_ptr<Engine>> _classObj(_module, "Engine", "Basic execution unit of simulation, called from the simulation loop (O.engines)");
		_classObj.def(pybind11::init<>());
		// Register attributes (using def_readwrite for member pointers)
		_classObj.def_readwrite("dead", &Engine::dead, "If true, this engine will not run at all; can be used for making an engine temporarily deactivated and only resurrect it at a later point.");
		_classObj.def_readwrite("ompThreads", &Engine::ompThreads, "Number of threads to be used in the engine. If ompThreads<0 (default), the number will be typically OMP_NUM_THREADS or the number N defined by 'sudodem -jN' (this behavior can depend on the engine though). This attribute will only affect engines whose code includes openMP parallel regions (e.g. :yref:`InteractionLoop`). This attribute is mostly useful for experiments or when combining :yref:`ParallelEngine` with engines that run parallel regions, resulting in nested OMP loops with different number of threads at each level.");
		_classObj.def_readwrite("label", &Engine::label, "Textual label for this object; must be valid python identifier, you can refer to it directly from python.");
		// Python-specific properties (using def_property for getter/setter methods)
		_classObj.def_property("execTime", &Engine::get_execTime, &Engine::set_execTime, "Cummulative time this Engine took to run (only used if :yref:`O.timingEnabled<Omega.timingEnabled>`\\ ==\\ ``True``).");
		_classObj.def_property("execCount", &Engine::get_execCount, &Engine::set_execCount, "Cummulative count this engine was run (only used if :yref:`O.timingEnabled<Omega.timingEnabled>`\\ ==\\ ``True``).");
		_classObj.def_readonly("timingDeltas", &Engine::timingDeltas, "Detailed information about timing inside the Engine itself. Empty unless enabled in the source code and :yref:`O.timingEnabled<Omega.timingEnabled>`\\ ==\\ ``True``.");
		_classObj.def("call", &Engine::call);
	}
};
REGISTER_SERIALIZABLE(Engine);



