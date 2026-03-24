/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once
#ifndef BOX_H
#define BOX_H
#include <sudodem/pkg/common/Dispatching.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>

class Box: public Shape{
	public:
		Vector3r extents;
		
		Box() : extents(Vector3r::Zero()) {createIndex();}
		Box(const Vector3r& _extents): extents(_extents){createIndex();}
		virtual ~Box () {};
		
		REGISTER_ATTRIBUTES(Shape, extents);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Box);
		
		REGISTER_CLASS_INDEX_H(Box,Shape)
};

REGISTER_SERIALIZABLE_BASE(Box, Shape);

class Bo1_Box_Aabb : public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body*) override;
	FUNCTOR1D(Box);
	
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Bo1_Box_Aabb);
};

REGISTER_SERIALIZABLE_BASE(Bo1_Box_Aabb, BoundFunctor);

#ifdef SUDODEM_OPENGL
class Gl1_Box : public GlShapeFunctor{
	public :
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;
		virtual string renders() const override { return "Box";}; FUNCTOR1D(Box);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Gl1_Box);
};

REGISTER_SERIALIZABLE_BASE(Gl1_Box, GlShapeFunctor);
#endif
#endif
