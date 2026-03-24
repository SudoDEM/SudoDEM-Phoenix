#pragma once
#ifndef SPHERE_H
#define SPHERE_H
#include<sudodem/core/Shape.hpp>
#include <sudodem/pkg/common/Dispatching.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>
// HACK to work around https://bugs.launchpad.net/sudodem/+bug/528509
// see comments there for explanation
namespace sudodem{

class Sphere: public Shape{
	public:
		Real radius = NaN;
		Real ref_radius = NaN;
		
		Sphere() : radius(NaN), ref_radius(NaN) {createIndex();}
		Sphere(Real _radius): radius(_radius),ref_radius(_radius){createIndex(); }
		virtual ~Sphere () {};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, radius, ref_radius);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
			REGISTER_CLASS_NAME_DERIVED(Sphere);
			REGISTER_CLASS_INDEX_H(Sphere,Shape)
		};
}
// necessary
using namespace sudodem;

// must be outside sudodem namespace
REGISTER_SERIALIZABLE_BASE(Sphere, Shape);

class Bo1_Sphere_Aabb : public BoundFunctor
{
	public :
		Real aabbEnlargeFactor = -1;

		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r&, const Body*) override;
	FUNCTOR1D(Sphere);
	
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Bo1_Sphere_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
class Gl1_Sphere : public GlShapeFunctor{
	private:
		// for stripes
		static int glStripedSphereList;
		static int glGlutSphereList;
	//Generate GlList for GLUT sphere
		void initGlutGlList();
		//Generate GlList for sliced spheres
		void initStripedGlList();
		//for regenerating glutSphere list if needed
		static int preSlices;
		// subdivide triangle for striped sphere
		static void subdivideTriangle(const Vector3r& v1, const Vector3r& v2, const Vector3r& v3, int depth);
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;
		virtual void initgl() override;
		virtual string renders() const override { return "Sphere";}; FUNCTOR1D(Sphere);
		
		static bool wire;
		static bool stripes;
		static int Slices;
		static int glutSlices;
		static int glutStacks;
		static Real quality;
		static bool localSpecView;
		static vector<Vector3r> vertices;
		static vector<Vector3r> faces;
		static Real prevQuality;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Gl1_Sphere, GlShapeFunctor);
#endif
#endif