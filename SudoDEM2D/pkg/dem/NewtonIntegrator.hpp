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
#include"Superellipse.hpp"

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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<NewtonIntegrator, FieldApplier, std::shared_ptr<NewtonIntegrator>> _classObj(_module, "NewtonIntegrator", "Engine integrating newtonian motion equations.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("damping", &NewtonIntegrator::damping, "damping coefficient for Cundall's non viscous damping (see [Chareyre2005]_) [-]");
			_classObj.def_readwrite("isSuperellipse", &NewtonIntegrator::isSuperellipse, "Enable optimation for Superellipse");
			_classObj.def_readwrite("quiet_system_flag", &NewtonIntegrator::quiet_system_flag, "the flag for quiet system");
			_classObj.def_readwrite("gravity", &NewtonIntegrator::gravity, "Gravitational acceleration (effectively replaces GravityEngine).");
			_classObj.def_readwrite("maxVelocitySq", &NewtonIntegrator::maxVelocitySq, "store square of max. velocity, for informative purposes; computed again at every step. |yupdate|");
			_classObj.def_readwrite("exactAsphericalRot", &NewtonIntegrator::exactAsphericalRot, "Enable more exact body rotation integrator for :yref:`aspherical bodies<Body.aspherical>` *only*, using formulation from [Allen1989]_, pg. 89.");
			_classObj.def_readwrite("prevVelGrad", &NewtonIntegrator::prevVelGrad, "Store previous velocity gradient (:yref:`Cell::velGrad`) to track acceleration. |yupdate|");
			_classObj.def_readwrite("prevCellSize", &NewtonIntegrator::prevCellSize, "cell size from previous step, used to detect change and find max velocity");
			_classObj.def_readwrite("warnNoForceReset", &NewtonIntegrator::warnNoForceReset, "Warn when forces were not resetted in this step by :yref:`ForceResetter`; this mostly points to :yref:`ForceResetter` being forgotten incidentally and should be disabled only with a good reason.");
			_classObj.def_readwrite("nonviscDampIx", &NewtonIntegrator::nonviscDampIx, "Index of the energy dissipated using the non-viscous damping (:yref:`damping<NewtonIntegrator.damping>`).");
			_classObj.def_readwrite("kinSplit", &NewtonIntegrator::kinSplit, "Whether to separately track translational and rotational kinetic energy.");
			_classObj.def_readwrite("kinEnergyIx", &NewtonIntegrator::kinEnergyIx, "Index for kinetic energy in scene->energies.");
			_classObj.def_readwrite("kinEnergyTransIx", &NewtonIntegrator::kinEnergyTransIx, "Index for translational kinetic energy in scene->energies.");
			_classObj.def_readwrite("kinEnergyRotIx", &NewtonIntegrator::kinEnergyRotIx, "Index for rotational kinetic energy in scene->energies.");
			_classObj.def_readwrite("mask", &NewtonIntegrator::mask, "If mask defined and the bitwise AND between mask and body`s groupMask gives 0, the body will not move/rotate. Velocities and accelerations will be calculated not paying attention to this parameter.");
			_classObj.def_property("densityScaling", &NewtonIntegrator::get_densityScaling, &NewtonIntegrator::set_densityScaling, "if True, then density scaling [Pfc3dManual30]_ will be applied in order to have a critical timestep equal to :yref:`GlobalStiffnessTimeStepper::targetDt` for all bodies. This option makes the simulation unrealistic from a dynamic point of view, but may speedup quasistatic simulations. In rare situations, it could be useful to not set the scalling factor automatically for each body (which the time-stepper does). In such case revert :yref:`GlobalStiffnessTimeStepper.densityScaling` to False.");
		}
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(NewtonIntegrator, FieldApplier);
