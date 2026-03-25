#pragma once
#include <sudodem/lib/base/Math.hpp>
#include <sudodem/core/Shape.hpp>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/core/Scene.hpp>
#include <sudodem/core/State.hpp>
#include <sudodem/core/Shape.hpp>
#include <sudodem/core/IGeom.hpp>
#include <sudodem/core/IPhys.hpp>
#include <sudodem/core/Functor.hpp>
#include <sudodem/core/Dispatcher.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include <pybind11/pybind11.h>

/********
	functors
*********************/

class BoundFunctor: public Functor1D<
	/*dispatch types*/ Shape,
	/*return type*/    void ,
	/*argument types*/ TYPELIST_4(const shared_ptr<Shape>&, shared_ptr<Bound>&, const Se2r&, const Body*)
>{
	public: virtual ~BoundFunctor();
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("BoundFunctor");
		pybind11::class_<BoundFunctor, Functor1D<Shape,void,TYPELIST_4(const shared_ptr<Shape>&, shared_ptr<Bound>&, const Se2r&, const Body*)>, std::shared_ptr<BoundFunctor>> _classObj(_module, "BoundFunctor", "Functor for creating/updating :yref:`Body::bound`.");
		_classObj.def(pybind11::init<>());
	}
};
REGISTER_SERIALIZABLE_BASE(BoundFunctor, Functor);


class IGeomFunctor: public Functor2D<
	/*dispatch types*/ Shape,Shape,
	/*return type*/    bool,
	/*argument types*/ TYPELIST_7(const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector2r&, const bool&, const shared_ptr<Interaction>&)
>{
	public: virtual ~IGeomFunctor();
	virtual void preStep(){};
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("IGeomFunctor");
		pybind11::class_<IGeomFunctor, Functor2D<Shape,Shape,bool,TYPELIST_7(const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector2r&, const bool&, const shared_ptr<Interaction>&)>, std::shared_ptr<IGeomFunctor>> _classObj(_module, "IGeomFunctor", "Functor for creating/updating :yref:`Interaction::geom` objects.");
		_classObj.def(pybind11::init<>());
		_classObj.def("preStep", &IGeomFunctor::preStep, "called before every step once, from InteractionLoop (used to set Scene::flags & Scene::LOCAL_COORDS)");
	}
};
REGISTER_SERIALIZABLE_BASE(IGeomFunctor, Functor);


class IPhysFunctor: public Functor2D<
	/*dispatch types*/ Material, Material,
	/*retrun type*/    void,
	/*argument types*/ TYPELIST_3(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&)
>{
	public: virtual ~IPhysFunctor();
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("IPhysFunctor");
		pybind11::class_<IPhysFunctor, Functor2D<Material,Material,void,TYPELIST_3(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&)>, std::shared_ptr<IPhysFunctor>> _classObj(_module, "IPhysFunctor", "Functor for creating/updating :yref:`Interaction::phys` objects.");
		_classObj.def(pybind11::init<>());
	}
};
REGISTER_SERIALIZABLE_BASE(IPhysFunctor, Functor);


class LawFunctor: public Functor2D<
	/*dispatch types*/ IGeom,IPhys,
	/*return type*/    bool,
	/*argument types*/ TYPELIST_3(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*)
