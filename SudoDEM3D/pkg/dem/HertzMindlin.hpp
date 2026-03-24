// 2010 © Chiara Modenese <c.modenese@gmail.com>
//
/*
=== HIGH LEVEL OVERVIEW OF MINDLIN ===

Mindlin is a set of classes to include the Hertz-Mindlin formulation for the contact stiffnesses.
The DMT formulation is also considered (for adhesive particles, rigid and small bodies).

*/

#pragma once

#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/pkg/common/PeriodicEngines.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/pkg/common/MatchMaker.hpp>

#include <tuple>
#include<sudodem/lib/base/openmp-accu.hpp>


/******************** MindlinPhys *********************************/
class MindlinPhys: public FrictPhys{
	public:
		Real kno;
		Real kso;
		Real kr;
		Real ktw;
		Real maxBendPl;
		Vector3r normalViscous;
		Vector3r shearViscous;
		Vector3r shearElastic;
		Vector3r usElastic;
		Vector3r usTotal;
		Vector3r momentBend;
		Vector3r momentTwist;
		Real radius;
		Real adhesionForce;
		bool isAdhesive;
		bool isSliding;
		Real betan;
		Real betas;
		Real alpha;
		Vector3r prevU;
		Vector2r Fs;
		
		virtual ~MindlinPhys() {};
		
		MindlinPhys() : kno(0.0), kso(0.0), kr(0.0), ktw(0.0), maxBendPl(0.0), 
			normalViscous(Vector3r::Zero()), shearViscous(Vector3r::Zero()), 
			shearElastic(Vector3r::Zero()), usElastic(Vector3r::Zero()), usTotal(Vector3r::Zero()),
			momentBend(Vector3r::Zero()), momentTwist(Vector3r::Zero()), 
			radius(NaN), adhesionForce(0.0), isAdhesive(false), isSliding(false),
			betan(0.0), betas(0.0), alpha(0.0), prevU(Vector3r::Zero()), Fs(Vector2r::Zero()) { 
			createIndex(); 
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, kno, kso, kr, ktw, maxBendPl, normalViscous, shearViscous, 
			shearElastic, usElastic, usTotal, momentBend, momentTwist, radius, adhesionForce, 
			isAdhesive, isSliding, betan, betas, alpha, prevU, Fs);
		
	REGISTER_CLASS_INDEX_H(MindlinPhys,FrictPhys)
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(MindlinPhys, FrictPhys);



/******************** Ip2_FrictMat_FrictMat_MindlinPhys *******/
class Ip2_FrictMat_FrictMat_MindlinPhys: public IPhysFunctor{
	public :
		Real gamma;
		Real eta;
		Real krot;
		Real ktwist;
		Real constantkn;
		Real constantks;
		shared_ptr<MatchMaker> en;
		shared_ptr<MatchMaker> es;
		shared_ptr<MatchMaker> betan;
		shared_ptr<MatchMaker> betas;
		shared_ptr<MatchMaker> frictAngle;
		
		Ip2_FrictMat_FrictMat_MindlinPhys() : gamma(0.0), eta(0.0), krot(0.0), ktwist(0.0), 
			constantkn(1e6), constantks(1e6) {}
		
		virtual void go(const shared_ptr<Material>& b1,	const shared_ptr<Material>& b2,	const shared_ptr<Interaction>& interaction);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhysFunctor, gamma, eta, krot, ktwist, constantkn, constantks, en, es, betan, betas, frictAngle);
		
	FUNCTOR2D(FrictMat,FrictMat);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_MindlinPhys, IPhysFunctor);


class Law2_ScGeom_MindlinPhys_MindlinDeresiewitz: public LawFunctor{
	public:
		bool neverErase;
		
		Law2_ScGeom_MindlinPhys_MindlinDeresiewitz() : neverErase(false) {}
		
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, neverErase);
		
	FUNCTOR2D(ScGeom,MindlinPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_MindlinPhys_MindlinDeresiewitz, LawFunctor);

