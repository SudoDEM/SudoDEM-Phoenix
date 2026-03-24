/*************************************************************************
*  Copyright (C) 2005 by Bruno Chareyre   bruno.chareyre@hmg.inpg.fr     *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include<sudodem/pkg/dem/ElasticContactLaw.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/dem/DemXDofGeom.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Scene.hpp>


#if 1
Real Law2_ScGeom_FrictPhys_CundallStrack::getPlasticDissipation() {return (Real) plasticDissipation;}
void Law2_ScGeom_FrictPhys_CundallStrack::initPlasticDissipation(Real initVal) {plasticDissipation.reset(); plasticDissipation+=initVal;}
Real Law2_ScGeom_FrictPhys_CundallStrack::elasticEnergy()
{
	Real energy=0;
	for(const shared_ptr<Interaction>& I :  *scene->interactions){
		if(!I->isReal()) continue;
		FrictPhys* phys = dynamic_cast<FrictPhys*>(I->phys.get());
		if(phys) {
			energy += 0.5*(phys->normalForce.squaredNorm()/phys->kn + phys->shearForce.squaredNorm()/phys->ks);}
	}
	return energy;
}
#endif

void ElasticContactLaw::action()
{
	if(!functor) functor=shared_ptr<Law2_ScGeom_FrictPhys_CundallStrack>(new Law2_ScGeom_FrictPhys_CundallStrack);
	functor->neverErase=neverErase;
	functor->scene=scene;
	for(const shared_ptr<Interaction>& I :  *scene->interactions){
		if(!I->isReal()) continue;
		#ifdef SUDODEM_DEBUG
			// these checks would be redundant in the functor (LawDispatcher does that already)
			if(!dynamic_cast<ScGeom*>(I->geom.get()) || !dynamic_cast<FrictPhys*>(I->phys.get())) continue;
		#endif
			functor->go(I->geom, I->phys, I.get());
	}
}

CREATE_LOGGER(Law2_ScGeom_FrictPhys_CundallStrack);
bool Law2_ScGeom_FrictPhys_CundallStrack::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* contact){
	int id1 = contact->getId1(), id2 = contact->getId2();

	ScGeom*    geom= static_cast<ScGeom*>(ig.get());
	FrictPhys* phys = static_cast<FrictPhys*>(ip.get());
	if(geom->penetrationDepth <0){
		if (neverErase) {
			phys->shearForce = Vector3r::Zero();
			phys->normalForce = Vector3r::Zero();}
		else return false;
	}
	Real& un=geom->penetrationDepth;
	phys->normalForce=phys->kn*std::max(un,(Real) 0)*geom->normal;

	Vector3r& shearForce = geom->rotate(phys->shearForce);
	const Vector3r& shearDisp = geom->shearIncrement();
	shearForce -= phys->ks*shearDisp;
	Real maxFs = phys->normalForce.squaredNorm()*std::pow(phys->tangensOfFrictionAngle,2);

	if (!scene->trackEnergy  && !traceEnergy){//Update force but don't compute energy terms (see below))
		// PFC3d SlipModel, is using friction angle. CoulombCriterion
		if( shearForce.squaredNorm() > maxFs ){
			Real ratio = sqrt(maxFs) / shearForce.norm();
			shearForce *= ratio;}
	} else {
		//almost the same with additional Vector3r instatinated for energy tracing,
		//duplicated block to make sure there is no cost for the instanciation of the vector when traceEnergy==false
		if(shearForce.squaredNorm() > maxFs){
			Real ratio = sqrt(maxFs) / shearForce.norm();
			Vector3r trialForce=shearForce;//store prev force for definition of plastic slip
			//define the plastic work input and increment the total plastic energy dissipated
			shearForce *= ratio;
			Real dissip=((1/phys->ks)*(trialForce-shearForce))/*plastic disp*/ .dot(shearForce)/*active force*/;
			if (traceEnergy) plasticDissipation += dissip;
			else if(dissip>0) scene->energy->add(dissip,"plastDissip",plastDissipIx,/*reset*/false);
		}
		// compute elastic energy as well
		scene->energy->add(0.5*(phys->normalForce.squaredNorm()/phys->kn+phys->shearForce.squaredNorm()/phys->ks),"elastPotential",elastPotentialIx,/*reset at every timestep*/true);
	}
	if (!scene->isPeriodic && !sphericalBodies) {
		State* de1 = Body::byId(id1,scene)->state.get();
		State* de2 = Body::byId(id2,scene)->state.get();
		applyForceAtContactPoint(-phys->normalForce-shearForce, geom->contactPoint, id1, de1->se3.position, id2, de2->se3.position);}
	else {//we need to use correct branches in the periodic case, the following apply for spheres only
		Vector3r force = -phys->normalForce-shearForce;
		scene->forces.addForce(id1,force);
		scene->forces.addForce(id2,-force);
		scene->forces.addTorque(id1,(geom->radius1-0.5*geom->penetrationDepth)* geom->normal.cross(force));
		scene->forces.addTorque(id2,(geom->radius2-0.5*geom->penetrationDepth)* geom->normal.cross(force));
	}
	return true;
}

