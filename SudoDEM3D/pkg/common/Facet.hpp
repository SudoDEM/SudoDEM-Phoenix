/*************************************************************************
*  Copyright (C) 2008 by Sergei Dorofeenko				 *
*  sega@users.berlios.de                                                 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once
#ifndef FACET_H
#define FACET_H
#include <sudodem/pkg/common/Dispatching.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>
// define this to have topology information about facets enabled;
// it is necessary for FacetTopologyAnalyzer
// #define FACET_TOPO

class Facet : public Shape {
    public:
	Facet() { createIndex(); }
	vector<Vector3r> vertices;

	virtual ~Facet();

	// Postprocessed attributes

	/// Facet's normal
	Vector3r normal;
	/// Normals of edges
	Vector3r ne[3];
	/// Inscribing cirle radius
	Real icr;
	/// Length of the vertice vectors
	Real vl[3];
	/// Unit vertice vectors
	Vector3r vu[3];
	/// Facet's area
	Real area;

	void postLoad(Facet&);

	REGISTER_ATTRIBUTES(Shape, vertices, normal, area, icr);
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Facet);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX_H(Facet,Shape)

};

REGISTER_SERIALIZABLE_BASE(Facet, Shape);

class Bo1_Facet_Aabb : public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body*) override;
	FUNCTOR1D(Facet);
	
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Bo1_Facet_Aabb);
};

REGISTER_SERIALIZABLE_BASE(Bo1_Facet_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
class Gl1_Facet : public GlShapeFunctor
{
	public:
		static bool normals;
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;
		virtual string renders() const override { return "Facet";}; FUNCTOR1D(Facet);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Gl1_Facet);
};

REGISTER_SERIALIZABLE_BASE(Gl1_Facet, GlShapeFunctor);
#endif
#endif
