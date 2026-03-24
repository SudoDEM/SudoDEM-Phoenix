#pragma once

#include<vector>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Material.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

/*! Elastic material */
class VolumeFricMat: public Material{
	public:
		Real Kn;
		Real Ks;
		Real frictionAngle;
		bool IsSplitable;
		double strength;
		
		 VolumeFricMat(double N=1e8, double S=1e5, double F=0.5) : Kn(N), Ks(S), frictionAngle(F), IsSplitable(false), strength(100) { createIndex(); }
		 double GetStrength(){return strength;};
	virtual ~VolumeFricMat(){};
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Material, Kn, Ks, frictionAngle, IsSplitable, strength);
	
	REGISTER_CLASS_INDEX_H(VolumeFricMat,Material)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(VolumeFricMat, Material);

class VolumeFricPhys: public IPhys{
	public:
		Real kn;
		Vector3r normalForce;
		Real ks;
		Vector3r shearForce;
		Real tangensOfFrictionAngle;
		
	virtual ~VolumeFricPhys(){};
	
	VolumeFricPhys() : kn(0), normalForce(Vector3r::Zero()), ks(0), shearForce(Vector3r::Zero()), tangensOfFrictionAngle(NaN) { createIndex(); }
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(IPhys, kn, normalForce, ks, shearForce, tangensOfFrictionAngle);
	
	REGISTER_CLASS_INDEX_H(VolumeFricPhys,IPhys)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(VolumeFricPhys, IPhys);

//***************************************************************************
class Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
		
	FUNCTOR2D(VolumeFricMat,VolumeFricMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys, IPhysFunctor);


class VolumetricLaw: public LawFunctor{
	public:
		OpenMPAccumulator<Real> plasticDissipation;
		Vector3r shearForce;
		bool neverErase;
		bool traceEnergy;
		int plastDissipIx;
		int elastPotentialIx;
		
		VolumetricLaw() : shearForce(Vector3r::Zero()), neverErase(false), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
		
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		Real elasticEnergy ();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, shearForce, neverErase, traceEnergy, plastDissipIx, elastPotentialIx);
		
	FUNCTOR2D(ScGeom,VolumeFricPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(VolumetricLaw, LawFunctor);