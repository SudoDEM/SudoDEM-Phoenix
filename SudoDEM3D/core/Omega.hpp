/*************************************************************************
* Copyright (C) 2004 by Olivier Galizzi         *
* olivier.galizzi@imag.fr            *
* Copyright (C) 2004 by Janek Kozicki         *
* cosurgi@berlios.de             *
*                  *
* This program is free software; it is licensed under the terms of the *
* GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

// Workaround for Qt/Python slots conflict
#ifdef slots
#undef slots
#endif
#include <Python.h>
#ifdef slots
#define slots Q_SLOTS
#endif
#include <fstream>
#include <time.h>
#include <thread>
#include <iostream>
#include <chrono>

#include <sudodem/lib/base/Math.hpp>
#include <sudodem/lib/base/Logging.hpp>
// ClassFactory removed - using ClassRegistry instead

#include <sudodem/lib/base/Singleton.hpp>

#include <sudodem/core/SimulationFlow.hpp>



class Scene;
class ThreadRunner;

struct DynlibDescriptor{
	set<string> baseClasses;
	bool isSerializable;
};

class Omega: public Singleton<Omega>{
	shared_ptr<ThreadRunner> simulationLoop;
	SimulationFlow simulationFlow_;
	map<string,DynlibDescriptor> dynlibs;
	void buildDynlibDatabase(const vector<string>& dynlibsList);

	vector<shared_ptr<Scene> > scenes;
	int currentSceneNb = 0;
	shared_ptr<Scene> sceneAnother; // used for temporarily running different simulation, in Omega().switchscene()

	std::chrono::system_clock::time_point startupLocalTime;

	map<string,string> memSavedSimulations;

	// to avoid accessing simulation when it is being loaded (should avoid crashes with the UI)
	std::mutex loadingSimulationMutex;
	std::mutex tmpFileCounterMutex;
	long tmpFileCounter;
	std::string tmpFileDir;

	public:
		// management, not generally useful
		void init();
		void reset();
		void timeInit();
		void initTemps();
		void cleanupTemps();
		const map<string,DynlibDescriptor>& getDynlibsDescriptor();
		void loadPlugins(vector<string> pluginFiles);
		bool isInheritingFrom(const string& className, const string& baseClassName );
		bool isInheritingFrom_recursive(const string& className, const string& baseClassName );
		void createSimulationLoop();
		bool hasSimulationLoop(){return (bool)(simulationLoop);}
		string gdbCrashBatch;
		char** origArgv=nullptr; 
		int origArgc=0;
		// do not change by hand
		/* Mutex for:
		* 1. GLViewer::paintGL (deffered lock: if fails, no GL painting is done)
		* 2. other threads that wish to manipulate GL
		* 3. Omega when substantial changes to the scene are being made (bodies being deleted, simulation loaded etc) so that GL doesn't access those and crash */
		std::timed_mutex renderMutex;


		void run();
		void pause();
		void step();
		void stop(); // resets the simulationLoop
		bool isRunning();
		std::string sceneFile; // updated at load/save automatically
		void loadSimulation(const string& name, bool quiet=false);
		void saveSimulation(const string& name, bool quiet=false);

		void resetScene();
		void resetCurrentScene();
		void resetAllScenes();
		const shared_ptr<Scene>& getScene();
		int addScene();
		void switchToScene(int i);
		//! Return unique temporary filename. May be deleted by the user; if not, will be deleted at shutdown.
		string tmpFilename();
		Real getRealTime();
    std::chrono::milliseconds getRealTime_duration();

		// configuration directory used for logging config and possibly other things
		std::string confDir;

	DECLARE_LOGGER;

	Omega(){ LOG_DEBUG("Constructing Omega."); }
	~Omega(){}

	FRIEND_SINGLETON(Omega);
	friend class pyOmega;
};


