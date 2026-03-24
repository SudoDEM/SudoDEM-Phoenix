/*************************************************************************
 Copyright (C) 2008 by Bruno Chareyre		                         *
*  bruno.chareyre@hmg.inpg.fr      					 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/common/FieldApplier.hpp>
#include<sudodem/core/Interaction.hpp>
//#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/common/Callbacks.hpp>
#include<sudodem/pkg/dem/GlobalStiffnessTimeStepper.hpp>
#include<sudodem/pkg/dem/Superquadrics.hpp>
#include<sudodem/pkg/dem/PolySuperellipsoid.hpp>
#include<sudodem/pkg/dem/GJKParticle.hpp>

#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

/*! An engine that can replace the usual series of engines used for integrating the laws of motion.

 */
class State;

class NewtonIntegrator : public FieldApplier{
	inline void cundallDamp1st(Vector3r& force, const Vector3r& vel);
	inline void cundallDamp2nd(const Real& dt, const Vector3r& vel, Vector3r& accel);
	inline void leapfrogTranslate(State*, const Body::id_t& id, const Real& dt); // leap-frog translate
	inline void leapfrogSphericalRotate(State*, const Body::id_t& id, const Real& dt); // leap-frog rotate of spherical body
	inline void leapfrogAsphericalRotate(State*, const Body::id_t& id, const Real& dt, const Vector3r& M); // leap-frog rotate of aspherical body
	inline void leapfrogAsphericalRotate(State*, Matrix3r&, const Body::id_t& id, const Real& dt, const Vector3r& M);
	inline void leapfrogSuperquadricsRotate(Superquadrics* ,State*, const Body::id_t& id, const Real& dt, const Vector3r& M); // leap-frog rotate of Superquadrics
	inline void leapfrogPolySuperellipsoidRotate(PolySuperellipsoid* ,State*, const Body::id_t& id, const Real& dt, const Vector3r& M); // leap-frog rotate of PolySuperellipsoid
	inline void leapfrogGJKParticleRotate(GJKParticle* ,State*, const Body::id_t& id, const Real& dt, const Vector3r& M); // leap-frog rotate of GJKParticle
	Quaternionr DotQ(const Vector3r& angVel, const Quaternionr& Q);

	// compute linear and angular acceleration, respecting State::blockedDOFs
	Vector3r computeAccel(const Vector3r& force, const Real& mass, int blockedDOFs);
	Vector3r computeAngAccel(const Vector3r& torque, const Vector3r& inertia, int blockedDOFs);

	void updateEnergy(const shared_ptr<Body>&b, const State* state, const Vector3r& fluctVel, const Vector3r& f, const Vector3r& m);
	#ifdef SUDODEM_OPENMP
	void ensureSync(); bool syncEnsured;
	#endif
	// whether the cell has changed from the previous step
	bool cellChanged;
	bool homoDeform;

	// wether a body has been selected in Qt view
	bool bodySelected;
	Matrix3r dVelGrad;

	public:
		bool densityScaling;
		Real updatingDispFactor;
		Real damping;
		unsigned int isSuperquadrics;
		bool quiet_system_flag;
		Vector3r gravity;
		Real maxVelocitySq;
		bool exactAsphericalRot;
		Matrix3r prevVelGrad;
		Vector3r prevCellSize;
		bool warnNoForceReset;
		int nonviscDampIx;
		bool kinSplit;
		int kinEnergyIx;
		int kinEnergyTransIx;
		int kinEnergyRotIx;
		int mask;
		
		#ifdef SUDODEM_BODY_CALLBACK
			vector<shared_ptr<BodyCallback> > callbacks;
		#endif

		// function to save maximum velocity, for the verlet-distance optimization
		void saveMaximaVelocity(const Body::id_t& id, State* state);
		void saveMaximaDisplacement(const shared_ptr<Body>& b);
		bool get_densityScaling ();
		void set_densityScaling (bool dsc);

		#ifdef SUDODEM_OPENMP
			vector<Real> threadMaxVelocitySq;
		#endif
		virtual void action();
		
		NewtonIntegrator() : densityScaling(false), updatingDispFactor(0), damping(0.2), isSuperquadrics(0), quiet_system_flag(false), gravity(Vector3r::Zero()), maxVelocitySq(NaN), exactAsphericalRot(true), prevVelGrad(Matrix3r::Zero()), prevCellSize(Vector3r(NaN,NaN,NaN)), warnNoForceReset(true), nonviscDampIx(-1), kinSplit(false), kinEnergyIx(-1), kinEnergyTransIx(-1), kinEnergyRotIx(-1), mask(-1), cellChanged(false), homoDeform(false), bodySelected(false), dVelGrad(Matrix3r::Zero())
		{
			#ifdef SUDODEM_OPENMP
				threadMaxVelocitySq.resize(omp_get_max_threads()); syncEnsured=false;
			#endif
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FieldApplier, damping, isSuperquadrics, quiet_system_flag, gravity, maxVelocitySq, exactAsphericalRot, prevVelGrad, prevCellSize, warnNoForceReset, nonviscDampIx, kinSplit, kinEnergyIx, kinEnergyTransIx, kinEnergyRotIx, mask, densityScaling);
		
	#ifdef SUDODEM_BODY_CALLBACK
		REGISTER_ATTRIBUTES(FieldApplier, callbacks);
	#endif
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(NewtonIntegrator, FieldApplier);
