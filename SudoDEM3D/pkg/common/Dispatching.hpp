#pragma once
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/IPhys.hpp>
#include<sudodem/core/Functor.hpp>
#include<sudodem/core/Dispatcher.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<pybind11/pybind11.h>

/********
	functors
*********************/

class BoundFunctor: public Functor1D<
	/*dispatch types*/ Shape,
	/*return type*/    void ,
	/*argument types*/ TYPELIST_4(const shared_ptr<Shape>&, shared_ptr<Bound>&, const Se3r&, const Body*)
>{
	public: virtual ~BoundFunctor();
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(BoundFunctor, Functor);


class IGeomFunctor: public Functor2D<
	/*dispatch types*/ Shape,Shape,
	/*return type*/    bool,
	/*argument types*/ TYPELIST_7(const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector3r&, const bool&, const shared_ptr<Interaction>&)
>{
	public: virtual ~IGeomFunctor();
	virtual void preStep(){};
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(IGeomFunctor, Functor);


class IPhysFunctor: public Functor2D<
	/*dispatch types*/ Material, Material,
	/*retrun type*/    void,
	/*argument types*/ TYPELIST_3(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&)
>{
	public: virtual ~IPhysFunctor();
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(IPhysFunctor, Functor);


class LawFunctor: public Functor2D<
	/*dispatch types*/ IGeom,IPhys,
	/*return type*/    bool,
	/*argument types*/ TYPELIST_3(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*)
>{
	public: virtual ~LawFunctor();
	virtual void preStep(){};
	void addForce (const Body::id_t id, const Vector3r& f,Scene* rb){rb->forces.addForce (id,f);}
	void addTorque(const Body::id_t id, const Vector3r& t,Scene* rb){rb->forces.addTorque(id,t);}
	void applyForceAtContactPoint(const Vector3r& force, const Vector3r& contactPoint, const Body::id_t id1, const Vector3r& pos1, const Body::id_t id2, const Vector3r& pos2){
		Vector3r d1 = contactPoint-pos1;
		Vector3r d2 = -(contactPoint-pos2);
		Vector3r t = d1.cross(force);
		addForce(id1, force,scene); addTorque(id1, t,scene);
		addForce(id2,-force,scene); addTorque(id2,-d2.cross(force),scene);
	}
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(LawFunctor, Functor);


/********
	dispatchers
*********************/

class BoundDispatcher: public Dispatcher1D<
	/* functor type*/ BoundFunctor
>{
	public:
		typedef BoundFunctor FunctorType;
		std::vector<shared_ptr<BoundFunctor>> functors;
		bool activated = true;
		Real sweepDist = 0;
		Real minSweepDistFactor = 0.2;
		Real updatingDispFactor = -1;
		Real targetInterv = -1;
		int ompThreads = 0;

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Dispatcher, functors, activated, sweepDist, minSweepDistFactor, updatingDispFactor, targetInterv, ompThreads);

void updateScenePtr(){
			for(const auto& f : functors){ f->scene=scene; }
			// FIX: Update scene pointer in DynLibDispatcher's callBacks array as well
			dispatcherBase::updateScenePtr(scene);
		}
		void postLoad(BoundDispatcher&){ clearMatrix(); for(const auto& f : functors) add(SUDODEM_PTR_CAST<BoundFunctor>(f)); }
		virtual void add(BoundFunctor* f){ add(shared_ptr<BoundFunctor>(f)); }
		SUDODEM_NOINLINE virtual void add(shared_ptr<BoundFunctor> f){ static volatile int unique_id = 45; (void)unique_id; bool dupe=false; string fn=f->getClassName(); for(const auto& f2 : functors) { if(fn==f2->getClassName()) dupe=true; } if(!dupe) functors.push_back(f); addFunctor(f); }
		SUDODEM_NOINLINE virtual void addFunctor(shared_ptr<BoundFunctor> f){ add1DEntry(f->get1DFunctorType1(),f); }
		pybind11::list functors_get(void) const { pybind11::list ret; for(const auto& f : functors){ ret.append(f); } return ret; }
		void functors_set(const std::vector<shared_ptr<BoundFunctor>>& ff){ functors.clear(); for(const auto& f : ff) add(f); postLoad(*this); }
		void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override{ if(pybind11::len(t)==0)return; if(pybind11::len(t)!=1) throw invalid_argument("Exactly one list of BoundFunctor must be given."); typedef std::vector<shared_ptr<BoundFunctor>> vecF; vecF vf=t[0].cast<vecF>(); functors_set(vf); t=pybind11::tuple(); }

	public:
		virtual void action() override;
		virtual bool isActivated() override{ return activated; }
		void processBody(const shared_ptr<Body>&);
		DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(BoundDispatcher, Dispatcher);


class IGeomDispatcher:	public Dispatcher2D<
	/* functor type*/ IGeomFunctor,
	/* autosymmetry*/ false
>{
	public:
		typedef IGeomFunctor FunctorType;
		std::vector<shared_ptr<IGeomFunctor>> functors;
		bool alreadyWarnedNoCollider = false;

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Dispatcher, functors, alreadyWarnedNoCollider);

		void updateScenePtr(){
			for(const auto& f : functors){ f->scene=scene; }
			// FIX: Update scene pointer in DynLibDispatcher's callBacks matrix as well
			dispatcherBase::updateScenePtr(scene);
		}
		void postLoad(IGeomDispatcher&){ clearMatrix(); for(const auto& f : functors) add(SUDODEM_PTR_CAST<IGeomFunctor>(f)); }
		virtual void add(IGeomFunctor* f){ add(shared_ptr<IGeomFunctor>(f)); }
		SUDODEM_NOINLINE virtual void add(shared_ptr<IGeomFunctor> f){ static volatile int unique_id = 43; (void)unique_id; bool dupe=false; string fn=f->getClassName(); for(const auto& f2 : functors) { if(fn==f2->getClassName()) dupe=true; } if(!dupe) functors.push_back(f); addFunctor(f); }
		SUDODEM_NOINLINE virtual void addFunctor(shared_ptr<IGeomFunctor> f){ add2DEntry(f->get2DFunctorType1(),f->get2DFunctorType2(),f); }
		pybind11::list functors_get(void) const { pybind11::list ret; for(const auto& f : functors){ ret.append(f); } return ret; }
		void functors_set(const std::vector<shared_ptr<IGeomFunctor>>& ff){ functors.clear(); for(const auto& f : ff) add(f); postLoad(*this); }
		void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override{ if(pybind11::len(t)==0)return; if(pybind11::len(t)!=1) throw invalid_argument("Exactly one list of IGeomFunctor must be given."); typedef std::vector<shared_ptr<IGeomFunctor>> vecF; vecF vf=t[0].cast<vecF>(); functors_set(vf); t=pybind11::tuple(); }

	public:
		virtual void action() override;
		shared_ptr<Interaction> explicitAction(const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool force);
		DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(IGeomDispatcher, Dispatcher);


class IPhysDispatcher: public Dispatcher2D<
	/*functor type*/ IPhysFunctor
>{
	public:
		typedef IPhysFunctor FunctorType;
		std::vector<shared_ptr<IPhysFunctor>> functors;

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Dispatcher, functors);

		void updateScenePtr(){
			for(const auto& f : functors){ f->scene=scene; }
			// FIX: Update scene pointer in DynLibDispatcher's callBacks matrix as well
			dispatcherBase::updateScenePtr(scene);
		}
		void postLoad(IPhysDispatcher&){ clearMatrix(); for(const auto& f : functors) add(SUDODEM_PTR_CAST<IPhysFunctor>(f)); }
		virtual void add(IPhysFunctor* f){ add(shared_ptr<IPhysFunctor>(f)); }
		SUDODEM_NOINLINE virtual void add(shared_ptr<IPhysFunctor> f){ static volatile int unique_id = 44; (void)unique_id; bool dupe=false; string fn=f->getClassName(); for(const auto& f2 : functors) { if(fn==f2->getClassName()) dupe=true; } if(!dupe) functors.push_back(f); addFunctor(f); }
		SUDODEM_NOINLINE virtual void addFunctor(shared_ptr<IPhysFunctor> f){ add2DEntry(f->get2DFunctorType1(),f->get2DFunctorType2(),f); }
		pybind11::list functors_get(void) const { pybind11::list ret; for(const auto& f : functors){ ret.append(f); } return ret; }
		void functors_set(const std::vector<shared_ptr<IPhysFunctor>>& ff){ functors.clear(); for(const auto& f : ff) add(f); postLoad(*this); }
		void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override{ if(pybind11::len(t)==0)return; if(pybind11::len(t)!=1) throw invalid_argument("Exactly one list of IPhysFunctor must be given."); typedef std::vector<shared_ptr<IPhysFunctor>> vecF; vecF vf=t[0].cast<vecF>(); functors_set(vf); t=pybind11::tuple(); }

	public:
		virtual void action() override;
		void explicitAction(shared_ptr<Material>& pp1, shared_ptr<Material>& pp2, shared_ptr<Interaction>& i);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(IPhysDispatcher, Dispatcher);


class LawDispatcher: public Dispatcher2D<
	/*functor type*/ LawFunctor,
	/*autosymmetry*/ false
>{
	public:
		typedef LawFunctor FunctorType;
		std::vector<shared_ptr<LawFunctor>> functors;

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Dispatcher, functors);

		void updateScenePtr(){
			for(const auto& f : functors){ f->scene=scene; }
			// FIX: Update scene pointer in DynLibDispatcher's callBacks matrix as well
			dispatcherBase::updateScenePtr(scene);
		}
		void postLoad(LawDispatcher&){ clearMatrix(); for(const auto& f : functors) add(SUDODEM_PTR_CAST<LawFunctor>(f)); }
		virtual void add(LawFunctor* f){ add(shared_ptr<LawFunctor>(f)); }
		SUDODEM_NOINLINE virtual void add(shared_ptr<LawFunctor> f){ static volatile int unique_id = 42; (void)unique_id; bool dupe=false; string fn=f->getClassName(); for(const auto& f2 : functors) { if(fn==f2->getClassName()) dupe=true; } if(!dupe) functors.push_back(f); addFunctor(f); }
		SUDODEM_NOINLINE virtual void addFunctor(shared_ptr<LawFunctor> f){ add2DEntry(f->get2DFunctorType1(),f->get2DFunctorType2(),f); }
		pybind11::list functors_get(void) const { pybind11::list ret; for(const auto& f : functors){ ret.append(f); } return ret; }
		void functors_set(const std::vector<shared_ptr<LawFunctor>>& ff){ functors.clear(); for(const auto& f : ff) add(f); postLoad(*this); }
		void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override{ if(pybind11::len(t)==0)return; if(pybind11::len(t)!=1) throw invalid_argument("Exactly one list of LawFunctor must be given."); typedef std::vector<shared_ptr<LawFunctor>> vecF; vecF vf=t[0].cast<vecF>(); functors_set(vf); t=pybind11::tuple(); }

	public: virtual void action() override;
	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(LawDispatcher, Dispatcher);