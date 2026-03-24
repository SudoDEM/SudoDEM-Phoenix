/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once
#ifndef AABB_H
#define AABB_H
#include<sudodem/core/Bound.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>
/*! Representation of bound by min and max points.

This class is redundant, since it has no data members; don't delete it, though,
as Bound::{min,max} might move here one day.

*/
class Aabb : public Bound{
	public :
		virtual ~Aabb() {};
		
		Aabb() { createIndex(); }
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(Aabb,Bound)
};
REGISTER_SERIALIZABLE_BASE(Aabb, Bound);

#ifdef SUDODEM_OPENGL
class Gl1_Aabb: public GlBoundFunctor{
	public:
		virtual void go(const shared_ptr<Bound>&, Scene*) override;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Gl1_Aabb, GlShapeFunctor);
#endif

#endif
