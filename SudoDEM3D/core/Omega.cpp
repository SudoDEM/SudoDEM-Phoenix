/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include <sudodem/core/Omega.hpp>
#include <sudodem/core/Scene.hpp>
#include <sudodem/core/TimeStepper.hpp>
#include <sudodem/core/ThreadRunner.hpp>
#include <sudodem/lib/base/Math.hpp>
#include <sudodem/lib/multimethods/FunctorWrapper.hpp>
#include <sudodem/lib/multimethods/Indexable.hpp>
#include <sudodem/lib/serialization/ObjectIO.hpp>


#ifdef _WIN32
	#include <filesystem>
#elif defined(__APPLE__) || defined(__linux__)
    #include <cxxabi.h>
    #include <dlfcn.h>

#endif


class RenderMutexLock {
	private:
		std::lock_guard<std::timed_mutex> lock;
	public:
		RenderMutexLock(): lock(Omega::instance().renderMutex){/* cerr<<"Lock renderMutex"<<endl;*/}
		~RenderMutexLock(){/* cerr<<"Unlock renderMutex"<<endl;*/ }
};

CREATE_LOGGER(Omega);
SINGLETON_SELF(Omega);

const map<string,DynlibDescriptor>& Omega::getDynlibsDescriptor(){return dynlibs;}

const shared_ptr<Scene>& Omega::getScene(){
    if(scenes.empty() || currentSceneNb < 0 || currentSceneNb >= (int)scenes.size()) {
        static shared_ptr<Scene> empty;
        return empty;
    }
    return scenes[currentSceneNb];
}
void Omega::resetCurrentScene(){ RenderMutexLock lock; scenes.at(currentSceneNb) = shared_ptr<Scene>(new Scene); scenes.at(currentSceneNb)->interactions->postLoad__calledFromScene(scenes.at(currentSceneNb)->bodies);}
void Omega::resetScene(){ resetCurrentScene(); }//RenderMutexLock lock; scene = shared_ptr<Scene>(new Scene);}
void Omega::resetAllScenes(){
	RenderMutexLock lock;
	scenes.resize(1);
	scenes[0] = shared_ptr<Scene>(new Scene);
	scenes[0]->interactions->postLoad__calledFromScene(scenes[0]->bodies);
	currentSceneNb=0;
}
int Omega::addScene(){
	scenes.push_back(shared_ptr<Scene>(new Scene));
	scenes.back()->interactions->postLoad__calledFromScene(scenes.back()->bodies);
	return scenes.size()-1;
}
void Omega::switchToScene(int i) {
	if (i<0 || i>=int(scenes.size())) {
		LOG_ERROR("Scene "<<i<<" has not been created yet, no switch.");
		return;
	}
	currentSceneNb=i;
}



Real Omega::getRealTime(){ return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startupLocalTime).count() / 1e3; }
std::chrono::milliseconds Omega::getRealTime_duration(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startupLocalTime);}


void Omega::initTemps(){
	
	#ifdef _WIN32
		std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / ("sudodem-" + std::to_string(std::rand()));
		std::filesystem::create_directory(tmpPath);
		tmpFileDir = tmpPath.string();
	#elif defined(__APPLE__) || defined(__linux__)
		char dirTemplate[] = "/tmp/sudodem-XXXXXX";
		tmpFileDir = mkdtemp(dirTemplate);
	#endif

	tmpFileCounter = 0;

}

void Omega::cleanupTemps(){
  std::filesystem::path tmpPath(tmpFileDir);
  std::filesystem::remove_all(tmpPath);
}

std::string Omega::tmpFilename(){
	if(tmpFileDir.empty()) throw runtime_error("tmpFileDir empty; Omega::initTemps not yet called()?");
	std::lock_guard<std::mutex> lock(tmpFileCounterMutex);
	return tmpFileDir+"/tmp-"+std::to_string(tmpFileCounter++);
}

void Omega::reset(){
	stop();
	init();
}

