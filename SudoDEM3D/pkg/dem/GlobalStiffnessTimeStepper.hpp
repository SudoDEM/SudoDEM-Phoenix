/*************************************************************************
*  Copyright (C) 2006 by Bruno Chareyre                                  *
*  bruno.chareyre@hmg.inpg.fr                                            *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/core/TimeStepper.hpp>


/*! \brief Compute the critical timestep of the leap-frog scheme based on global stiffness of bodies
	See usage details in TriaxialTest documentation (TriaxialTest is also a good example of how to use this class)
 */

class Interaction;
class BodyContainer;
class Scene;

class GlobalStiffnessTimeStepper : public TimeStepper
{
	private :
		vector<Vector3r> stiffnesses;
		vector<Vector3r> Rstiffnesses;
		vector<Vector3r> viscosities;
		vector<Vector3r> Rviscosities;
		void computeStiffnesses(Scene*);

		Real		newDt;
		bool		computedSomething,
 				computedOnce;
		void findTimeStepFromBody(const shared_ptr<Body>& body, Scene * ncb);

	public :
		Real defaultDt;
		Real maxDt;
		Real previousDt;
		Real timestepSafetyCoefficient;
		bool densityScaling;
		Real targetDt;
		bool viscEl;
		
		GlobalStiffnessTimeStepper() : defaultDt(-1), maxDt(Mathr::MAX_REAL), previousDt(1), 
			timestepSafetyCoefficient(0.8), densityScaling(false), targetDt(1), 
			viscEl(false), newDt(0), computedSomething(false), computedOnce(false) {}
		
		virtual ~GlobalStiffnessTimeStepper();

		virtual void computeTimeStep(Scene*) override;
		virtual bool isActivated() override;
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(TimeStepper, defaultDt, maxDt, previousDt, timestepSafetyCoefficient, 
			densityScaling, targetDt, viscEl);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(GlobalStiffnessTimeStepper, TimeStepper);