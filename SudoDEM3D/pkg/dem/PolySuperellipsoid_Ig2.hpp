#include<sudodem/pkg/dem/PolySuperellipsoid.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/pkg/common/Box.hpp>

class Ig2_PolySuperellipsoid_PolySuperellipsoid_PolySuperellipsoidGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_PolySuperellipsoid_PolySuperellipsoid_PolySuperellipsoidGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
			FUNCTOR2D(PolySuperellipsoid,PolySuperellipsoid);
			DEFINE_FUNCTOR_ORDER_2D(PolySuperellipsoid,PolySuperellipsoid);
		
			DECLARE_LOGGER;
		
			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		};REGISTER_SERIALIZABLE_BASE(Ig2_PolySuperellipsoid_PolySuperellipsoid_PolySuperellipsoidGeom, IGeomFunctor);

class Ig2_Wall_PolySuperellipsoid_PolySuperellipsoidGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Wall_PolySuperellipsoid_PolySuperellipsoidGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
			FUNCTOR2D(Wall,PolySuperellipsoid);
			DEFINE_FUNCTOR_ORDER_2D(Wall,PolySuperellipsoid);
		
			DECLARE_LOGGER;
		
			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		};REGISTER_SERIALIZABLE_BASE(Ig2_Wall_PolySuperellipsoid_PolySuperellipsoidGeom, IGeomFunctor);

class Ig2_Facet_PolySuperellipsoid_PolySuperellipsoidGeom: public IGeomFunctor
{
	public:
		virtual ~Ig2_Facet_PolySuperellipsoid_PolySuperellipsoidGeom(){};
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
			FUNCTOR2D(Facet,PolySuperellipsoid);
			DEFINE_FUNCTOR_ORDER_2D(Facet,PolySuperellipsoid);
		
			DECLARE_LOGGER;
		
			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		};REGISTER_SERIALIZABLE_BASE(Ig2_Facet_PolySuperellipsoid_PolySuperellipsoidGeom, IGeomFunctor);