class Law2_ScGeom_MindlinPhys_HertzWithLinearShear: public LawFunctor{
	public:
		bool neverErase;
		int nonLin;
		
		Law2_ScGeom_MindlinPhys_HertzWithLinearShear() : neverErase(false), nonLin(0) {}
		
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, neverErase, nonLin);
		
	FUNCTOR2D(ScGeom,MindlinPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_MindlinPhys_HertzWithLinearShear, LawFunctor);


/******************** Law2_ScGeom_MindlinPhys_Mindlin *********/
class Law2_ScGeom_MindlinPhys_Mindlin: public LawFunctor{
	public:
		bool preventGranularRatcheting;
		bool includeAdhesion;
		bool calcEnergy;
		bool includeMoment;
		bool neverErase;
		OpenMPAccumulator<Real> frictionDissipation;
		OpenMPAccumulator<Real> shearEnergy;
		OpenMPAccumulator<Real> normDampDissip;
		OpenMPAccumulator<Real> shearDampDissip;
		
		Law2_ScGeom_MindlinPhys_Mindlin() : preventGranularRatcheting(true), includeAdhesion(false), 
			calcEnergy(false), includeMoment(false), neverErase(false) {}
		
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		Real normElastEnergy();
		Real adhesionEnergy();

		Real getfrictionDissipation();
		Real getshearEnergy();
		Real getnormDampDissip();
		Real getshearDampDissip();
		Real contactsAdhesive();
		Real ratioSlidingContacts();

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, preventGranularRatcheting, includeAdhesion, calcEnergy, 
			includeMoment, neverErase, frictionDissipation, shearEnergy, normDampDissip, shearDampDissip);
		
	FUNCTOR2D(ScGeom,MindlinPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_MindlinPhys_Mindlin, LawFunctor);

// The following code was moved from Ip2_FrictMat_FrictMat_MindlinCapillaryPhys.hpp

class MindlinCapillaryPhys : public MindlinPhys
{
	public :
		int currentIndexes [4]; // used for faster interpolation (stores previous positions in tables)
		bool meniscus;
		bool isBroken;
		Real capillaryPressure;
		Real vMeniscus;
		Real Delta1;
		Real Delta2;
		Vector3r fCap;
		short int fusionNumber;
		
		virtual ~MindlinCapillaryPhys() {};

		MindlinCapillaryPhys() : meniscus(false), isBroken(false), capillaryPressure(0.), vMeniscus(0.), 
			Delta1(0.), Delta2(0.), fCap(Vector3r::Zero()), fusionNumber(0) { 
			createIndex();
			currentIndexes[0]=currentIndexes[1]=currentIndexes[2]=currentIndexes[3]=0;
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(MindlinPhys, currentIndexes, meniscus, isBroken, capillaryPressure, 
			vMeniscus, Delta1, Delta2, fCap, fusionNumber);
		
	REGISTER_CLASS_INDEX_H(MindlinCapillaryPhys,MindlinPhys)
};
REGISTER_SERIALIZABLE_BASE(MindlinCapillaryPhys, MindlinPhys);


class Ip2_FrictMat_FrictMat_MindlinCapillaryPhys : public IPhysFunctor
{
	public :
		Real gamma;
		Real eta;
		Real krot;
		Real ktwist;
		shared_ptr<MatchMaker> en;
		shared_ptr<MatchMaker> es;
		shared_ptr<MatchMaker> betan;
		shared_ptr<MatchMaker> betas;
		
		Ip2_FrictMat_FrictMat_MindlinCapillaryPhys() : gamma(0.0), eta(0.0), krot(0.0), ktwist(0.0) {}
		
		virtual void go(	const shared_ptr<Material>& b1,
					const shared_ptr<Material>& b2,
					const shared_ptr<Interaction>& interaction);

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhysFunctor, gamma, eta, krot, ktwist, en, es, betan, betas);
		
	FUNCTOR2D(FrictMat,FrictMat);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_MindlinCapillaryPhys, IPhysFunctor);