void Omega::init(){
	sceneFile="";
	//resetScene();
	resetAllScenes();
	sceneAnother=shared_ptr<Scene>(new Scene);
	sceneAnother->interactions->postLoad__calledFromScene(sceneAnother->bodies);
	timeInit();
	createSimulationLoop();
}

void Omega::timeInit(){
	startupLocalTime=std::chrono::system_clock::now();
}

void Omega::createSimulationLoop(){	simulationLoop=shared_ptr<ThreadRunner>(new ThreadRunner(&simulationFlow_));}
void Omega::stop(){ LOG_DEBUG("");  if (simulationLoop&&simulationLoop->looping())simulationLoop->stop(); if (simulationLoop) simulationLoop=shared_ptr<ThreadRunner>(); }

/* WARNING: even a single simulation step is run asynchronously; the call will return before the iteration is finished. */
void Omega::step(){
	if (simulationLoop){
		simulationLoop->spawnSingleAction();
	}
}

void Omega::run(){
	if(!simulationLoop){ LOG_ERROR("No Omega::simulationLoop? Creating one (please report bug)."); createSimulationLoop(); }
	if (simulationLoop && !simulationLoop->looping()){
		simulationLoop->start();
	}
}


void Omega::pause(){
	if (simulationLoop && simulationLoop->looping()){
		simulationLoop->stop();
	}
}

bool Omega::isRunning(){ if(simulationLoop) return simulationLoop->looping(); else return false; }

