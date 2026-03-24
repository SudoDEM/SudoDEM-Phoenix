#include<sudodem/pkg/dem/Superquadrics.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/pkg/common/Box.hpp>

class Ig2_Superquadrics_Superquadrics_SuperquadricsGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Superquadrics_Superquadrics_SuperquadricsGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		FUNCTOR2D(Superquadrics,Superquadrics);
		DEFINE_FUNCTOR_ORDER_2D(Superquadrics,Superquadrics);

	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Superquadrics_Superquadrics_SuperquadricsGeom, IGeomFunctor);

class Ig2_Superquadrics_Superquadrics_SuperquadricsGeom2: public IGeomFunctor
{
	public:
		virtual ~Ig2_Superquadrics_Superquadrics_SuperquadricsGeom2(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		FUNCTOR2D(Superquadrics,Superquadrics);
		DEFINE_FUNCTOR_ORDER_2D(Superquadrics,Superquadrics);

	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Superquadrics_Superquadrics_SuperquadricsGeom2, IGeomFunctor);

class Ig2_Wall_Superquadrics_SuperquadricsGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Wall_Superquadrics_SuperquadricsGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		FUNCTOR2D(Wall,Superquadrics);
		DEFINE_FUNCTOR_ORDER_2D(Wall,Superquadrics);

	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Superquadrics_SuperquadricsGeom, IGeomFunctor);

class Ig2_Wall_Superquadrics_SuperquadricsGeom2: public IGeomFunctor
{
	public:
		virtual ~Ig2_Wall_Superquadrics_SuperquadricsGeom2(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		FUNCTOR2D(Wall,Superquadrics);
		DEFINE_FUNCTOR_ORDER_2D(Wall,Superquadrics);

	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Superquadrics_SuperquadricsGeom2, IGeomFunctor);

class Ig2_Facet_Superquadrics_SuperquadricsGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Facet_Superquadrics_SuperquadricsGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		FUNCTOR2D(Facet,Superquadrics);
		DEFINE_FUNCTOR_ORDER_2D(Facet,Superquadrics);

	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Superquadrics_SuperquadricsGeom, IGeomFunctor);