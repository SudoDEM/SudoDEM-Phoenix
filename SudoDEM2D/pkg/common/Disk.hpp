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

class Disk: public Shape{
	public:
		Real radius = NaN;
		Real ref_radius = NaN;
		
		Disk() : radius(NaN), ref_radius(NaN) {createIndex();}
		Disk(Real _radius): radius(_radius),ref_radius(_radius){createIndex(); }
		virtual ~Disk () {};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, radius, ref_radius);
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("Disk");
			pybind11::class_<Disk, Shape, std::shared_ptr<Disk>> _classObj(_module, "Disk", "Geometry of spherical particle.");
			_classObj.def(pybind11::init<Real>());
			_classObj.def_readwrite("radius", &Disk::radius, "Radius [m]");
					_classObj.def_readwrite("ref_radius", &Disk::ref_radius, "reference radius [m]");
					}
				REGISTER_CLASS_NAME_DERIVED(Disk);
				REGISTER_CLASS_INDEX(Disk,Shape);
			};
}
// necessary
using namespace sudodem;

// must be outside sudodem namespace
REGISTER_SERIALIZABLE_BASE(Disk, Shape);

class Bo1_Disk_Aabb : public BoundFunctor
{
	public :
		Real aabbEnlargeFactor = -1;

		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se2r&, const Body*) override;
	FUNCTOR1D(Disk);
	
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("Bo1_Disk_Aabb");
			pybind11::class_<Bo1_Disk_Aabb, BoundFunctor, std::shared_ptr<Bo1_Disk_Aabb>> _classObj(_module, "Bo1_Disk_Aabb", "Functor creating :yref:`Aabb` from :yref:`Disk`.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("aabbEnlargeFactor", &Bo1_Disk_Aabb::aabbEnlargeFactor, "Relative enlargement of the bounding box; deactivated if negative.\n\n.. note::\n\tThis attribute is used to create distant interaction, but is only meaningful with an :yref:`IGeomFunctor` which will not simply discard such interactions: :yref:`Ig2_Disk_Disk_ScGeom::interactionDetectionFactor` should have the same value as :yref:`aabbEnlargeFactor<Bo1_Disk_Aabb::aabbEnlargeFactor>`.");
		}
};

REGISTER_SERIALIZABLE_BASE(Bo1_Disk_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
class Gl1_Disk : public GlShapeFunctor{
	private:
		// for stripes
		static int glStripedDiskList;
		static int glGlutDiskList;
		//Generate GlList for GLUT disk
		void initGlutGlList();
		//Generate GlList for sliced disks
		void initStripedGlList();
		//for regenerating glutDisk list if needed
		static int preSlices;
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;
		virtual string renders() const override { return "Disk";}; FUNCTOR1D(Disk);
		
		static bool wire;
		static int Slices;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("Gl1_Disk");
			pybind11::class_<Gl1_Disk, Functor, std::shared_ptr<Gl1_Disk>> _classObj(_module, "Gl1_Disk", "Renders :yref:`Disk` object");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite_static("wire", &Gl1_Disk::wire, "Only show wireframe (controlled by ``glutSlices`` and ``glutStacks``.");
			_classObj.def_readonly_static("Slices", &Gl1_Disk::Slices, "Base number of disk slices, multiplied by :yref:`Gl1_Disk::quality` before use); not used with ``stripes`` (see `glut{Solid,Wire}Disk reference <http://www.opengl.org/documentation/specs/glut/spec3/node81.html>`_)");
		}
};

REGISTER_SERIALIZABLE_BASE(Gl1_Disk, GlShapeFunctor);
#endif
#endif