void Omega::buildDynlibDatabase(const vector<string>& dynlibsList){
	LOG_DEBUG("called with "<<dynlibsList.size()<<" plugins.");
	std::cerr << "DEBUG buildDynlibDatabase: called with " << dynlibsList.size() << " plugins" << std::endl;
	for(const auto& name : dynlibsList) {
		if (name.find("Gl") != string::npos) {
			std::cerr << "DEBUG buildDynlibDatabase: GL class: " << name << std::endl;
		}
	}
	pybind11::module_ wrapperScope=pybind11::module_::import("sudodem.wrapper");
	std::list<string> pythonables;
	
	// Set of core classes that are already registered in sudodemWrapper.cpp
	// These should NOT be re-registered here to avoid conflicts
	std::set<std::string> coreClasses = {
		"Serializable", "Engine", "Functor", "Dispatcher",
		"Bound", "Shape", "State", "Material"
	};
	
	for(const string& name : dynlibsList){
		shared_ptr<Factorable> f;
		try {
			LOG_DEBUG("Factoring plugin "<<name);
			// Use ClassRegistry to create instances
			f = ClassRegistry::instance().create(name);
			if (!f) {
				LOG_DEBUG("Failed to create " << name << " via ClassRegistry, trying fallback...");
				continue;
			}
			dynlibs[name].isSerializable = ((SUDODEM_PTR_DYN_CAST<Serializable>(f)).get()!=0);
			
			// Get base class information from ClassRegistry
			string baseClassName = ClassRegistry::instance().getBaseClassName(name);
			if (!baseClassName.empty()) {
				dynlibs[name].baseClasses.insert(baseClassName);
			}
			if (name.find("Gl1_Superellipse") != string::npos || name.find("GlShapeFunctor") != string::npos) {
				std::cerr << "DEBUG buildDynlibDatabase: " << name << " baseClass=" << baseClassName << std::endl;
			}
			
			// Also get base classes from the object itself (for backward compatibility)
			for(int i=0;i<f->getBaseClassNumber();i++){
				dynlibs[name].baseClasses.insert(f->getBaseClassName(i));
			}
			
			// Skip core classes that are already registered in sudodemWrapper.cpp
			if(coreClasses.find(name) != coreClasses.end()) {
				LOG_DEBUG("Skipping core class "<<name<<" (already registered in sudodemWrapper.cpp)");
				continue;
			}
			if(dynlibs[name].isSerializable) {
				LOG_DEBUG("Adding "<<name<<" to pythonables list, isSerializable=true");
				pythonables.push_back(name);
			} else {
				LOG_DEBUG("Not adding "<<name<<" to pythonables list, isSerializable=false");
			}
		}
		catch (std::runtime_error& e){
			LOG_DEBUG("Error factoring plugin: "<<e.what());
			/* FIXME: this catches all errors! Some of them are not harmful, however:
			 * when a class is not factorable, it is OK to skip it; */
		}
	}	
	// Register Python classes in proper order (base classes before derived classes)
	// With the new system, classes are auto-registered, so we just need to verify
	// that all classes are properly registered
	for(int i=0; i<100 && pythonables.size()>0; i++){
		if(getenv("SUDODEM_DEBUG")) cerr<<endl<<"[[[ Round "<<i<<" ]]]: ";
		std::list<string> done;
		for(std::list<string>::iterator I=pythonables.begin(); I!=pythonables.end(); ){
			// Use ClassRegistry to create instances
			shared_ptr<Factorable> obj = ClassRegistry::instance().create(*I);
			shared_ptr<Serializable> s = std::static_pointer_cast<Serializable>(obj);
			try{
				if(getenv("SUDODEM_DEBUG")) cerr<<"{{"<<*I<<"}}";
				// With the new system, classes are already registered via ClassRegistry::registerAll()
				// We just verify they exist in the Python module
				try {
					wrapperScope.attr((*I).c_str());
					std::list<string>::iterator prev=I++;
					pythonables.erase(prev);
				} catch (pybind11::error_already_set&) {
					if(getenv("SUDODEM_DEBUG")){ cerr<<"["<<*I<<": not found in Python module]"; }
					I++;
				}
			} catch (pybind11::error_already_set& e){
				if(getenv("SUDODEM_DEBUG")){ cerr<<"["<<*I<<": "<<e.what()<<"]"; }
				I++;
			} catch (std::exception& e){
				if(getenv("SUDODEM_DEBUG")){ cerr<<"["<<*I<<": std::exception: "<<e.what()<<"]"; }
				I++;
			} catch (...){
				if(getenv("SUDODEM_DEBUG")){ cerr<<"["<<*I<<": unknown exception]"; PyErr_Print(); }
				I++;
			}
		}
	}

	// Build base class hierarchy using information from ClassRegistry
	map<string,DynlibDescriptor>::iterator dli    = dynlibs.begin();
	map<string,DynlibDescriptor>::iterator dliEnd = dynlibs.end();
	for( ; dli!=dliEnd ; ++dli){
		set<string>::iterator bci    = (*dli).second.baseClasses.begin();
		set<string>::iterator bciEnd = (*dli).second.baseClasses.end();
		for( ; bci!=bciEnd ; ++bci){
			string name = *bci;
			if (name=="Dispatcher1D" || name=="Dispatcher2D") (*dli).second.baseClasses.insert("Dispatcher");
			else if (name=="Functor1D" || name=="Functor2D") (*dli).second.baseClasses.insert("Functor");
			else if (name=="Serializable") (*dli).second.baseClasses.insert("Factorable");
			else if (name!="Factorable" && name!="Indexable" && name!="FunctorWrapper" && name!="DynLibDispatcher") {
				// Use ClassRegistry to create instances
				shared_ptr<Factorable> f = ClassRegistry::instance().create(name);
				if (f) {
					for(int i=0;i<f->getBaseClassNumber();i++)
						dynlibs[name].baseClasses.insert(f->getBaseClassName(i));
				}
			}
		}
	}
}


bool Omega::isInheritingFrom(const string& className, const string& baseClassName){
	return (dynlibs[className].baseClasses.find(baseClassName)!=dynlibs[className].baseClasses.end());
}

bool Omega::isInheritingFrom_recursive(const string& className, const string& baseClassName){
	if (dynlibs[className].baseClasses.find(baseClassName)!=dynlibs[className].baseClasses.end()) return true;
	for(const string& parent : dynlibs[className].baseClasses){
		if(isInheritingFrom_recursive(parent,baseClassName)) return true;
	}
	return false;
}

