/*************************************************************************
*  Copyright (C) 2016 by Zhswee                                          *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/pkg/common/BoundaryController.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/dem/NewtonIntegrator.hpp>
#include<fstream>
#include<string>

class Scene;
class State;
class Body;

/*! \brief Compression engine for cylindrical specimen
 */
class CylindricalCompressionEngine : public BoundaryController
{
protected:
	bool firstRun;
	std::ofstream out;
	bool majorok;
	shared_ptr<Body> box[6];
	shared_ptr<NewtonIntegrator> newton;
	enum {left_wall=0, right_wall, front_wall, back_wall, bottom_wall, top_wall};

	inline const Vector3r getForce(Scene* rb, Body::id_t id) { return rb->forces.getForce(id); }

public:
	// Box geometry
	Real height0, height;
	Real Init_boxVolume;
	Real Current_boxVolume;
	Real particlesVolume;
	Real porosity;
	Real strain[3];

	// Control parameters
	bool continueFlag;
	unsigned int iterate_num;
	unsigned int solve_num;
	unsigned int iterate_num_max;
	unsigned int solve_num_max;
	unsigned int echo_interval;
	unsigned int ramp_interval;
	unsigned int ramp_chunks;
	unsigned int rampNum;

	// Stress goals
	Real goalx, goaly, goalz;
	Real f_threshold;
	Real fmin_threshold;
	Real unbf_tol;

	// Wall control
	bool wall_fix;
	Real wall_max_vel_single;
	Real alpha;
	Real max_vel;

	// Wall activation flags
	bool left_wall_activated;
	bool right_wall_activated;
	bool front_wall_activated;
	bool back_wall_activated;
	bool bottom_wall_activated;
	bool top_wall_activated;

	// Hydro strain
	bool hydroStrain;
	Real hydroStrainRate;

	// Servo control
	bool z_servo;
	bool interStressControl;
	unsigned int shearMode;
	Real target_strain;

	// Data recording
	bool debug;
	unsigned int savedata_interval;
	std::string file;

	// Results
	Real externalWork;
	Real UnbalancedForce;
	bool Flag_ForceTarget;

	// Wall max velocity
	Real wall_max_vel[6];

	// Cylinder specific parameters
	Real wall_radius;
	Real wall_radius0;
	Real left_facet_pos;
	Real loading_area;

	// Cylinder stress and force
	Real cylinder_stress;
	Real cylinder_force;
	int num_pwall;

	// Cylinder specific gains
	Real gain_x;
	Real avg_xstiff;
	Real gain_alpha;

	// Cylinder wall stress components
	Real wszz;
	Real gain_z;
	Real avg_zstiff;

	// Wall stiffness
	Real wall_stiffness[6];

	CylindricalCompressionEngine();
	~CylindricalCompressionEngine();

	// Implementation methods
	void updateStiffness();
	void updateBoxSize();
	void getBox();
	Matrix3r getStressTensor();
	Vector3r getStress();
	void servo(Real dt);
	void get_gain();
	Real ComputeUnbalancedForce(bool maxUnbalanced=false);
	void recordData();

	// Cylinder specific methods
	void cylinwall_go();
	void servo_cylinder(Real dt);
	void consol_ss_cylinder();

	// Action methods
	void hydroConsolidation();
	void generalConsolidation();
	void checkTarget();

	// Specific action
	void action_cylindrical();

	// Main action
	virtual void action() override;

	// Serialization - key state variables only (max 30 attributes)
	REGISTER_ATTRIBUTES(BoundaryController,
		height0, height,
		particlesVolume, porosity,
		iterate_num_max, solve_num_max,
		goalx, goaly, goalz, f_threshold, fmin_threshold, unbf_tol,
		wall_fix, alpha,
		hydroStrain, hydroStrainRate, z_servo,
		shearMode, target_strain,
		debug, savedata_interval, file,
		externalWork, Flag_ForceTarget,
		wall_radius, wall_radius0, loading_area,
		gain_alpha);

	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(CylindricalCompressionEngine, BoundaryController);