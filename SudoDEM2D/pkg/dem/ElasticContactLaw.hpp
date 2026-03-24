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
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I) override;
		Real elasticEnergy ();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		
		Law2_ScGeom_FrictPhys_CundallStrack() : neverErase(false), sphericalBodies(true), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
		FUNCTOR2D(ScGeom,FrictPhys);
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Law2_ScGeom_FrictPhys_CundallStrack, LawFunctor, std::shared_ptr<Law2_ScGeom_FrictPhys_CundallStrack>> _classObj(_module, "Law2_ScGeom_FrictPhys_CundallStrack", "Law for linear compression, and Mohr-Coulomb plasticity surface without cohesion.\nThis law implements the classical linear elastic-plastic law from [CundallStrack1979]_ (see also [Pfc3dManual30]_). The normal force is (with the convention of positive tensile forces) $F_n=\\min(k_n u_n, 0)$. The shear force is $F_s=k_s u_s$, the plasticity condition defines the maximum value of the shear force : $F_s^{\\max}=F_n\\tan(\\phi)$, with $\\phi$ the friction angle.\n\nThis law is well tested in the context of triaxial simulation, and has been used for a number of published results (see e.g. [Scholtes2009b]_ and other papers from the same authors). It is generalised by :yref:`Law2_ScGeom6D_CohFrictPhys_CohesionMoment`, which adds cohesion and moments at contact.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("neverErase", &Law2_ScGeom_FrictPhys_CundallStrack::neverErase, "Keep interactions even if particles go away from each other (only in case another constitutive law is in the scene, e.g. :yref:`Law2_ScGeom_CapillaryPhys_Capillarity`)");
			_classObj.def_readwrite("sphericalBodies", &Law2_ScGeom_FrictPhys_CundallStrack::sphericalBodies, "If true, compute branch vectors from radii (faster), else use contactPoint-position. Turning this flag true is safe for disk-disk contacts and a few other specific cases. It will give wrong values of torques on facets or boxes.");
			_classObj.def_readwrite("traceEnergy", &Law2_ScGeom_FrictPhys_CundallStrack::traceEnergy, "Define the total energy dissipated in plastic slips at all contacts. This will trace only plastic energy in this law, see O.trackEnergy for a more complete energies tracing");
			_classObj.def("elasticEnergy", &Law2_ScGeom_FrictPhys_CundallStrack::elasticEnergy, "Compute and return the total elastic energy in all \"FrictPhys\" contacts");
			_classObj.def("plasticDissipation", &Law2_ScGeom_FrictPhys_CundallStrack::getPlasticDissipation, "Total energy dissipated in plastic slips at all FrictPhys contacts. Computed only if :yref:`Law2_ScGeom_FrictPhys_CundallStrack::traceEnergy` is true.");
			_classObj.def("initPlasticDissipation", &Law2_ScGeom_FrictPhys_CundallStrack::initPlasticDissipation, "Initialize cummulated plastic dissipation to a value (0 by default).");
		}
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_FrictPhys_CundallStrack, LawFunctor);

class Law2_ScGeom_ViscoFrictPhys_CundallStrack: public Law2_ScGeom_FrictPhys_CundallStrack{
	public:
		bool shearCreep;
		Real viscosity;
		Real creepStiffness;
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I) override;
		
		Law2_ScGeom_ViscoFrictPhys_CundallStrack() : shearCreep(false), viscosity(1), creepStiffness(1) {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Law2_ScGeom_ViscoFrictPhys_CundallStrack, Law2_ScGeom_FrictPhys_CundallStrack, std::shared_ptr<Law2_ScGeom_ViscoFrictPhys_CundallStrack>> _classObj(_module, "Law2_ScGeom_ViscoFrictPhys_CundallStrack", "Law similar to :yref:`Law2_ScGeom_FrictPhys_CundallStrack` with the addition of shear creep at contacts.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("shearCreep", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::shearCreep, " ");
			_classObj.def_readwrite("viscosity", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::viscosity, " ");
			_classObj.def_readwrite("creepStiffness", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::creepStiffness, " ");
		}
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_ViscoFrictPhys_CundallStrack, LawFunctor);

class ElasticContactLaw : public GlobalEngine{
		shared_ptr<Law2_ScGeom_FrictPhys_CundallStrack> functor;
		bool neverErase;
	public :
		void action() override;
		
		ElasticContactLaw() : neverErase(false) {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<ElasticContactLaw, GlobalEngine, std::shared_ptr<ElasticContactLaw>> _classObj(_module, "ElasticContactLaw", "[DEPRECATED] Loop over interactions applying :yref:`Law2_ScGeom_FrictPhys_CundallStrack` on all interactions.\n\n.. note::\n  Use :yref:`InteractionLoop` and :yref:`Law2_ScGeom_FrictPhys_CundallStrack` instead of this class for performance reasons.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("neverErase", &ElasticContactLaw::neverErase, "Keep interactions even if particles go away from each other (only in case another constitutive law is in the scene, e.g. :yref:`Law2_ScGeom_CapillaryPhys_Capillarity`)");
		}
};
REGISTER_SERIALIZABLE_BASE(ElasticContactLaw, GlobalEngine);