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
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(InteractionLoop, GlobalEngine);
