// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2006 Janek Kozicki <cosurgi@berlios.de>

#pragma once

#include<sudodem/lib/multimethods/FunctorWrapper.hpp>
#include<sudodem/core/Bound.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/Functor.hpp>
#include<sudodem/core/Dispatcher.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/IPhys.hpp>
#include<pybind11/pybind11.h>
#include<sudodem/lib/base/PreprocessorUtils.hpp>

#define RENDERS(name) public: virtual string renders() const override { return #name;}; FUNCTOR1D(name);

struct GLViewInfo{
	GLViewInfo(): sceneCenter(Vector3r::Zero()), sceneRadius(1.){}
	Vector3r sceneCenter;
	Real sceneRadius;
};

class OpenGLRenderer;

#define GL_FUNCTOR(Klass,typelist,renderedType) class Klass: public Functor1D<renderedType,void,typelist>{public:\
	virtual ~Klass(){};\
	virtual string renders() const { throw std::runtime_error(#Klass ": unregistered gldraw class.\n"); };\
	virtual void initgl() {/*WARNING: it must deal with static members, because it is called from another instance!*/};\
	public: virtual void pyRegisterClass(pybind11::module_ _module) override {\
		checkPyClassRegistersItself(#Klass);\
		pybind11::class_<Klass, Functor, std::shared_ptr<Klass>> _classObj(_module, #Klass, "Abstract functor for rendering :yref:`" #renderedType "` objects.");\
		_classObj.def(pybind11::init<>());\
	}\
	}; REGISTER_SERIALIZABLE_BASE(Klass, Functor);

GL_FUNCTOR(GlBoundFunctor,TYPELIST_2(const shared_ptr<Bound>&, Scene*),Bound);
GL_FUNCTOR(GlShapeFunctor,TYPELIST_4(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&),Shape);
GL_FUNCTOR(GlIGeomFunctor,TYPELIST_5(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool),IGeom);
GL_FUNCTOR(GlIPhysFunctor,TYPELIST_5(const shared_ptr<IPhys>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool),IPhys);
GL_FUNCTOR(GlStateFunctor,TYPELIST_1(const shared_ptr<State>&),State);

#define GL_DISPATCHER(Klass,Functor) class Klass: public Dispatcher1D<Functor>{\
	public:\
		typedef Functor FunctorType;\
		std::vector<shared_ptr<Functor>> functors;\
		void updateScenePtr(){ for(const auto& f : functors){ f->scene=scene; }}\
		void postLoad(Klass&){ clearMatrix(); for(const auto& f : functors) add(SUDODEM_PTR_CAST<Functor>(f)); }\
		__attribute__((noinline)) virtual void add(Functor* f){ add(shared_ptr<Functor>(f)); }\
		__attribute__((noinline)) virtual void add(shared_ptr<Functor> f){ static volatile int unique_id = __LINE__; (void)unique_id; bool dupe=false; string fn=f->getClassName(); for(const auto& f2 : functors) { if(fn==f2->getClassName()) dupe=true; } if(!dupe) functors.push_back(f); addFunctor(f); }\
		__attribute__((noinline)) virtual void addFunctor(shared_ptr<Functor> f){ add1DEntry(f->get1DFunctorType1(),f); }\
		pybind11::list functors_get(void) const { pybind11::list ret; for(const auto& f : functors){ ret.append(f); } return ret; }\
		void functors_set(const std::vector<shared_ptr<Functor>>& ff){ functors.clear(); for(const auto& f : ff) add(f); postLoad(*this); }\
		void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override{ if(pybind11::len(t)==0)return; if(pybind11::len(t)!=1) throw invalid_argument("Exactly one list of " SUDODEM_STRINGIZE(Functor) " must be given."); typedef std::vector<shared_ptr<Functor>> vecF; vecF vf=t[0].cast<vecF>(); functors_set(vf); t=pybind11::tuple(); }\
	public:\
		virtual void pyRegisterClass(pybind11::module_ _module) override {\
			checkPyClassRegistersItself(#Klass);\
			pybind11::class_<Klass, Dispatcher, std::shared_ptr<Klass>> _classObj(_module, #Klass, "Dispatcher calling :yref:`functors<" SUDODEM_STRINGIZE(Functor) ">` based on received argument type(s).");\
			_classObj.def(pybind11::init<>());\
			_classObj.def_property("functors", &Klass::functors_get, &Klass::functors_set, "Functors associated with this dispatcher.");\
			_classObj.def("dispMatrix", &Klass::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.");\
			_classObj.def("dispFunctor", &Klass::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");\
		}\
	}; REGISTER_SERIALIZABLE_BASE(Klass, Dispatcher);

GL_DISPATCHER(GlBoundDispatcher,GlBoundFunctor);
GL_DISPATCHER(GlShapeDispatcher,GlShapeFunctor);
GL_DISPATCHER(GlIGeomDispatcher,GlIGeomFunctor);
GL_DISPATCHER(GlIPhysDispatcher,GlIPhysFunctor);
GL_DISPATCHER(GlStateDispatcher,GlStateFunctor);
#undef GL_FUNCTOR
#undef GL_DISPATCHER