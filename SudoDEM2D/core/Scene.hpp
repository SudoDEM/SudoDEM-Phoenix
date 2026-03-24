/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/core/Body.hpp>
#include<sudodem/core/Cell.hpp>
#include<sudodem/core/BodyContainer.hpp>
#include<sudodem/core/Engine.hpp>
#include<sudodem/core/Material.hpp>
#include<sudodem/core/DisplayParameters.hpp>
#include<sudodem/core/ForceContainer.hpp>
#include<sudodem/core/InteractionContainer.hpp>
#include<sudodem/core/EnergyTracker.hpp>
#include<pybind11/pybind11.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

class Bound;
#ifdef SUDODEM_OPENGL
	class OpenGLRenderer;
#endif

#ifdef SUDODEM_LIQMIGRATION
struct intReal {
	public:
		id_t id;
		Real Vol;
};
#endif

class Scene: public Serializable{
	public:
		// Member variables for serialization
		Real dt = 1e-8;
		long iter = 0;
		bool subStepping = false;
		int subStep = -1;
		Real time = 0;
		Real speed = 0;
		long stopAtIter = 0;
		Real stopAtTime = 0;
		bool isPeriodic = false;
		bool trackEnergy = false;
		bool doSort = false;
		bool runInternalConsistencyChecks = true;
		Body::id_t selectedBody = -1;
		int flags = 0;
		list<string> tags;
		vector<shared_ptr<Engine>> engines;
		vector<shared_ptr<Engine>> _nextEngines;
		shared_ptr<BodyContainer> bodies = shared_ptr<BodyContainer>(new BodyContainer());
		shared_ptr<InteractionContainer> interactions = shared_ptr<InteractionContainer>(new InteractionContainer());
		shared_ptr<EnergyTracker> energy = shared_ptr<EnergyTracker>(new EnergyTracker());
		vector<shared_ptr<Material>> materials;
		shared_ptr<Bound> bound;
		shared_ptr<Cell> cell = shared_ptr<Cell>(new Cell());
		vector<shared_ptr<Serializable>> miscParams;
		vector<shared_ptr<DisplayParameters>> dispParams;

		//! Adds material to Scene::materials. It also sets id of the material accordingly and returns it.
		int addMaterial(shared_ptr<Material> m){ materials.push_back(m); m->id=(int)materials.size()-1; return m->id; }
		//! Checks that type of Body::state satisfies Material::stateTypeOk. Throws runtime_error if not. (Is called from BoundDispatcher the first time it runs)
		void checkStateTypes();
		//! update our bound; used directly instead of a BoundFunctor, since we don't derive from Body anymore
		void updateBound();

		// neither serialized, nor accessible from python (at least not directly)
		ForceContainer forces;
    //    NodeForceContainer nodeforces;
		// initialize tags (author, date, time)
		void fillDefaultTags();
		// advance by one iteration by running all engines
		void moveToNextTimeStep();

		/* Functions operating on TimeStepper; they all throw exception if there is more than 1 */
		// return whether a TimeStepper is present
		bool timeStepperPresent();
		// true if TimeStepper is present and active, false otherwise
		bool timeStepperActive();
		// (de)activate TimeStepper; returns whether the operation was successful (i.e. whether a TimeStepper was found)
		bool timeStepperActivate(bool activate);
		static const int nSpeedIter = 10;       //Number of iterations, which are taking into account for speed calculation
		Eigen::Matrix<Real,nSpeedIter,1> SpeedElements; //Array for saving speed-values for last "nSpeedIter"-iterations


		shared_ptr<Engine> engineByName(const string& s);

		#ifdef SUDODEM_LIQMIGRATION
			OpenMPVector<Interaction* > addIntrs;    //Array of added interactions, needed for liquid migration.
			OpenMPVector<intReal > delIntrs;     //Array of deleted interactions, needed for liquid migration.
		#endif

		#ifdef SUDODEM_OPENGL
			shared_ptr<OpenGLRenderer> renderer;
		#endif

		void postLoad(Scene&);

		// bits for Scene::flags
		enum { LOCAL_COORDS=1, COMPRESSION_NEGATIVE=2 }; /* add powers of 2 as needed */
		// convenience accessors
		bool usesLocalCoords() const { return flags & LOCAL_COORDS; }
		void setLocalCoords(bool d){ if(d) flags|=LOCAL_COORDS; else flags&=~(LOCAL_COORDS); }
		bool compressionNegative() const { return flags & COMPRESSION_NEGATIVE; }
		void setCompressionNegative(bool d){ if(d) flags|=COMPRESSION_NEGATIVE; else flags&=~(COMPRESSION_NEGATIVE); }
		std::chrono::system_clock::time_point prevTime; //Time value on the previous step

	public:
		virtual void pyRegisterClass(pybind11::module_ _module) override;

		// Use REGISTER_ATTRIBUTES macro for serialization - automatically handles preLoad/postLoad
		REGISTER_ATTRIBUTES(Serializable,
			dt, iter, subStepping, subStep, time, speed, stopAtIter, stopAtTime,
			isPeriodic, trackEnergy, doSort, runInternalConsistencyChecks, selectedBody, flags,
			tags, engines, _nextEngines, bodies, interactions, energy, materials, bound, cell,
			miscParams, dispParams
		);

	DECLARE_LOGGER;

	REGISTER_CLASS_NAME_DERIVED(Scene);
	REGISTER_BASE_CLASS_NAME_DERIVED(Serializable);
};
REGISTER_SERIALIZABLE(Scene);