bool Law2_ScGeom_ViscoFrictPhys_CundallStrack::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* contact){
	ScGeom*    geom= static_cast<ScGeom*>(ig.get());
	ViscoFrictPhys* phys = static_cast<ViscoFrictPhys*>(ip.get());
	if (shearCreep) {
			const Real& dt=scene->dt;
			geom->rotate(phys->creepedShear);
			phys->creepedShear+= creepStiffness*phys->ks*(phys->shearForce-phys->creepedShear)*dt/viscosity;
			phys->shearForce -= phys->ks*((phys->shearForce-phys->creepedShear)*dt/viscosity);}
	return Law2_ScGeom_FrictPhys_CundallStrack::go(ig,ip,contact);
}

// pyRegisterClass implementations moved from ElasticContactLaw.hpp
void Law2_ScGeom_FrictPhys_CundallStrack::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Law2_ScGeom_FrictPhys_CundallStrack");
	pybind11::class_<Law2_ScGeom_FrictPhys_CundallStrack, LawFunctor, std::shared_ptr<Law2_ScGeom_FrictPhys_CundallStrack>> _classObj(_module, "Law2_ScGeom_FrictPhys_CundallStrack", "Law for linear compression, and Mohr-Coulomb plasticity surface without cohesion.\nThis law implements the classical linear elastic-plastic law from [CundallStrack1979]_ (see also [Pfc3dManual30]_). The normal force is (with the convention of positive tensile forces) $F_n=\\min(k_n u_n, 0)$. The shear force is $F_s=k_s u_s$, the plasticity condition defines the maximum value of the shear force : $F_s^{\\max}=F_n\\tan(\\phi)$, with $\\phi$ the friction angle.\n\nThis law is well tested in the context of triaxial simulation, and has been used for a number of published results (see e.g. [Scholtes2009b]_ and other papers from the same authors). It is generalised by :yref:`Law2_ScGeom6D_CohFrictPhys_CohesionMoment`, which adds cohesion and moments at contact.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("neverErase", &Law2_ScGeom_FrictPhys_CundallStrack::neverErase, "Keep interactions even if particles go away from each other (only in case another constitutive law is in the scene, e.g. :yref:`Law2_ScGeom_CapillaryPhys_Capillarity`)");
	_classObj.def_readwrite("sphericalBodies", &Law2_ScGeom_FrictPhys_CundallStrack::sphericalBodies, "If true, compute branch vectors from radii (faster), else use contactPoint-position. Turning this flag true is safe for sphere-sphere contacts and a few other specific cases. It will give wrong values of torques on facets or boxes.");
	_classObj.def_readwrite("traceEnergy", &Law2_ScGeom_FrictPhys_CundallStrack::traceEnergy, "Define the total energy dissipated in plastic slips at all contacts. This will trace only plastic energy in this law, see O.trackEnergy for a more complete energies tracing");
	_classObj.def("elasticEnergy", &Law2_ScGeom_FrictPhys_CundallStrack::elasticEnergy, "Compute and return the total elastic energy in all \"FrictPhys\" contacts");
	_classObj.def("plasticDissipation", &Law2_ScGeom_FrictPhys_CundallStrack::getPlasticDissipation, "Total energy dissipated in plastic slips at all FrictPhys contacts. Computed only if :yref:`Law2_ScGeom_FrictPhys_CundallStrack::traceEnergy` is true.");
	_classObj.def("initPlasticDissipation", &Law2_ScGeom_FrictPhys_CundallStrack::initPlasticDissipation, "Initialize cummulated plastic dissipation to a value (0 by default).");
}

void Law2_ScGeom_ViscoFrictPhys_CundallStrack::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Law2_ScGeom_ViscoFrictPhys_CundallStrack");
	pybind11::class_<Law2_ScGeom_ViscoFrictPhys_CundallStrack, Law2_ScGeom_FrictPhys_CundallStrack, std::shared_ptr<Law2_ScGeom_ViscoFrictPhys_CundallStrack>> _classObj(_module, "Law2_ScGeom_ViscoFrictPhys_CundallStrack", "Law similar to :yref:`Law2_ScGeom_FrictPhys_CundallStrack` with the addition of shear creep at contacts.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("shearCreep", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::shearCreep, "Enable shear creep");
	_classObj.def_readwrite("viscosity", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::viscosity, "Viscosity coefficient");
	_classObj.def_readwrite("creepStiffness", &Law2_ScGeom_ViscoFrictPhys_CundallStrack::creepStiffness, "Creep stiffness");
}

void ElasticContactLaw::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ElasticContactLaw");
	pybind11::class_<ElasticContactLaw, GlobalEngine, std::shared_ptr<ElasticContactLaw>> _classObj(_module, "ElasticContactLaw", "[DEPRECATED] Loop over interactions applying :yref:`Law2_ScGeom_FrictPhys_CundallStrack` on all interactions.\n\n.. note::\n  Use :yref:`InteractionLoop` and :yref:`Law2_ScGeom_FrictPhys_CundallStrack` instead of this class for performance reasons.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("neverErase", &ElasticContactLaw::neverErase, "Keep interactions even if particles go away from each other (only in case another constitutive law is in the scene, e.g. :yref:`Law2_ScGeom_CapillaryPhys_Capillarity`)");
}
