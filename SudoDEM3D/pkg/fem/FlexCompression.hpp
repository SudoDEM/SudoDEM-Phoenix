/*************************************************************************
*  Copyright (C) 2017 by Sway Zhao                                       *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/pkg/common/BoundaryController.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/dem/Superquadrics.hpp>
#include<array>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/pkg/common/Facet.hpp>

#include<sudodem/pkg/fem/Node.hpp>
#include<sudodem/pkg/fem/TriElement.hpp>

#include<fstream>
#include<string>
#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

#define CUBIC_SAMPLE//
#define SOLE_GAIN //move two walls with their corresponding gain in the same direction
//testing contact model
#define FM//FricMat

class State;

class Scene;
class State;


/*! \brief Controls the stress on the boundaries of a box and compute strain-like and stress-like quantities for the packing. The algorithms used have been developed initialy for simulations reported in [Chareyre2002a] and [Chareyre2005]. They have been ported to SudoDEM in a second step and used in e.g. [Kozicki2008],[Scholtes2009b],[Jerier2010b].
*/

class FlexCompressionEngine : public BoundaryController
{
	private :
		//! is this the beginning of the simulation, after reading the scene? -> it is the first time that SudoDEM passes trought the engine ThreeDTriaxialEngine
		bool firstRun;
		std::ofstream out;
		bool majorok;
        int numWallFacet;//number of all walls and TriElements
        shared_ptr<Body> top_wall,bottom_wall;
        vector<shared_ptr<Node>> top_nodes;//nodes contacting the top wall
        vector<shared_ptr<Node>> bottom_nodes;//nodes contacting the bottom wall
        vector<shared_ptr<Node>> free_nodes;//other nodes without contacting the walls
        vector<shared_ptr<TriElement>> Elements;//store elements of the flexible wall
		inline const Vector3r getForce(Scene* rb, Body::id_t id){ return rb->forces.getForce(id); /* needs sync, which is done at the beginning of action */ }
	public :
		//! The value of stiffness (updated according to stiffnessUpdateInterval)
		Real wall_stiffness[6];//left_wall=0,right_wall,front_wall,back_wall,bottom_wall,top_wall

		Real 	pos[4];

		Real 	bottom_wpos[4];
		Real 	top_wpos[4];
		Real x_area; //area in the x axis
        Real loading_area;//for cylinder wall
		Vector2r wall_pos[6];//left_wall=0,right_wall,front_wall,back_wall,bottom_wall,top_wall
		//stiffness in x, y, z direction
        Real avg_rstiff,avg_zstiff;
        Real gain_r,gain_z;

        double wsxx,wsyy,wszz;//stress in the three directions
        double cylinder_force;
        double cylinder_stress;
        unsigned int num_pwall;//number of particles touching with the cylindrical wall
        unsigned int iterate_num,solve_num;
        bool Flag_ForceTarget;//forces acting on walls reach the target or not

		Real shearStress[2];//left and right wall
		Real left_facet_pos;
		//! Value of spheres volume (solid volume)
		Real particlesVolume;

		Real previousStress [2];//shear stress at the previous step.
		//! Value of box volume
		Real Init_boxVolume;
		Real Current_boxVolume;
		//! Sample porosity
		Real porosity;
		Real fmax;//the maximum of shear f when shearing
		//strain
		Real strain[3];

		// Member variables extracted from SUDODEM_CLASS_BASE_DOC_ATTRS_DEPREC_INIT_CTOR_PY
		double wall_radius;
		bool wall_fix;
		double gain_alpha;
		double target_strain;
		unsigned int iterate_num_max;
		unsigned int solve_num_max;
		bool z_servo;
		bool isFlexible;
		bool keepRigid;
		unsigned int stiffnessUpdateInterval;
		unsigned int echo_interval;
		unsigned int ramp_interval;
		unsigned int ramp_chunks;
		unsigned int stressMask;
		bool bottom_wall_activated;
		bool top_wall_activated;
		std::string file;
		Real height;
		Real f_threshold;
		Real fmin_threshold;
		Real alpha;
		Real wall_radius0;
		Real height0;
		Real wall_equal_tol;
		Real majorF_tol;
		Real unbf_tol;
		Real wall_max_vel;
		int step_interval;
		bool Sphere_on;
		bool hydroStrain;
		Real hydroStrainRate;
		bool debugMode;
		Real goalr;
		Real goalz;
		unsigned int total_incr;
		int outinfo_interval;
		unsigned int savedata_interval;
		Real max_vel;
		Real externalWork;
		Real shearRate;
		Real currentStrainRate2;
		Real UnbalancedForce;
		Real frictionAngleDegree;
		bool updateFrictionAngle;
		bool Confining;
		bool truncate;
		std::string Key;

