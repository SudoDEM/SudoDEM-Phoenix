#pragma once

#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<tuple>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>


// The following code was moved from RolFrictMat.hpp
class RolFrictMat : public FrictMat
{
	public :
		Real Kn;
		Real Ks;
		Real alphaKr;
		Real alphaKtw;
		Real etaRoll;
		Real etaTwist;
		
		virtual ~RolFrictMat () {};

		RolFrictMat() : Kn(1e7), Ks(1e7), alphaKr(2.0), alphaKtw(2.0), etaRoll(-1.), etaTwist(-1.) { createIndex(); }
		
		// Constructor with common parameters for Python compatibility
		RolFrictMat(Real _Kn, Real _Ks, Real _frictionAngle, Real _density) : Kn(_Kn), Ks(_Ks), alphaKr(2.0), alphaKtw(2.0), etaRoll(-1.), etaTwist(-1.) {
			createIndex();
			frictionAngle = _frictionAngle;
			density = _density;
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictMat, Kn, Ks, alphaKr, alphaKtw, etaRoll, etaTwist);
		
	REGISTER_CLASS_INDEX_H(RolFrictMat,FrictMat)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(RolFrictMat, FrictMat);

// The following code was moved from RolFrictPhys.hpp
class RolFrictPhys : public FrictPhys
{
	public :
		Real kr;
		Real ktw;
		Real maxRollPl;
		Real maxTwistPl;
		Real creep_viscosity;
		Vector3r moment_twist;
		Vector3r moment_bending;
		
		virtual ~RolFrictPhys() {};

		RolFrictPhys() : kr(0), ktw(0), maxRollPl(0.0), maxTwistPl(0.0), creep_viscosity(-1), moment_twist(Vector3r::Zero()), moment_bending(Vector3r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, kr, ktw, maxRollPl, maxTwistPl, creep_viscosity, moment_twist, moment_bending);
		
	REGISTER_CLASS_INDEX_H(RolFrictPhys,FrictPhys)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(RolFrictPhys, FrictPhys);

class RollingResistanceLaw: public LawFunctor{
	public:
		OpenMPAccumulator<Real> plasticDissipation;
		bool momentRotationLaw;
		bool traceEnergy;
		bool use_rolling_resistance;
		int elastPotentialIx;
		int shearDissipIx;
		int bendingDissipIx;
		int twistDissipIx;
		
		Real normElastEnergy();
		Real shearElastEnergy();
		Real bendingElastEnergy();
		Real twistElastEnergy();
		Real totalElastEnergy();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		virtual bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		
		RollingResistanceLaw() : momentRotationLaw(true), traceEnergy(false), use_rolling_resistance(true), elastPotentialIx(-1), shearDissipIx(-1), bendingDissipIx(-1), twistDissipIx(-1) {}
		
		RollingResistanceLaw(bool use_rolling_resistance_) : momentRotationLaw(true), traceEnergy(false), use_rolling_resistance(use_rolling_resistance_), elastPotentialIx(-1), shearDissipIx(-1), bendingDissipIx(-1), twistDissipIx(-1) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, momentRotationLaw, traceEnergy, use_rolling_resistance, elastPotentialIx, shearDissipIx, bendingDissipIx, twistDissipIx);
		
	FUNCTOR2D(ScGeom,RolFrictPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(RollingResistanceLaw, LawFunctor);


// The following code was moved from Ip2_RolFrictMat_RolFrictMat_RolFrictPhys.hpp
class Ip2_RolFrictMat_RolFrictMat_RolFrictPhys : public IPhysFunctor
{
	public :
		virtual void go(	const shared_ptr<Material>& b1,
					const shared_ptr<Material>& b2,
					const shared_ptr<Interaction>& interaction);
		
	FUNCTOR2D(RolFrictMat,RolFrictMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Ip2_RolFrictMat_RolFrictMat_RolFrictPhys, IPhysFunctor);