/*************************************************************************
*  Copyright (C) 2014 by Klaus Thoeni                                    *
*  klaus.thoeni@newcastle.edu.au                                         *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include<sudodem/pkg/dem/FrictViscoPM.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Scene.hpp>


FrictViscoMat::~FrictViscoMat(){}

REGISTER_CLASS_INDEX_CPP(FrictViscoMat,FrictMat)
REGISTER_CLASS_INDEX_CPP(FrictViscoPhys,FrictPhys)

/********************** Ip2_FrictViscoMat_FrictMat_FrictViscoPhys ****************************/
CREATE_LOGGER(FrictViscoPhys);

FrictViscoPhys::~FrictViscoPhys(){};

/********************** Ip2_FrictViscoMat_FrictMat_FrictViscoPhys ****************************/
CREATE_LOGGER(Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys);

void Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::go(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction){

	LOG_TRACE( "Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::go - contact law" );

	if(interaction->phys) return;
	const shared_ptr<FrictViscoMat>& mat1 = SUDODEM_PTR_CAST<FrictViscoMat>(b1);
	const shared_ptr<FrictViscoMat>& mat2 = SUDODEM_PTR_CAST<FrictViscoMat>(b2);
	interaction->phys = shared_ptr<FrictViscoPhys>(new FrictViscoPhys());
	const shared_ptr<FrictViscoPhys>& contactPhysics = SUDODEM_PTR_CAST<FrictViscoPhys>(interaction->phys);

	Real Ra,Rb;
	assert(dynamic_cast<GenericSpheresContact*>(interaction->geom.get()));//only in debug mode
	GenericSpheresContact* sphCont=SUDODEM_CAST<GenericSpheresContact*>(interaction->geom.get());
	Ra=sphCont->refR1>0?sphCont->refR1:sphCont->refR2;
	Rb=sphCont->refR2>0?sphCont->refR2:sphCont->refR1;

	// calculate stiffness from MatchMaker or use harmonic avarage as usual if not given
	Real Kn = (kn) ? (*kn)(mat1->id,mat2->id) : (mat1->Kn + mat2->Kn) / 2.0;
	Real Ks = (kRatio) ? (*kRatio)(mat1->id,mat2->id)*Kn : (mat1->Ks + mat2->Ks) / 2.0;

	Real frictionAngle = (!frictAngle) ? std::min(mat1->frictionAngle,mat2->frictionAngle) : (*frictAngle)(mat1->id,mat2->id,mat1->frictionAngle,mat2->frictionAngle);
	contactPhysics->tangensOfFrictionAngle = std::tan(frictionAngle);
	contactPhysics->kn = Kn;
	contactPhysics->ks = Ks;

	/************************/
	/* DAMPING COEFFICIENTS */
	/************************/
	Real betana = mat1->betan;
	Real betanb = mat2->betan;

	// inclusion of local viscous damping
	if (betana!=0 || betanb!=0){
		Body::id_t ida = interaction->getId1(); // get id body 1
		Body::id_t idb = interaction->getId2(); // get id body 2
		State* dea = Body::byId(ida,scene)->state.get();
		State* deb = Body::byId(idb,scene)->state.get();
		const shared_ptr<Body>& ba=Body::byId(ida,scene);
		const shared_ptr<Body>& bb=Body::byId(idb,scene);
		Real mbar = (!ba->isDynamic() && bb->isDynamic()) ? deb->mass : ((!bb->isDynamic() && ba->isDynamic()) ? dea->mass : (dea->mass*deb->mass / (dea->mass + deb->mass))); // get equivalent mass if both bodies are dynamic, if not set it equal to the one of the dynamic body
		TRVAR2(Kn,mbar);
		contactPhysics->cn_crit = 2.*sqrt(mbar*Kn); // Critical damping coefficient (normal direction)
		contactPhysics->cn = contactPhysics->cn_crit * ( (betana!=0 && betanb!=0) ? ((betana+betanb)/2.) : ( (betanb==0) ? betana : betanb )); // Damping normal coefficient
	}
	else
		contactPhysics->cn=0.;
	TRVAR1(contactPhysics->cn);

}

/********************** Ip2_FrictViscoMat_FrictMat_FrictViscoPhys ****************************/
CREATE_LOGGER(Ip2_FrictMat_FrictViscoMat_FrictViscoPhys);

void Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::go(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction){

	LOG_TRACE( "Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::go - contact law" );

	if(interaction->phys) return;
	const shared_ptr<FrictMat>& mat1 = SUDODEM_PTR_CAST<FrictMat>(b1);
	const shared_ptr<FrictViscoMat>& mat2 = SUDODEM_PTR_CAST<FrictViscoMat>(b2);
	interaction->phys = shared_ptr<FrictViscoPhys>(new FrictViscoPhys());
	const shared_ptr<FrictViscoPhys>& contactPhysics = SUDODEM_PTR_CAST<FrictViscoPhys>(interaction->phys);

	Real Ra,Rb;
	assert(dynamic_cast<GenericSpheresContact*>(interaction->geom.get()));//only in debug mode
	GenericSpheresContact* sphCont=SUDODEM_CAST<GenericSpheresContact*>(interaction->geom.get());
	Ra=sphCont->refR1>0?sphCont->refR1:sphCont->refR2;
	Rb=sphCont->refR2>0?sphCont->refR2:sphCont->refR1;

	// calculate stiffness from MatchMaker or use harmonic avarage as usual if not given
	Real Kn = (kn) ? (*kn)(mat1->id,mat2->id) : (mat1->Kn + mat2->Kn) / 2.0;
	Real Ks = (kRatio) ? (*kRatio)(mat1->id,mat2->id)*Kn : (mat1->Ks + mat2->Ks) / 2.0;

	Real frictionAngle = (!frictAngle) ? std::min(mat1->frictionAngle,mat2->frictionAngle) : (*frictAngle)(mat1->id,mat2->id,mat1->frictionAngle,mat2->frictionAngle);
	contactPhysics->tangensOfFrictionAngle = std::tan(frictionAngle);
	contactPhysics->kn = Kn;
	contactPhysics->ks = Ks;

	/************************/
	/* DAMPING COEFFICIENTS */
	/************************/
	Real betanb = mat2->betan;

	// inclusion of local viscous damping
	if (betanb!=0){
		Body::id_t ida = interaction->getId1(); // get id body 1
		Body::id_t idb = interaction->getId2(); // get id body 2
		State* dea = Body::byId(ida,scene)->state.get();
		State* deb = Body::byId(idb,scene)->state.get();
		const shared_ptr<Body>& ba=Body::byId(ida,scene);
		const shared_ptr<Body>& bb=Body::byId(idb,scene);
		Real mbar = (!ba->isDynamic() && bb->isDynamic()) ? deb->mass : ((!bb->isDynamic() && ba->isDynamic()) ? dea->mass : (dea->mass*deb->mass / (dea->mass + deb->mass))); // get equivalent mass if both bodies are dynamic, if not set it equal to the one of the dynamic body
		TRVAR2(Kn,mbar);
		contactPhysics->cn_crit = 2.*sqrt(mbar*Kn); // Critical damping coefficient (normal direction)
		contactPhysics->cn = contactPhysics->cn_crit * betanb; // Damping normal coefficient
	}
	else
		contactPhysics->cn=0.;
	TRVAR1(contactPhysics->cn);

}

/********************** Law2_ScGeom_FrictViscoPhys_CundallStrackVisco ****************************/

// #if 1
Real Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::getPlasticDissipation() {return (Real) plasticDissipation;}
void Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::initPlasticDissipation(Real initVal) {plasticDissipation.reset(); plasticDissipation+=initVal;}
Real Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::elasticEnergy()
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
// #endif


CREATE_LOGGER(Law2_ScGeom_FrictViscoPhys_CundallStrackVisco);

bool Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* contact)
{
	LOG_TRACE( "Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::go - create interaction physics" );

	int id1 = contact->getId1(), id2 = contact->getId2();

	ScGeom*    geom= static_cast<ScGeom*>(ig.get());
	FrictViscoPhys* phys = static_cast<FrictViscoPhys*>(ip.get());
	if(geom->penetrationDepth <0){
		if (neverErase) {
			phys->shearForce = Vector3r::Zero();
			phys->normalForce = Vector3r::Zero();}
		else return false;
	}
	Real& un=geom->penetrationDepth;
	phys->normalForce = phys->kn*std::max(un,(Real) 0) * geom->normal;

	/************************/
	/* DAMPING CONTRIBUTION */
	/************************/

	// define shifts to handle periodicity
	const Vector3r shift2 = scene->isPeriodic ? scene->cell->intrShiftPos(contact->cellDist): Vector3r::Zero();
	const Vector3r shiftVel = scene->isPeriodic ? scene->cell->intrShiftVel(contact->cellDist): Vector3r::Zero();

	State* de1 = Body::byId(id1,scene)->state.get();
	State* de2 = Body::byId(id2,scene)->state.get();

	// get incident velocity
	Vector3r incidentV = geom->getIncidentVel(de1, de2, scene->dt, shift2, shiftVel);
	Vector3r incidentVn = geom->normal.dot(incidentV)*geom->normal; // contact normal velocity
	phys->normalViscous = phys->cn*incidentVn;
	phys->normalForce -= phys->normalViscous;

	// shear force
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

// Destructor definition to ensure proper typeinfo emission
Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::~Law2_ScGeom_FrictViscoPhys_CundallStrackVisco() {}

void FrictViscoMat::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("FrictViscoMat");
	pybind11::class_<FrictViscoMat, FrictMat, std::shared_ptr<FrictViscoMat>> _classObj(_module, "FrictViscoMat", "Material for use with the FrictViscoPM classes");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("betan", &FrictViscoMat::betan, "Fraction of the viscous damping coefficient in normal direction");
}

void FrictViscoPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("FrictViscoPhys");
	pybind11::class_<FrictViscoPhys, FrictPhys, std::shared_ptr<FrictViscoPhys>> _classObj(_module, "FrictViscoPhys", "Representation of a single interaction of the FrictViscoPM type");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("cn_crit", &FrictViscoPhys::cn_crit, "Normal viscous constant for critical damping");
	_classObj.def_readwrite("cn", &FrictViscoPhys::cn, "Normal viscous constant");
	_classObj.def_readwrite("normalViscous", &FrictViscoPhys::normalViscous, "Normal viscous component");
}

void Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys");
	pybind11::class_<Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys, IPhysFunctor, std::shared_ptr<Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys>> _classObj(_module, "Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys", "Converts 2 FrictViscoMat instances to FrictViscoPhys");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("kn", &Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::kn, "Instance of MatchMaker determining how to compute interaction's normal contact stiffnesses");
	_classObj.def_readwrite("kRatio", &Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::kRatio, "Instance of MatchMaker determining how to compute interaction's shear contact stiffnesses");
	_classObj.def_readwrite("frictAngle", &Ip2_FrictViscoMat_FrictViscoMat_FrictViscoPhys::frictAngle, "Instance of MatchMaker determining how to compute interaction's friction angle");
}

void Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_FrictMat_FrictViscoMat_FrictViscoPhys");
	pybind11::class_<Ip2_FrictMat_FrictViscoMat_FrictViscoPhys, IPhysFunctor, std::shared_ptr<Ip2_FrictMat_FrictViscoMat_FrictViscoPhys>> _classObj(_module, "Ip2_FrictMat_FrictViscoMat_FrictViscoPhys", "Converts a FrictMat and FrictViscoMat instance to FrictViscoPhys");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("kn", &Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::kn, "Instance of MatchMaker determining how to compute interaction's normal contact stiffnesses");
	_classObj.def_readwrite("kRatio", &Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::kRatio, "Instance of MatchMaker determining how to compute interaction's shear contact stiffnesses");
	_classObj.def_readwrite("frictAngle", &Ip2_FrictMat_FrictViscoMat_FrictViscoPhys::frictAngle, "Instance of MatchMaker determining how to compute interaction's friction angle");
}

void Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Law2_ScGeom_FrictViscoPhys_CundallStrackVisco");
	pybind11::class_<Law2_ScGeom_FrictViscoPhys_CundallStrackVisco, LawFunctor, std::shared_ptr<Law2_ScGeom_FrictViscoPhys_CundallStrackVisco>> _classObj(_module, "Law2_ScGeom_FrictViscoPhys_CundallStrackVisco", "Constitutive law for the FrictViscoPM.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("neverErase", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::neverErase, "Keep interactions even if particles go away from each other");
	_classObj.def_readwrite("sphericalBodies", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::sphericalBodies, "If true, compute branch vectors from radii (faster)");
	_classObj.def_readwrite("traceEnergy", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::traceEnergy, "Define the total energy dissipated in plastic slips at all contacts");
	_classObj.def("elasticEnergy", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::elasticEnergy, "Compute and return the total elastic energy in all FrictViscoPhys contacts");
	_classObj.def("plasticDissipation", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::getPlasticDissipation, "Total energy dissipated in plastic slips");
	_classObj.def("initPlasticDissipation", &Law2_ScGeom_FrictViscoPhys_CundallStrackVisco::initPlasticDissipation, "Initialize cummulated plastic dissipation to a value");
}