void Omega::loadPlugins(vector<string> pluginFiles){
	// With static linking, all classes are already registered via static initialization
	// when the shared library is loaded. No dynamic library loading is needed.
	// The pluginFiles parameter is kept for backward compatibility but is ignored.
	
	LOG_DEBUG("Using static linking - all classes registered via static initialization");
	
	// Get all registered classes from ClassRegistry
	vector<string> registeredClasses = ClassRegistry::instance().getRegisteredClasses();

	// Sort and unique the list
	list<string> pluginsList(registeredClasses.begin(), registeredClasses.end());
	pluginsList.sort();
	pluginsList.unique();

	// Build class hierarchy database using the new registry
	buildDynlibDatabase(vector<string>(pluginsList.begin(), pluginsList.end()));

	LOG_INFO("Loaded " << pluginsList.size() << " classes from " << pluginFiles.size() << " plugin(s)");
	std::cerr << "DEBUG loadPlugins: " << pluginsList.size() << " classes registered" << std::endl;
	for(const auto& name : pluginsList) {
		if (name.find("Gl") != string::npos || name.find("Shape") != string::npos) {
			std::cerr << "DEBUG loadPlugins: class " << name << std::endl;
		}
	}
}

void Omega::loadSimulation(const string& f, bool quiet){
	bool isMem=(f.size() >= 8 && f.substr(0, 8) == ":memory:");
	if(!isMem && !std::filesystem::exists(f)) throw runtime_error("Simulation file to load doesn't exist: "+f);
	if(isMem && memSavedSimulations.count(f)==0) throw runtime_error("Cannot load nonexistent memory-saved simulation "+f);

	if(!quiet) LOG_INFO("Loading file "+f);
	//shared_ptr<Scene> scene = getScene();
	shared_ptr<Scene>& scene = scenes[currentSceneNb];
	//shared_ptr<Scene>& scene = getScene();
	{
		stop(); // stop current simulation if running
		resetScene();
		RenderMutexLock lock;
		if(isMem){
			istringstream iss(memSavedSimulations[f]);
			sudodem::ObjectIO::load<decltype(scene),cereal::BinaryInputArchive>(iss,"scene",scene);
		} else {
			sudodem::ObjectIO::load(f,"scene",scene);
		}
	}
	// Debug output
	std::cerr << "DEBUG loadSimulation: scene ptr=" << scene.get() << std::endl;
	if(scene) {
		std::cerr << "DEBUG loadSimulation: scene->getClassName()=" << scene->getClassName() << std::endl;
	}
	if(!scene || scene->getClassName()!="Scene") throw logic_error("Wrong file format (scene is not a Scene!?) in "+f);
	sceneFile=f;
	timeInit();
	if(!quiet) LOG_DEBUG("Simulation loaded");
}



void Omega::saveSimulation(const string& f, bool quiet){
	if(f.size()==0) throw runtime_error("f of file to save has zero length.");
	if(!quiet) LOG_INFO("Saving file " << f);
	//shared_ptr<Scene> scene = getScene();
	shared_ptr<Scene>& scene = scenes[currentSceneNb];
	//shared_ptr<Scene>& scene = getScene();
	if(f.size() >= 8 && f.substr(0, 8) == ":memory:"){
		if(memSavedSimulations.count(f)>0 && !quiet) LOG_INFO("Overwriting in-memory saved simulation "<<f);
		ostringstream oss;
		sudodem::ObjectIO::save<decltype(scene),cereal::BinaryOutputArchive>(oss,"scene",scene);
		memSavedSimulations[f]=oss.str();
	}
	else {
		// handles automatically the XML/binary distinction as well as gz/bz2 compression
		sudodem::ObjectIO::save(f,"scene",scene);
	}
	sceneFile=f;
}




