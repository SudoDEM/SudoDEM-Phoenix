/*************************************************************************
*  Copyright (C) 2016 by Zhswee                                          *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once
#ifdef SUDODEM_CGAL
#include<sudodem/pkg/dem/VolumeFric.hpp>
#include<sudodem/pkg/common/BoundaryController.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/dem/Polyhedra.hpp>
#include<array>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<fstream>
#include<string>

class State;

class Scene;
class State;


/*! \brief Controls the stress on the boundaries of a box and compute strain-like and stress-like quantities for the packing. The algorithms used have been developed initialy for simulations reported in [Chareyre2002a] and [Chareyre2005]. They have been ported to SudoDEM in a second step and used in e.g. [Kozicki2008],[Scholtes2009b],[Jerier2010b].
*/

class PolyCompressionEngine : public BoundaryController
{
	private :
		bool firstRun;
		std::ofstream out;
		bool majorok;
		shared_ptr<Body> box [6];

		enum {left_wall=0,right_wall,front_wall,back_wall,bottom_wall,top_wall};

		inline const Vector3r getForce(Scene* rb, Body::id_t id){ return rb->forces.getForce(id); }
	public :
		Vector3r translationAxisy;
		Vector3r translationAxisx;
		Vector3r translationAxisz;

		Real stiffness;
		Real pos[4];
		Vector3r strain;

		Real shearStress[2];
		Real left_facet_pos;
		Real particlesVolume;
		Real previouswallpos;
		Real previousStress[2];
		Real boxVolume;
		Real porosity;
		Real fmax;
		
		// Attributes from macro
		unsigned int stiffnessUpdateInterval;
		unsigned int echo_interval;
		std::string file;
		Real height;
		Real width;
		Real depth;
		Real LP_area;
		Real f_threshold;
		Real height0;
		Real width0;
		Real depth0;
		Real wall_equal_tol;
		Real majorF_tol;
		Real unbf_tol;
		Real wall_max_vel;
		int step_interval;
		Real h0;
		Real w0;
		Real d0;
		bool Sphere_on;
		bool controlFlag;
		Real goal;
		unsigned int total_incr;
		int outinfo_interval;
		unsigned int savedata_interval;
		Real max_vel;
		Real externalWork;
		Real thickness;
		Real shearRate;
		Real currentStrainRate2;
		Real UnbalancedForce;
		Real frictionAngleDegree;
		bool updateFrictionAngle;
		bool Confining;
		std::string Key;
		
		PolyCompressionEngine() : firstRun(true), majorok(true), stiffness(0), 
			left_facet_pos(0), particlesVolume(0), previouswallpos(0),
			boxVolume(0), porosity(1), fmax(0), translationAxisy(0,1,0), 
			translationAxisx(1,0,0), translationAxisz(0,0,0),
			stiffnessUpdateInterval(10), echo_interval(10), file("recordData"),
			height(0), width(0), depth(0), LP_area(0), f_threshold(0.05),
			height0(0), width0(0), depth0(0), wall_equal_tol(0.001), majorF_tol(0.985),
			unbf_tol(0.5), wall_max_vel(1), step_interval(20), h0(0.003), w0(0.003), d0(0.003),
			Sphere_on(false), controlFlag(false), goal(0), total_incr(24000), outinfo_interval(5),
			savedata_interval(12), max_vel(1), externalWork(0), thickness(0), shearRate(0),
			currentStrainRate2(0), UnbalancedForce(1), frictionAngleDegree(-1), updateFrictionAngle(false),
			Confining(true)
		{
			previousStress[0] = 0.; previousStress[1] = 0.;
			shearStress[0] = 0.; shearStress[1] = 0.;
			for(int j = 0; j < 4; j++) pos[j] = 0;
			strain = Vector3r::Zero();
		}
		
		virtual ~PolyCompressionEngine();

		virtual void action();
		void updateStiffness();
		Real ComputeUnbalancedForce(bool maxUnbalanced=false);
		Real getResultantF();
		void recordData();
		bool checkForce(int wall, Vector3r resultantForce);
		void checkMajorF(bool init);
		Real GetOverlpV();
		void setContactProperties(Real frictionDegree);
		void loadingStress();
		void getBox();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(BoundaryController, stiffnessUpdateInterval, echo_interval, file, height, width, depth,
			LP_area, f_threshold, height0, width0, depth0, wall_equal_tol, majorF_tol, unbf_tol,
			wall_max_vel, step_interval, h0, w0, d0, Sphere_on, controlFlag, goal, total_incr,
			outinfo_interval, savedata_interval, max_vel, externalWork, thickness, shearRate,
			currentStrainRate2, UnbalancedForce, frictionAngleDegree, updateFrictionAngle, Confining,
			Key, strain, stiffness, pos, shearStress, left_facet_pos, particlesVolume, previouswallpos,
			previousStress, boxVolume, porosity, fmax, translationAxisy, translationAxisx, translationAxisz);
		
		pybind11::list get_strain() const;
		pybind11::list get_porosity() const;
		pybind11::list get_boxVolume() const;
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PolyCompressionEngine, BoundaryController);
#endif