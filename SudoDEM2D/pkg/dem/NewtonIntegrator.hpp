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
//#include<sudodem/pkg/dem/GlobalStiffnessTimeStepper.hpp>
#include<sudodem/pkg/dem/Superellipse.hpp>

#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

/*! An engine that can replace the usual series of engines used for integrating the laws of motion.

 */
class State;

class NewtonIntegrator : public FieldApplier{
	inline void cundallDamp1st(Vector2r& force, const Vector2r& vel);
	inline void cundallDamp2nd(const Real& dt, const Vector2r& vel, Vector2r& accel);
	inline void cundallDamp1st(Real& m, const Real& angvel);
	inline void cundallDamp2nd(const Real& dt, const Real& angvel, Real& accel);
	inline void leapfrogTranslate(State*, const Body::id_t& id, const Real& dt); // leap-frog translate
	inline void leapfrogSphericalRotate(State*, const Body::id_t& id, const Real& dt); // leap-frog rotate of spherical body
	inline void leapfrogAsphericalRotate(State*, const Body::id_t& id, const Real& dt, const Real& M); // leap-frog rotate of aspherical body
	inline void leapfrogSuperellipseRotate(Superellipse* shape,State* state, const Body::id_t& id, const Real& dt);
	//inline void leapfrogSuperellipseRotate(Superellipse* ,State*, const Body::id_t& id, const Real& dt, const Real& M); // leap-frog rotate of Superellipse
	//Quaternionr DotQ(const Vector3r& angVel, const Quaternionr& Q);

	// compute linear and angular acceleration, respecting State::blockedDOFs
	Vector2r computeAccel(const Vector2r& force, const Real& mass, int blockedDOFs);
	Real computeAngAccel(const Real& torque, const Real& inertia, int blockedDOFs);

	void updateEnergy(const shared_ptr<Body>&b, const State* state, const Vector2r& fluctVel, const Vector2r& f, const Real& m);
	#ifdef SUDODEM_OPENMP
	void ensureSync(); bool syncEnsured;
	#endif
	// whether the cell has changed from the previous step
	bool cellChanged;
	bool homoDeform;

	// wether a body has been selected in Qt view
	bool bodySelected;
	Matrix2r dVelGrad;

	public:
		bool densityScaling;// internal for density scaling
		Real updatingDispFactor;//(experimental) Displacement factor used to trigger bound update: the bound is updated only if updatingDispFactor*disp>sweepDist when >0, else all bounds are updated.
		Real damping;
		bool isSuperellipse;
		bool quiet_system_flag;
		Vector2r gravity;
		Real maxVelocitySq;
		bool exactAsphericalRot;
		Matrix2r prevVelGrad;
		Vector2r prevCellSize;
		bool warnNoForceReset;
		int nonviscDampIx;
		bool kinSplit;
		int kinEnergyIx;
		int kinEnergyTransIx;
		int kinEnergyRotIx;
		int mask;
		std::string label;
		
		#ifdef SUDODEM_BODY_CALLBACK
			vector<shared_ptr<BodyCallback> > callbacks;
		#endif
		
		#ifdef SUDODEM_OPENMP
			vector<Real> threadMaxVelocitySq;
		#endif
		
		// function to save maximum velocity, for the verlet-distance optimization
		void saveMaximaVelocity(const Body::id_t& id, State* state);
		void saveMaximaDisplacement(const shared_ptr<Body>& b);
		bool get_densityScaling ();
		void set_densityScaling (bool dsc);
		virtual void action() override;
		
		NewtonIntegrator() : densityScaling(false), updatingDispFactor(-1), damping(0.2), isSuperellipse(false), quiet_system_flag(false), gravity(Vector2r::Zero()), maxVelocitySq(NaN), exactAsphericalRot(true), prevVelGrad(Matrix2r::Zero()), prevCellSize(Vector2r(NaN,NaN)), warnNoForceReset(true), nonviscDampIx(-1), kinSplit(false), kinEnergyIx(-1), kinEnergyTransIx(-1), kinEnergyRotIx(-1), mask(-1) {
			#ifdef SUDODEM_OPENMP
				threadMaxVelocitySq.resize(omp_get_max_threads()); syncEnsured=false;
			#endif
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FieldApplier, densityScaling, updatingDispFactor, damping, isSuperellipse, quiet_system_flag, gravity, maxVelocitySq, exactAsphericalRot, prevVelGrad, prevCellSize, warnNoForceReset, nonviscDampIx, kinSplit, kinEnergyIx, kinEnergyTransIx, kinEnergyRotIx, mask);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(NewtonIntegrator, FieldApplier);
