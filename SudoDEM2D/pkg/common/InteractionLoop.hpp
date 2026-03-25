// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/pkg/common/Callbacks.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>

#ifdef USE_TIMING_DELTAS
	#define TIMING_DELTAS_CHECKPOINT(cpt) timingDeltas->checkpoint(cpt)
	#define TIMING_DELTAS_START() timingDeltas->start()
#else
	#define TIMING_DELTAS_CHECKPOINT(cpt)
	#define TIMING_DELTAS_START()
#endif

class InteractionLoop: public GlobalEngine {
	bool alreadyWarnedNoCollider;
	typedef std::pair<Body::id_t, Body::id_t> idPair;
	// store interactions that should be deleted after loop in action, not later
	#ifdef SUDODEM_OPENMP
		std::vector<std::list<idPair> > eraseAfterLoopIds;
		void eraseAfterLoop(Body::id_t id1,Body::id_t id2){ eraseAfterLoopIds[omp_get_thread_num()].push_back(idPair(id1,id2)); }
	#else
		list<idPair> eraseAfterLoopIds;
		void eraseAfterLoop(Body::id_t id1,Body::id_t id2){ eraseAfterLoopIds.push_back(idPair(id1,id2)); }
	#endif
	
	public:
		shared_ptr<IGeomDispatcher> geomDispatcher;
		shared_ptr<IPhysDispatcher> physDispatcher;
		shared_ptr<LawDispatcher> lawDispatcher;
		vector<shared_ptr<IntrCallback> > callbacks;
		bool eraseIntsInLoop;

		virtual void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override;
		virtual void action() override;
		
		InteractionLoop() : geomDispatcher(new IGeomDispatcher), physDispatcher(new IPhysDispatcher), lawDispatcher(new LawDispatcher), eraseIntsInLoop(false), alreadyWarnedNoCollider(false) {
			#ifdef SUDODEM_OPENMP
				eraseAfterLoopIds.resize(omp_get_max_threads());
			#endif
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlobalEngine, geomDispatcher, physDispatcher, lawDispatcher, callbacks, eraseIntsInLoop);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<InteractionLoop, GlobalEngine, std::shared_ptr<InteractionLoop>> _classObj(_module, "InteractionLoop", "Unified dispatcher for handling interaction loop at every step, for parallel performance reasons.\n\n.. admonition:: Special constructor\n\n\tConstructs from 3 lists of :yref:`Ig2<IGeomFunctor>`, :yref:`Ip2<IPhysFunctor>`, :yref:`Law<LawFunctor>` functors respectively; they will be passed to interal dispatchers, which you might retrieve.");
			// Default constructor
			_classObj.def(pybind11::init<>());
			// Constructor with 3 functor lists
			_classObj.def(pybind11::init([](pybind11::list geomFunctors, pybind11::list physFunctors, pybind11::list lawFunctors) {
				auto loop = std::make_shared<InteractionLoop>();
				// Process IGeomFunctor list
				typedef std::vector<shared_ptr<IGeomFunctor>> vecGeom;
				vecGeom vg = pybind11::cast<vecGeom>(geomFunctors);
				for (auto gf : vg) loop->geomDispatcher->add(gf);
				// Process IPhysFunctor list
				typedef std::vector<shared_ptr<IPhysFunctor>> vecPhys;
				vecPhys vp = pybind11::cast<vecPhys>(physFunctors);
				for (auto pf : vp) loop->physDispatcher->add(pf);
				// Process LawFunctor list
				typedef std::vector<shared_ptr<LawFunctor>> vecLaw;
				vecLaw vl = pybind11::cast<vecLaw>(lawFunctors);
				for (auto lf : vl) loop->lawDispatcher->add(lf);
				return loop;
			}), pybind11::arg("geomFunctors"), pybind11::arg("physFunctors"), pybind11::arg("lawFunctors"));
			_classObj.def_readonly("geomDispatcher", &InteractionLoop::geomDispatcher, ":yref:`IGeomDispatcher` object that is used for dispatch.");
			_classObj.def_readonly("physDispatcher", &InteractionLoop::physDispatcher, ":yref:`IPhysDispatcher` object used for dispatch.");
			_classObj.def_readonly("lawDispatcher", &InteractionLoop::lawDispatcher, ":yref:`LawDispatcher` object used for dispatch.");
			_classObj.def_readwrite("callbacks", &InteractionLoop::callbacks, ":yref:`Callbacks<IntrCallback>` which will be called for every :yref:`Interaction`, if activated.");
			_classObj.def_readwrite("eraseIntsInLoop", &InteractionLoop::eraseIntsInLoop, "Defines if the interaction loop should erase pending interactions, else the collider takes care of that alone (depends on what collider is used).");
		}
		DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(InteractionLoop, GlobalEngine);
