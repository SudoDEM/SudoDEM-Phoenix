/*************************************************************************
*  Copyright (C) 2005 by Bruno Chareyre   bruno.chareyre@hmg.inpg.fr     *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

class Law2_ScGeom_FrictPhys_CundallStrack: public LawFunctor{
	public:
		OpenMPAccumulator<Real> plasticDissipation;
		bool neverErase;
		bool sphericalBodies;
		bool traceEnergy;
		int plastDissipIx;
		int elastPotentialIx;
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		Real elasticEnergy ();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		
		Law2_ScGeom_FrictPhys_CundallStrack() : neverErase(false), sphericalBodies(true), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, neverErase, sphericalBodies, traceEnergy, plastDissipIx, elastPotentialIx);
		
		FUNCTOR2D(ScGeom,FrictPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_FrictPhys_CundallStrack, LawFunctor);

class Law2_ScGeom_ViscoFrictPhys_CundallStrack: public Law2_ScGeom_FrictPhys_CundallStrack{
	public:
		bool shearCreep;
		Real viscosity;
		Real creepStiffness;
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		
		Law2_ScGeom_ViscoFrictPhys_CundallStrack() : shearCreep(false), viscosity(1), creepStiffness(1) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Law2_ScGeom_FrictPhys_CundallStrack, shearCreep, viscosity, creepStiffness);
		
		FUNCTOR2D(ScGeom,ViscoFrictPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_ViscoFrictPhys_CundallStrack, LawFunctor);

class ElasticContactLaw : public GlobalEngine{
		shared_ptr<Law2_ScGeom_FrictPhys_CundallStrack> functor;
		bool neverErase;
	public :
		void action();
		
		ElasticContactLaw() : neverErase(false) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlobalEngine, neverErase);
		
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ElasticContactLaw, GlobalEngine);



