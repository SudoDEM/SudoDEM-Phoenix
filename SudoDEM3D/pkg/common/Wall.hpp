// © 2009 Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/Shape.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>


/*! Object representing infinite plane aligned with the coordinate system (axis-aligned wall). */
class Wall: public Shape{
	public:
		virtual ~Wall(); // vtable
		int sense;
		int axis;
		
		Wall() : sense(0), axis(0) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, sense, axis);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Wall);
	REGISTER_CLASS_INDEX_H(Wall,Shape)
};
REGISTER_SERIALIZABLE_BASE(Wall, Shape);

/*! Functor for computing axis-aligned bounding box
    from axis-aligned wall. Has no parameters. */
class Bo1_Wall_Aabb: public BoundFunctor{
	public:
		virtual void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body*) override;
		FUNCTOR1D(Wall);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Bo1_Wall_Aabb, BoundFunctor);

class Fwall : public Shape {
		public:

	virtual ~Fwall();
	
	Fwall() : vl(0), vu(Vector3r::Zero()), vertex1(Vector3r::Zero()), vertex2(Vector3r::Zero()), normal(Vector3r::Zero()) { createIndex(); }

	// Postprocessed attributes

	/// Fwall's normal
	//Vector3r nf;
	/// Normals of edge
	//Vector3r ne;
	/// Inscribing cirle radius
	//Real icr;
	/// Length of the vertice vector
	Real vl;
	/// vertice vector
	Vector3r vu;//vertice 1 to vertice 2
	Vector3r vertex1;
	Vector3r vertex2;
	Vector3r normal;

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Shape, vl, vu, vertex1, vertex2, normal);

	void postLoad(Fwall&);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX_H(Fwall,Shape)
};
REGISTER_SERIALIZABLE_BASE(Fwall, Shape);

class Bo1_Fwall_Aabb : public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body*) override;
		FUNCTOR1D(Fwall);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Bo1_Fwall_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	class Gl1_Wall: public GlShapeFunctor{
		public:
			int div = 20;

			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;

					SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;		RENDERS(Wall);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_Wall, GlShapeFunctor);
	class Gl1_Fwall : public GlShapeFunctor
	{
		public:
			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;

					SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;			RENDERS(Fwall);
	};

	REGISTER_SERIALIZABLE_BASE(Gl1_Fwall, GlShapeFunctor);

#endif
