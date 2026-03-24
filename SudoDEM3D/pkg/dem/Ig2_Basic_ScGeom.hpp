/*************************************************************************
*  Copyright (C) 2018 by Shiwei Zhao                                     *
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*  Copyright (C) 2006 by Bruno Chareyre                                  *
*  bruno.chareyre@hmg.inpg.fr                                            *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/pkg/common/Box.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
//geometry computation between basic shapes:sphere-sphere, sphere-facet,sphere-wall,sphere-box

class Ig2_Sphere_Sphere_ScGeom: public IGeomFunctor{
	public:
		Real interactionDetectionFactor;
		bool avoidGranularRatcheting;
		
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

		Ig2_Sphere_Sphere_ScGeom() : interactionDetectionFactor(1), avoidGranularRatcheting(true) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeomFunctor, interactionDetectionFactor, avoidGranularRatcheting);
		
	FUNCTOR2D(Sphere,Sphere);
	// needed for the dispatcher, even if it is symmetric
	DEFINE_FUNCTOR_ORDER_2D(Sphere,Sphere);
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Sphere_Sphere_ScGeom, IGeomFunctor);

class Ig2_Sphere_Sphere_ScGeom6D: public Ig2_Sphere_Sphere_ScGeom{
	public:
		bool updateRotations;
		bool creep;
		
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

		Ig2_Sphere_Sphere_ScGeom6D() : updateRotations(true), creep(false) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Ig2_Sphere_Sphere_ScGeom, updateRotations, creep);
		
		FUNCTOR2D(Sphere,Sphere);
		// needed for the dispatcher, even if it is symmetric
		DEFINE_FUNCTOR_ORDER_2D(Sphere,Sphere);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	};
	REGISTER_SERIALIZABLE_BASE(Ig2_Sphere_Sphere_ScGeom6D, IGeomFunctor);

class Ig2_Facet_Sphere_ScGeom : public IGeomFunctor
{
	public :
		Real shrinkFactor;
		
		virtual bool go(const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
					
		Ig2_Facet_Sphere_ScGeom() : shrinkFactor(0) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeomFunctor, shrinkFactor);
		
	DECLARE_LOGGER;
	FUNCTOR2D(Facet,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Facet,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Sphere_ScGeom, IGeomFunctor);

class Ig2_Facet_Sphere_ScGeom6D : public Ig2_Facet_Sphere_ScGeom
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
	FUNCTOR2D(Facet,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Facet,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Sphere_ScGeom6D, IGeomFunctor);

class Ig2_Wall_Sphere_ScGeom: public IGeomFunctor{
	public:
		bool noRatch;
		
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		
		Ig2_Wall_Sphere_ScGeom() : noRatch(true) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeomFunctor, noRatch);
		
	FUNCTOR2D(Wall,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Wall,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Sphere_ScGeom, IGeomFunctor);


class Ig2_Box_Sphere_ScGeom : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

		virtual bool goReverse(	const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

	FUNCTOR2D(Box,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Box,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Box_Sphere_ScGeom, IGeomFunctor);

class Ig2_Box_Sphere_ScGeom6D : public Ig2_Box_Sphere_ScGeom
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

		virtual bool goReverse(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);

	FUNCTOR2D(Box,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Box,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Box_Sphere_ScGeom6D, IGeomFunctor);
