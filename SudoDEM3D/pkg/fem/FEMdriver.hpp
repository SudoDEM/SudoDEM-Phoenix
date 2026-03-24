/*************************************************************************
 Copyright (C) 2017 by Sway Zhao		                                 *
*  zhswee@gmail.com      				                            	 *
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

#include <sudodem/pkg/fem/TriElement.hpp>

#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

class FEMdriver : public FieldApplier{
public:
	inline void cundallDamp1st(Vector3r& force, const Vector3r& vel);
	inline void cundallDamp2nd(const Real& dt, const Vector3r& vel, Vector3r& accel);
	Quaternionr DotQ(const Vector3r& angVel, const Quaternionr& Q);

	// compute linear and angular acceleration, respecting State::blockedDOFs
	Vector3r computeAccel(const Vector3r& force, const Real& mass, int blockedDOFs);
	Vector3r computeAngAccel(const Vector3r& torque, const Vector3r& inertia, int blockedDOFs);
    //TriElement force
    void applyNodalForces();
    void driveNodes();
	#ifdef SUDODEM_OPENMP
	void ensureSync(); bool syncEnsured;
	#endif
	Matrix3r dVelGrad;

		#ifdef SUDODEM_OPENMP
			vector<Real> threadMaxVelocitySq;
		#endif

		virtual void action();

	// Member variables
	Real damping;
	bool bending;
	bool rotIncr;
	Real thickness;
	Real young;
	Real nu;
	Real bendThickness;
	Vector3r gravity;
	Real maxVelocitySq;
	Vector3r prevCellSize;
	bool warnNoForceReset;
	int mask;

	//! Default constructor
	FEMdriver() : damping(0.2), bending(false), rotIncr(false), thickness(0), young(0), nu(0), bendThickness(0), gravity(Vector3r::Zero()), maxVelocitySq(NaN), prevCellSize(Vector3r(NaN, NaN, NaN)), warnNoForceReset(true), mask(-1) {
		#ifdef SUDODEM_OPENMP
			threadMaxVelocitySq.resize(omp_get_max_threads()); syncEnsured=false;
		#endif
	}

	//! Python constructor
	static std::shared_ptr<FEMdriver> create() { return std::make_shared<FEMdriver>(); }

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	REGISTER_ATTRIBUTES(GlobalEngine, damping, bending, rotIncr, thickness, young, nu, bendThickness, gravity, maxVelocitySq, prevCellSize, warnNoForceReset, mask);
};
REGISTER_SERIALIZABLE(FEMdriver);

