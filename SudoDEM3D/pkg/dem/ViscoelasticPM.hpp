// 2009 © Sergei Dorofeenko <sega@users.berlios.de>
// This file contains a set of classes for modelling of viscoelastic
// particles.

#pragma once

#include<sudodem/pkg/common/ElastMat.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/pkg/dem/DemXDofGeom.hpp>
#include<sudodem/pkg/common/MatchMaker.hpp>

#ifdef SUDODEM_SPH
#include<sudodem/pkg/common/SPHEngine.hpp>
#endif


/* Simple viscoelastic model */

/// Material
/// Note: Shop::getViscoelasticFromSpheresInteraction can get kn,cn,ks,cs from a analytical solution of a pair spheres interaction problem.
class ViscElMat : public FrictMat {
	public:
		Real tc;
		Real en;
		Real et;
		Real kn;
		Real cn;
		Real ks;
		Real cs;
		Real mR;
		unsigned int mRtype;
		
#ifdef SUDODEM_SPH
		bool SPHmode;
		Real mu;
		int KernFunctionPressure;
		int KernFunctionVisco;
#endif
		
		virtual ~ViscElMat();
		
		ViscElMat() : tc(NaN), en(NaN), et(NaN), kn(NaN), cn(NaN), ks(NaN), cs(NaN), mR(0.0), mRtype(1)
#ifdef SUDODEM_SPH
			, SPHmode(false), mu(-1), KernFunctionPressure(2), KernFunctionVisco(3)
#endif
		{ createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictMat, tc, en, et, kn, cn, ks, cs, mR, mRtype);
#ifdef SUDODEM_SPH
		REGISTER_ATTRIBUTES(FrictMat, SPHmode, mu, KernFunctionPressure, KernFunctionVisco);
#endif
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(ViscElMat,FrictMat)
};
REGISTER_SERIALIZABLE_BASE(ViscElMat, FrictMat);

/// Interaction physics
class ViscElPhys : public FrictPhys{
	public:
		Real R;
		Real cn;
		Real cs;
		Real mR;
		unsigned int mRtype;
		
#ifdef SUDODEM_SPH
		bool SPHmode;
		Real h;
		Real mu;
		KernelFunction kernelFunctionCurrentPressure;
		KernelFunction kernelFunctionCurrentVisco;
#endif
		
		virtual ~ViscElPhys();
		
		ViscElPhys() : R(0), cn(NaN), cs(NaN), mR(0.0), mRtype(1)
#ifdef SUDODEM_SPH
			, SPHmode(false), h(-1), mu(-1)
#endif
		{ createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, R, cn, cs, mR, mRtype);
#ifdef SUDODEM_SPH
		REGISTER_ATTRIBUTES(FrictPhys, SPHmode, h, mu);
#endif
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(ViscElPhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(ViscElPhys, FrictPhys);

/// Convert material to interaction physics.
// Uses the rule of consecutively connection.
class Ip2_ViscElMat_ViscElMat_ViscElPhys: public IPhysFunctor {
	public :
		shared_ptr<MatchMaker> tc;
		shared_ptr<MatchMaker> en;
		shared_ptr<MatchMaker> et;
		
		virtual void go(const shared_ptr<Material>& b1,
					const shared_ptr<Material>& b2,
					const shared_ptr<Interaction>& interaction);
					
		virtual void Calculate_ViscElMat_ViscElMat_ViscElPhys(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction, shared_ptr<ViscElPhys> phys);
	FUNCTOR2D(ViscElMat,ViscElMat);

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(IPhysFunctor, tc, en, et);
		
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_ViscElMat_ViscElMat_ViscElPhys, IPhysFunctor);

/// Constitutive law
/// This class provides linear viscoelastic contact model
class Law2_ScGeom_ViscElPhys_Basic: public LawFunctor {
	public :
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
	public :
	FUNCTOR2D(ScGeom,ViscElPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_ScGeom_ViscElPhys_Basic, LawFunctor);

Real contactParameterCalculation(const Real& l1,const Real& l2);
bool computeForceTorqueViscEl(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I, Vector3r & force, Vector3r & torque1, Vector3r & torque2);