>{
	public: virtual ~LawFunctor();
	virtual void preStep(){};
	void addForce (const Body::id_t id, const Vector2r& f,Scene* rb){rb->forces.addForce (id,f);}
	void addTorque(const Body::id_t id, const Real& t,Scene* rb){rb->forces.addTorque(id,t);}
	void applyForceAtContactPoint(const Vector2r& force, const Vector2r& contactPoint, const Body::id_t id1, const Vector2r& pos1, const Body::id_t id2, const Vector2r& pos2){
		Vector2r d1 = contactPoint-pos1;
		Vector2r d2 = -(contactPoint-pos2);
		double theta1 = atan2(force[1],force[0]) - atan2(d1[1],d1[0]);
		double t = force.norm()*sin(theta1);
		addForce(id1, force,scene); addTorque(id1, t*d1.norm(),scene);
		addForce(id2,-force,scene); addTorque(id2,-t*d2.norm(),scene);
	}
	public: SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("LawFunctor");
		pybind11::class_<LawFunctor, Functor2D<IGeom,IPhys,bool,TYPELIST_3(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*)>, std::shared_ptr<LawFunctor>> _classObj(_module, "LawFunctor", "Functor for applying constitutive laws on :yref:`interactions<Interaction>`.");
		_classObj.def(pybind11::init<>());
		_classObj.def("preStep", &LawFunctor::preStep, "called before every step once, from InteractionLoop (used to set Scene::flags & Scene::COMPRESSION_NEGATIVE)");
		_classObj.def("addForce", &LawFunctor::addForce, "Convenience functions to get forces/torques quickly.");
		_classObj.def("addTorque", &LawFunctor::addTorque, "Convenience functions to get forces/torques quickly.");
		_classObj.def("applyForceAtContactPoint", &LawFunctor::applyForceAtContactPoint, "Convenience function to apply force and torque from one force at contact point.");
	}
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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("BoundDispatcher");
			pybind11::class_<BoundDispatcher, Dispatcher, std::shared_ptr<BoundDispatcher>> _classObj(_module, "BoundDispatcher", "Dispatcher calling :yref:`functors<BoundFunctor>` based on received argument type(s).");
			_classObj.def(pybind11::init<>());
			_classObj.def("add", static_cast<void (BoundDispatcher::*)(shared_ptr<BoundFunctor>)>(&BoundDispatcher::add), "Add functor to dispatcher.");
			_classObj.def_property("functors", &BoundDispatcher::functors_get, &BoundDispatcher::functors_set, "Functors associated with this dispatcher.");
			_classObj.def("dispMatrix", &BoundDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
			_classObj.def("dispFunctor", &BoundDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
			_classObj.def_readwrite("activated", &BoundDispatcher::activated, "Whether the engine is activated (only should be changed by the collider)");
			_classObj.def_readwrite("sweepDist", &BoundDispatcher::sweepDist, "Distance by which enlarge all bounding boxes, to prevent collider from being run at every step (only should be changed by the collider).");
			_classObj.def_readwrite("minSweepDistFactor", &BoundDispatcher::minSweepDistFactor, "Minimal distance by which enlarge all bounding boxes; superseeds computed value of sweepDist when lower that (minSweepDistFactor x sweepDist). Updated by the collider.");
			_classObj.def_readwrite("updatingDispFactor", &BoundDispatcher::updatingDispFactor, "see :yref:`InsertionSortCollider::updatingDispFactor`");
			_classObj.def_readwrite("targetInterv", &BoundDispatcher::targetInterv, "see :yref:`InsertionSortCollider::targetInterv`");
		}
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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("IGeomDispatcher");
			pybind11::class_<IGeomDispatcher, Dispatcher, std::shared_ptr<IGeomDispatcher>> _classObj(_module, "IGeomDispatcher", "Dispatcher calling :yref:`functors<IGeomFunctor>` based on received argument type(s).");
			_classObj.def(pybind11::init<>());
			_classObj.def("add", static_cast<void (IGeomDispatcher::*)(shared_ptr<IGeomFunctor>)>(&IGeomDispatcher::add), "Add functor to dispatcher.");
			_classObj.def_property("functors", &IGeomDispatcher::functors_get, &IGeomDispatcher::functors_set, "Functors associated with this dispatcher.");
			_classObj.def("dispMatrix", &IGeomDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
			_classObj.def("dispFunctor", &IGeomDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
		}
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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("IPhysDispatcher");
			pybind11::class_<IPhysDispatcher, Dispatcher, std::shared_ptr<IPhysDispatcher>> _classObj(_module, "IPhysDispatcher", "Dispatcher calling :yref:`functors<IPhysFunctor>` based on received argument type(s).");
			_classObj.def(pybind11::init<>());
			_classObj.def("add", static_cast<void (IPhysDispatcher::*)(shared_ptr<IPhysFunctor>)>(&IPhysDispatcher::add), "Add functor to dispatcher.");
			_classObj.def_property("functors", &IPhysDispatcher::functors_get, &IPhysDispatcher::functors_set, "Functors associated with this dispatcher.");
			_classObj.def("dispMatrix", &IPhysDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
			_classObj.def("dispFunctor", &IPhysDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
		}
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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("LawDispatcher");
			pybind11::class_<LawDispatcher, Dispatcher, std::shared_ptr<LawDispatcher>> _classObj(_module, "LawDispatcher", "Dispatcher calling :yref:`functors<LawFunctor>` based on received argument type(s).");
			_classObj.def(pybind11::init<>());
			_classObj.def("add", static_cast<void (LawDispatcher::*)(shared_ptr<LawFunctor>)>(&LawDispatcher::add), "Add functor to dispatcher.");
			_classObj.def_property("functors", &LawDispatcher::functors_get, &LawDispatcher::functors_set, "Functors associated with this dispatcher.");
			_classObj.def("dispMatrix", &LawDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");
			_classObj.def("dispFunctor", &LawDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
		}
};
REGISTER_SERIALIZABLE_BASE(LawDispatcher, Dispatcher);