		virtual ~FlexCompressionEngine();

		virtual void action();
        double get_particleVolume();
        double get_boxVolume();
		Vector2r getStress();
        void initialize();
        void get_boundary_nodes();
        int get_numWallFacet();
        //for rigid cylindrical wall
        void rigid_consolidate();
        void rigid_shear();
        void fix_wall();
        void free_wall();
        void applySurfaceLoad();
        //for flexible cylindrical wall
        void flexible_consolidate();
        void flexible_shear();
		//! Compute the mean/max unbalanced force in the assembly (normalized by mean contact force)
    	Real ComputeUnbalancedForce(bool maxUnbalanced=false);
		Real getResultantF();//resultant force acting on the walls and particles in the down box.
		//recording
		void recordData();

		//check force on walls
		bool checkForce(int wall, Vector3r resultantForce);
		void checkMajorF(bool init);//calculate stress of bottom and top walls

		///Change physical propertieaabbs of interactions and/or bodies in the middle of a simulation (change only friction for the moment, complete this function to set cohesion and others before compression test)
		void setContactProperties(Real frictionDegree);
		void loadingStress();



        void get_gainz();
        void servo(double dt);
        void servo_cylinder(double dt);

        void consol_ss_cylinder();
        void checkTarget();
        void quiet_system();
        void cylinwall_go();
        void shear();
        unsigned int rampNum;//chunck number for accelerating walls

		//! Default constructor
		FlexCompressionEngine() {
			firstRun = true;
			majorok = true;
			strain[0]=strain[1]=strain[2] = 0;
			x_area = 0;
			numWallFacet = 0;
			porosity=1;
			fmax = 0;
			rampNum = 0;
			Init_boxVolume=0;Current_boxVolume=0;
			avg_rstiff = avg_zstiff = 0.0;
			gain_z = gain_r = 0.0;
			iterate_num = solve_num =0;
			Flag_ForceTarget = false;
			// Initialize member variables
			wall_radius = 5.0;
			wall_fix = true;
			gain_alpha = 0.5;
			target_strain = 0.15;
			iterate_num_max = 100;
			solve_num_max = 2000;
			z_servo = true;
			isFlexible = false;
			keepRigid = true;
			stiffnessUpdateInterval = 100;
			echo_interval = 10;
			ramp_interval = 10000;
			ramp_chunks = 400;
			stressMask = 7;
			bottom_wall_activated = true;
			top_wall_activated = true;
			file = "recordData";
			height = 0;
			f_threshold = 0.05;
			fmin_threshold = 0.2;
			alpha = 0.5;
			wall_radius0 = 0;
			height0 = 0;
			wall_equal_tol = 0.001;
			majorF_tol = 0.985;
			unbf_tol = 0.5;
			wall_max_vel = 1.;
			step_interval = 20;
			Sphere_on = false;
			hydroStrain = false;
			hydroStrainRate = 0.0;
			debugMode = false;
			goalr = 0;
			goalz = 0;
			total_incr = 24000;
			outinfo_interval = 5;
			savedata_interval = 12;
			max_vel = 1;
			externalWork = 0;
			shearRate = 0;
			currentStrainRate2 = 0;
			UnbalancedForce = 1;
			frictionAngleDegree = -1;
			updateFrictionAngle = false;
			Confining = true;
			Key = "";
		}

		//! Python constructor
		static std::shared_ptr<FlexCompressionEngine> create() { return std::make_shared<FlexCompressionEngine>(); }

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

		DECLARE_LOGGER;
	REGISTER_ATTRIBUTES(FlexCompressionEngine,
			wall_radius, wall_fix, gain_alpha, target_strain, iterate_num_max, solve_num_max,
			z_servo, isFlexible, keepRigid, stiffnessUpdateInterval, echo_interval, ramp_interval, ramp_chunks,
			stressMask, bottom_wall_activated, top_wall_activated, file, height,
			f_threshold, fmin_threshold, alpha, Sphere_on, hydroStrain, truncate
		);
};
REGISTER_SERIALIZABLE(FlexCompressionEngine);

