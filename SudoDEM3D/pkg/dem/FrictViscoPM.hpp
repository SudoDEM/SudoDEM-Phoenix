/*************************************************************************
*  Copyright (C) 2014 by Klaus Thoeni                                    *
*  klaus.thoeni@newcastle.edu.au                                         *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

/**
=== OVERVIEW OF FrictViscoPM ===

A particle model for friction and viscous damping in normal direction. The damping coefficient
has to be defined with the material whereas the contact stiffness kn and ks/kn can be defined with the Ip2 functor.

Remarks:
- maybe there is a better way of implementing this without copying from ElasticContactLaw
- maybe we can combine some ideas of this contact law with other contact laws
*/

#pragma once

#include<sudodem/pkg/common/ElastMat.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/common/MatchMaker.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

/** This class holds information associated with each body */
class FrictViscoMat: public FrictMat {
	public:
		Real betan;
		
		virtual ~FrictViscoMat();
		
		// Explicit constructor with initial values
		FrictViscoMat(): betan(0.) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictMat, betan);
		
		DECLARE_LOGGER;
			
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_INDEX_H(FrictViscoMat,FrictMat)
};
REGISTER_SERIALIZABLE_BASE(FrictViscoMat, FrictMat);

/** This class holds information associated with each interaction */
class FrictViscoPhys: public FrictPhys {
	public:
		Real cn_crit;
		Real cn;
		Vector3r normalViscous;
		
		virtual ~FrictViscoPhys();
		
		// Explicit constructor with initial values
		FrictViscoPhys(): cn_crit(NaN), cn(NaN), normalViscous(Vector3r::Zero()) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, cn_crit, cn, normalViscous);
		
		DECLARE_LOGGER;
			
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_INDEX_H(FrictViscoPhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(FrictViscoPhys, FrictPhys);

/** 2d functor creating IPhys (Ip2) taking FrictViscoMat and FrictViscoMat of 2 bodies, returning type FrictViscoPhys */
class Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys: public IPhysFunctor{
	public:
		shared_ptr<MatchMaker> kn;
		shared_ptr<MatchMaker> kRatio;
		shared_ptr<MatchMaker> frictAngle;
		
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);

		FUNCTOR2D(FrictViscoMat,FrictViscoMat);

		// Explicit constructor with initial values
		Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys() {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhysFunctor, kn, kRatio, frictAngle);
		
		DECLARE_LOGGER;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys, IPhysFunctor);

/** 2d functor creating IPhys (Ip2) taking FrictMat and FrictViscoMat of 2 bodies, returning type FrictViscoPhys */
class Ip2_FrictMat_FrictViscoMat_FrictViscoPhys: public IPhysFunctor{
	public:
		shared_ptr<MatchMaker> kn;
		shared_ptr<MatchMaker> kRatio;
		shared_ptr<MatchMaker> frictAngle;
		
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);

		FUNCTOR2D(FrictMat,FrictViscoMat);
		DEFINE_FUNCTOR_ORDER_2D(FrictMat,FrictViscoMat);

		// Explicit constructor with initial values
		Ip2_FrictMat_FrictViscoMat_FrictViscoPhys() {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhysFunctor, kn, kRatio, frictAngle);
		
		DECLARE_LOGGER;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictViscoMat_FrictViscoPhys, IPhysFunctor);

/** 2d functor creating the interaction law (Law2) based on SphereContactGeometry (ScGeom) and FrictViscoPhys of 2 bodies, returning type FrictViscoPM */
class Law2_ScGeom_FrictViscoPhys_CundallStrackVisco: public LawFunctor{
	public:
		bool neverErase;
		bool sphericalBodies;
		bool traceEnergy;
		int plastDissipIx;
		int elastPotentialIx;
		
		OpenMPAccumulator<Real> plasticDissipation;
		
		virtual ~Law2_ScGeom_FrictViscoPhys_CundallStrackVisco();
		
		// Explicit constructor with initial values
		Law2_ScGeom_FrictViscoPhys_CundallStrackVisco(): neverErase(false), sphericalBodies(true), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		Real elasticEnergy ();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, neverErase, sphericalBodies, traceEnergy, plastDissipIx, elastPotentialIx);
		
		FUNCTOR2D(ScGeom,FrictViscoPhys);
		DECLARE_LOGGER;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_FrictViscoPhys_CundallStrackVisco, LawFunctor);