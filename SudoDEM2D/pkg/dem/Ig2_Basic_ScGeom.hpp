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
#include<sudodem/pkg/common/Disk.hpp>
//#include<sudodem/pkg/common/Box.hpp>
//#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
//geometry computation between basic shapes:disk-disk, disk-facet,disk-wall,disk-box

class Ig2_Disk_Disk_ScGeom: public IGeomFunctor{
	public:
		Real interactionDetectionFactor;
		bool avoidGranularRatcheting;

		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c) override;
		virtual bool goReverse(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c) override;

		Ig2_Disk_Disk_ScGeom() : interactionDetectionFactor(1), avoidGranularRatcheting(true) {}
		FUNCTOR2D(Disk,Disk);


		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Disk_Disk_ScGeom, IGeomFunctor, std::shared_ptr<Ig2_Disk_Disk_ScGeom>> _classObj(_module, "Ig2_Disk_Disk_ScGeom", "Create/update a :yref:`ScGeom` instance representing the geometry of a contact point between two :yref:`Disks<Disk>` s.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("interactionDetectionFactor", &Ig2_Disk_Disk_ScGeom::interactionDetectionFactor, "Enlarge both radii by this factor (if >1), to permit creation of distant interactions.\n\nInteractionGeometry will be computed when interactionDetectionFactor*(rad1+rad2) > distance.\n\n.. note::\n\t This parameter is functionally coupled with :yref:`Bo1_Disk_Aabb::aabbEnlargeFactor`, which will create larger bounding boxes and should be of the same value.");
			_classObj.def_readwrite("avoidGranularRatcheting", &Ig2_Disk_Disk_ScGeom::avoidGranularRatcheting, "Define relative velocity so that ratcheting is avoided. It applies for disk-disk contacts. It eventualy also apply for disk-emulating interactions (i.e. convertible into the ScGeom type), if the virtual disk's motion is defined correctly (see e.g. :yref:`Ig2_Disk_ChainedCylinder_CylScGeom`.\n\nShort explanation of what we want to avoid :\n\nNumerical ratcheting is best understood considering a small elastic cycle at a contact between two grains : assuming b1 is fixed, impose this displacement to b2 :\n\n#. translation *dx* in the normal direction\n#. rotation *a*\n#. translation *-dx* (back to the initial position)\n#. rotation *-a* (back to the initial orientation)\n\n\nIf the branch vector used to define the relative shear in rotation×branch is not constant (typically if it is defined from the vector center→contactPoint), then the shear displacement at the end of this cycle is not zero: rotations *a* and *-a* are multiplied by branches of different lengths.\n\nIt results in a finite contact force at the end of the cycle even though the positions and orientations are unchanged, in total contradiction with the elastic nature of the problem. It could also be seen as an *inconsistent energy creation or loss*. Given that DEM simulations tend to generate oscillations around equilibrium (damped mass-spring), it can have a significant impact on the evolution of the packings, resulting for instance in slow creep in iterations under constant load.\n\nThe solution adopted here to avoid ratcheting is as proposed by McNamara and co-workers. They analyzed the ratcheting problem in detail - even though they comment on the basis of a cycle that differs from the one shown above. One will find interesting discussions in e.g. [McNamara2008]_, even though solution it suggests is not fully applied here (equations of motion are not incorporating alpha, in contradiction with what is suggested by McNamara et al.).\n\n");
		}
	// needed for the dispatcher, even if it is symmetric
	DEFINE_FUNCTOR_ORDER_2D(Disk,Disk);
};
REGISTER_SERIALIZABLE_BASE(Ig2_Disk_Disk_ScGeom, IGeomFunctor);

/*
class Ig2_Facet_Disk_ScGeom : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector2r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector2r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);
	DECLARE_LOGGER;
	DEFINE_FUNCTOR_ORDER_2D(Facet,Disk);
};

REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Disk_ScGeom, IGeomFunctor);

*/

class Ig2_Wall_Disk_ScGeom: public IGeomFunctor{
	public:
		bool noRatch;

		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c) override;
		
		Ig2_Wall_Disk_ScGeom() : noRatch(true) {}

		FUNCTOR2D(Wall,Disk);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Wall_Disk_ScGeom, IGeomFunctor, std::shared_ptr<Ig2_Wall_Disk_ScGeom>> _classObj(_module, "Ig2_Wall_Disk_ScGeom", "Create/update a :yref:`ScGeom` instance representing intersection of :yref:`Wall` and :yref:`Disk`.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("noRatch", &Ig2_Wall_Disk_ScGeom::noRatch, "Avoid granular ratcheting");
		}
	DEFINE_FUNCTOR_ORDER_2D(Wall,Disk);
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Disk_ScGeom, IGeomFunctor);

class Ig2_Fwall_Disk_ScGeom : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1,const shared_ptr<Shape>& cm2,const State& state1,const State& state2,const Vector2r& shift2,const bool& force,
					const shared_ptr<Interaction>& c) override;
		virtual bool goReverse(	const shared_ptr<Shape>& cm1,const shared_ptr<Shape>& cm2,const State& state1,const State& state2,const Vector2r& shift2,const bool& force,
					const shared_ptr<Interaction>& c) override;
		
		Ig2_Fwall_Disk_ScGeom() {}
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ig2_Fwall_Disk_ScGeom, IGeomFunctor, std::shared_ptr<Ig2_Fwall_Disk_ScGeom>> _classObj(_module, "Ig2_Fwall_Disk_ScGeom", "Create/update a :yref:`ScGeom` instance representing intersection of :yref:`Fwall` and :yref:`Disk`.");
			_classObj.def(pybind11::init<>());
		}
	DECLARE_LOGGER;
	DEFINE_FUNCTOR_ORDER_2D(Fwall,Disk);
};

REGISTER_SERIALIZABLE_BASE(Ig2_Fwall_Disk_ScGeom, IGeomFunctor);


/*
class Ig2_Box_Disk_ScGeom : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c);

		virtual bool goReverse(	const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2, const State& state1, const State& state2, const Vector2r& shift2, const bool& force, const shared_ptr<Interaction>& c);

	DEFINE_FUNCTOR_ORDER_2D(Box,Disk);
};
REGISTER_SERIALIZABLE_BASE(Ig2_Box_Disk_ScGeom, IGeomFunctor);
*/