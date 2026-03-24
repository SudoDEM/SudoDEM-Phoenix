#include"Superellipse.hpp"
//#include<sudodem/pkg/common/Sphere.hpp>
//#include<sudodem/pkg/common/Box.hpp>

//***************************************************************************
/*! Create Superellipse (collision geometry) from colliding Superellipses. */
class Ig2_Superellipse_Superellipse_SuperellipseGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Superellipse_Superellipse_SuperellipseGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c) override;
		FUNCTOR2D(Superellipse,Superellipse);
		DEFINE_FUNCTOR_ORDER_2D(Superellipse,Superellipse);
		
		Ig2_Superellipse_Superellipse_SuperellipseGeom() {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Superellipse_Superellipse_SuperellipseGeom, IGeomFunctor, std::shared_ptr<Ig2_Superellipse_Superellipse_SuperellipseGeom>> _classObj(_module, "Ig2_Superellipse_Superellipse_SuperellipseGeom", "Create/update geometry of collision between 2 Superellipses");
			_classObj.def(pybind11::init<>());
		}
	DECLARE_LOGGER;
	REGISTER_CLASS_NAME_DERIVED(Ig2_Superellipse_Superellipse_SuperellipseGeom);
	private:
};
REGISTER_SERIALIZABLE_BASE(Ig2_Superellipse_Superellipse_SuperellipseGeom, IGeomFunctor);
//***************************************************************************
/*! Create Superellipse (collision geometry) from colliding Wall & Superellipse. */
class Ig2_Wall_Superellipse_SuperellipseGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Wall_Superellipse_SuperellipseGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c) override;
		FUNCTOR2D(Wall,Superellipse);
		DEFINE_FUNCTOR_ORDER_2D(Wall,Superellipse);
		
		Ig2_Wall_Superellipse_SuperellipseGeom() {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Wall_Superellipse_SuperellipseGeom, IGeomFunctor, std::shared_ptr<Ig2_Wall_Superellipse_SuperellipseGeom>> _classObj(_module, "Ig2_Wall_Superellipse_SuperellipseGeom", "Create/update geometry of collision between Wall and Superellipse");
			_classObj.def(pybind11::init<>());
		}
	DECLARE_LOGGER;
	REGISTER_CLASS_NAME_DERIVED(Ig2_Wall_Superellipse_SuperellipseGeom);
	private:
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Superellipse_SuperellipseGeom, IGeomFunctor);
//***************************************************************************
class Ig2_Fwall_Superellipse_SuperellipseGeom : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1,const shared_ptr<Shape>& cm2,const State& state1,const State& state2,const Vector2r& shift2,const bool& force,
					const shared_ptr<Interaction>& c) override;
		virtual bool goReverse(	const shared_ptr<Shape>& cm1,const shared_ptr<Shape>& cm2,const State& state1,const State& state2,const Vector2r& shift2,const bool& force,
					const shared_ptr<Interaction>& c) override;
		FUNCTOR2D(Fwall,Superellipse);
		
		Ig2_Fwall_Superellipse_SuperellipseGeom() {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Fwall_Superellipse_SuperellipseGeom, IGeomFunctor, std::shared_ptr<Ig2_Fwall_Superellipse_SuperellipseGeom>> _classObj(_module, "Ig2_Fwall_Superellipse_SuperellipseGeom", "Create/update a :yref:`ScGeom` instance representing intersection of :yref:`Fwall` and :yref:`Disk`.");
			_classObj.def(pybind11::init<>());
		}
	DECLARE_LOGGER;
	DEFINE_FUNCTOR_ORDER_2D(Fwall,Superellipse);
	REGISTER_CLASS_NAME_DERIVED(Ig2_Fwall_Superellipse_SuperellipseGeom);
};

REGISTER_SERIALIZABLE_BASE(Ig2_Fwall_Superellipse_SuperellipseGeom, IGeomFunctor);
//***************************************************************************
/*! Create Superellipse (collision geometry) from colliding Facet & Superellipse. */
/*
class Ig2_Facet_Superellipse_SuperellipseGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Facet_Superellipse_SuperellipseGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		DEFINE_FUNCTOR_ORDER_2D(Facet,Superellipse);
	DECLARE_LOGGER;
	private:
};
REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Superellipse_SuperellipseGeom, IGeomFunctor);

*/