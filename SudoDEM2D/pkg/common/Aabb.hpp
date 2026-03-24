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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Aabb, Bound, std::shared_ptr<Aabb>> _classObj(_module, "Aabb", "Axis-aligned bounding box, for use with :yref:`InsertionSortCollider`. (This class is quasi-redundant since min,max are already contained in :yref:`Bound` itself. That might change at some point, though.)");
			_classObj.def(pybind11::init<>());
		}
	REGISTER_CLASS_INDEX(Aabb,Bound);
};
REGISTER_SERIALIZABLE_BASE(Aabb, Bound);

#ifdef SUDODEM_OPENGL
class Gl1_Aabb: public GlBoundFunctor{
	public:
		virtual void go(const shared_ptr<Bound>&, Scene*) override;

		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Gl1_Aabb, Functor, std::shared_ptr<Gl1_Aabb>> _classObj(_module, "Gl1_Aabb", "Render Axis-aligned bounding box (:yref:`Aabb`).");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Gl1_Aabb, GlShapeFunctor);
#endif

#endif
