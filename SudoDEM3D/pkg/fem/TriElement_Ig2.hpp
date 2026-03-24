/*************************************************************************
*  Copyright (C) 2020 by Sway Zhao                                       *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#ifndef TRIELEMENT_IG2_H
#define TRIELEMENT_IG2_H

#include <sudodem/pkg/fem/TriElement.hpp>
#include <sudodem/pkg/common/Sphere.hpp>
#include <sudodem/pkg/dem/PolySuperellipsoid.hpp>
#include <sudodem/pkg/dem/GJKParticle.hpp>

#include <sudodem/pkg/dem/FrictPhys.hpp>
#include <sudodem/pkg/dem/RollingResistanceLaw.hpp>

class Ig2_TriElement_Sphere_ScGeom : public IGeomFunctor
{
	public :
		// Member variable
		Real shrinkFactor;

		virtual ~Ig2_TriElement_Sphere_ScGeom() {}

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

		//! Default constructor
		Ig2_TriElement_Sphere_ScGeom() : shrinkFactor(0) {}

		//! Python constructor
		static std::shared_ptr<Ig2_TriElement_Sphere_ScGeom> create() { return std::make_shared<Ig2_TriElement_Sphere_ScGeom>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	FUNCTOR2D(TriElement,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(TriElement,Sphere);
	REGISTER_ATTRIBUTES(IGeomFunctor, shrinkFactor);
};
REGISTER_SERIALIZABLE(Ig2_TriElement_Sphere_ScGeom);

class Ig2_TriElement_PolySuperellipsoid_ScGeom : public IGeomFunctor
{
	public :
		// Member variable
		Real shrinkFactor;

		virtual ~Ig2_TriElement_PolySuperellipsoid_ScGeom() {}

		virtual bool go(const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);

		//! Default constructor
		Ig2_TriElement_PolySuperellipsoid_ScGeom() : shrinkFactor(0) {}

		//! Python constructor
		static std::shared_ptr<Ig2_TriElement_PolySuperellipsoid_ScGeom> create() { return std::make_shared<Ig2_TriElement_PolySuperellipsoid_ScGeom>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	FUNCTOR2D(TriElement,PolySuperellipsoid);
	DEFINE_FUNCTOR_ORDER_2D(TriElement,PolySuperellipsoid);
	REGISTER_ATTRIBUTES(IGeomFunctor, shrinkFactor);
};
REGISTER_SERIALIZABLE(Ig2_TriElement_PolySuperellipsoid_ScGeom);

class Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys: public IPhysFunctor{
	public:
		// Member variable
		shared_ptr<MatchMaker> frictAngle;

		virtual ~Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys() {}

		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);

		//! Default constructor
		Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys() {}

		//! Python constructor
		static std::shared_ptr<Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys> create() { return std::make_shared<Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	FUNCTOR2D(RolFrictMat,PolySuperellipsoidMat);
	REGISTER_ATTRIBUTES(IPhysFunctor, frictAngle);
};
REGISTER_SERIALIZABLE(Ip2_RolFrictMat_PolySuperellipsoidMat_FrictPhys);

//GJKParticle and TriElement
class Ig2_TriElement_GJKParticle_ScGeom : public IGeomFunctor
{
	public :
		// Member variable
		Real shrinkFactor;

		virtual ~Ig2_TriElement_GJKParticle_ScGeom() {}

		virtual bool go(const shared_ptr<Shape>& cm1,
					const shared_ptr<Shape>& cm2,
					const State& state1,
					const State& state2,
					const Vector3r& shift2,
					const bool& force,
					const shared_ptr<Interaction>& c);

		//! Default constructor
		Ig2_TriElement_GJKParticle_ScGeom() : shrinkFactor(0) {}

		//! Python constructor
		static std::shared_ptr<Ig2_TriElement_GJKParticle_ScGeom> create() { return std::make_shared<Ig2_TriElement_GJKParticle_ScGeom>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	FUNCTOR2D(TriElement,GJKParticle);
	DEFINE_FUNCTOR_ORDER_2D(TriElement,GJKParticle);
	REGISTER_ATTRIBUTES(IGeomFunctor, shrinkFactor);
};
REGISTER_SERIALIZABLE(Ig2_TriElement_GJKParticle_ScGeom);

class Ip2_RolFrictMat_GJKParticleMat_FrictPhys: public IPhysFunctor{
	public:
		// Member variable
		shared_ptr<MatchMaker> frictAngle;

		virtual ~Ip2_RolFrictMat_GJKParticleMat_FrictPhys() {}

		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);

		//! Default constructor
		Ip2_RolFrictMat_GJKParticleMat_FrictPhys() {}

		//! Python constructor
		static std::shared_ptr<Ip2_RolFrictMat_GJKParticleMat_FrictPhys> create() { return std::make_shared<Ip2_RolFrictMat_GJKParticleMat_FrictPhys>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	FUNCTOR2D(RolFrictMat,GJKParticleMat);
	REGISTER_ATTRIBUTES(IPhysFunctor, frictAngle);
};
REGISTER_SERIALIZABLE(Ip2_RolFrictMat_GJKParticleMat_FrictPhys);
#endif //TRIELEMENT_IG2_H
