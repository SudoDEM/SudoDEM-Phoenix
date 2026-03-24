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

#define SOLE_GAIN //move two walls with their corresponding gain in the same direction

class Scene;
class State;
class Body;

/*! \brief Compression engine for cubic specimen
 */
class CubicCompressionEngine : public BoundaryController
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
	Real width0, width;
	Real depth0, depth;
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

	// Wall arrays (cubic specific)
	Real wall_stiffness[6];
	Real pos[4];
	Real left_wpos[4];
	Real right_wpos[4];
	Real front_wpos[4];
	Real back_wpos[4];
	Real bottom_wpos[4];
	Real top_wpos[4];
	Vector3r wall_pos[6];
	Real wall_damping[6];

	// Areas
	Real x_area;
	Real y_area;
	Real z_area;

	// Stiffness and gains
	Real avg_xstiff;
	Real avg_ystiff;
	Real avg_zstiff;
	Real gain_x, gain_y, gain_z;

#ifdef SOLE_GAIN
	double wsxx_left, wsxx_right, wsyy_front, wsyy_back, wszz_top, wszz_bottom;
	Real gain_x1, gain_y1, avg_xstiff1, gain_z1, avg_ystiff1, avg_zstiff1;
#endif

	// Wall stresses
	Real left_wall_s;
	Real right_wall_s;
	Real front_wall_s;
	Real back_wall_s;
	Real bottom_wall_s;
	Real top_wall_s;

	// Wall forces
	Real f_left;
	Real f_right;
	Real f_front;
	Real f_back;
	Real f_bottom;
	Real f_top;

	// Wall velocities
	Real v_left;
	Real v_right;
	Real v_front;
	Real v_back;
	Real v_bottom;
	Real v_top;

	// Current box dimensions
	Real box_ax;
	Real box_ay;
	Real box_az;
	Real fmax;

	// Consolidation stress values
	Real wsxx, wsyy, wszz;

	// Additional parameters
	Real gain_alpha;

	CubicCompressionEngine();
	~CubicCompressionEngine();

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

	// Action methods
	void hydroConsolidation();
	void generalConsolidation();
	void checkTarget();

	// Specific methods
	void action_cubic();
	void move_wall(int wall_id, double Sign, int f_index, double (&wall_pos)[4],
	               double goal_stress, double wall_stress);
	void shear();
	void consol_ss();

	// Main action
	virtual void action() override;

	pybind11::list get_strain() const;

	// Serialization - key state variables only (max 30 attributes)
	REGISTER_ATTRIBUTES(BoundaryController,
		height0, height, width0, width, depth0, depth,
		particlesVolume, porosity,
		iterate_num_max, solve_num_max,
		goalx, goaly, goalz, f_threshold, fmin_threshold, unbf_tol,
		wall_fix, alpha,
		hydroStrain, hydroStrainRate, z_servo,
		shearMode, target_strain,
		debug, savedata_interval, file,
		externalWork, Flag_ForceTarget,
		gain_alpha);

	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(CubicCompressionEngine, BoundaryController);