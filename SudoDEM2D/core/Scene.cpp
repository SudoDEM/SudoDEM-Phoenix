/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include"Scene.hpp"
#include<sudodem/core/Engine.hpp>
#include<sudodem/core/Timing.hpp>
#include<sudodem/core/TimeStepper.hpp>

#include<sudodem/lib/base/Math.hpp>
#include<chrono>
#include<ctime>
#include<sstream>
#include<iomanip>

#include<sudodem/core/BodyContainer.hpp>
#include<sudodem/core/InteractionContainer.hpp>


// POSIX-only
#include<pwd.h>
#include<unistd.h>
#include<time.h>

CREATE_LOGGER(Scene);
// should be elsewhere, probably
bool TimingInfo::enabled=false;

void Scene::fillDefaultTags(){
	// fill default tags
	struct passwd* pw;
	char hostname[HOST_NAME_MAX];
	gethostname(hostname,HOST_NAME_MAX);
	pw=getpwuid(geteuid()); if(!pw) throw runtime_error("getpwuid(geteuid()) failed!");
	// a few default tags
	// real name: will have all non-ASCII characters replaced by ? since serialization doesn't handle that
	// the standard GECOS format is Real Name,,, - first comma and after will be discarded
	string gecos(pw->pw_gecos), gecos2; size_t p=gecos.find(","); if(p!=string::npos) gecos = gecos.substr(0, p); for(size_t i=0;i<gecos.size();i++){gecos2.push_back(((unsigned char)gecos[i])<128 ? gecos[i] : '?'); }
	
	// Replace spaces with ~ in author string
	string authorStr = "author=" + gecos2 + " (" + string(pw->pw_name) + "@" + hostname + ")";
	size_t pos = 0;
	while((pos = authorStr.find(" ", pos)) != string::npos) {
		authorStr.replace(pos, 1, "~");
		pos++;
	}
	tags.push_back(authorStr);
	
	// Get current time in ISO format
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm_now = *std::localtime(&now_time_t);
	std::ostringstream oss;
	oss << std::put_time(&tm_now, "%Y%m%dT%H%M%S");
	string isoTime = oss.str();
	tags.push_back("isoTime=" + isoTime);
	string id = isoTime + "p" + std::to_string(getpid());
	tags.push_back("id="+id);
	tags.push_back("d.id="+id);
	tags.push_back("id.d="+id);
}



void Scene::postLoad(Scene&){
	// update the interaction container; must be done in Scene ctor as well; important!
	interactions->postLoad__calledFromScene(bodies);

	// this might be removed at some point, since it is checked by regression tests now
	for(const shared_ptr<Body>& b : *bodies){
		if(!b || !b->material || b->material->id<0) continue; // not a shared material
		if(b->material!=materials[b->material->id]) throw std::logic_error("Scene::postLoad: Internal inconsistency, shared materials not preserved when loaded; please report bug.");
	}
}



void Scene::moveToNextTimeStep(){
	if(runInternalConsistencyChecks){
		runInternalConsistencyChecks=false;
		checkStateTypes();
	}
	// substepping or not, update engines from _nextEngines, if defined, at the beginning of step
	// subStep can be 0, which happens if simulations is saved in the middle of step (without substepping)
	// this assumes that prologue will not set _nextEngines, which is safe hopefully
	if(!_nextEngines.empty() && (subStep<0 || (subStep<=0 && !subStepping))){
		engines=_nextEngines;
		_nextEngines.clear();
		// hopefully this will not break in some margin cases (subStepping with setting _nextEngines and such)
		subStep=-1;
	}
	if(!subStepping && subStep<0){
		/* set substep to 0 during the loop, so that engines/nextEngines handler know whether we are inside the loop currently */
		subStep=0;
		// ** 1. ** prologue
		if(isPeriodic) cell->integrateAndUpdate(dt);
		//forces.reset(); // uncomment if ForceResetter is removed
		const bool TimingInfo_enabled=TimingInfo::enabled; // cache the value, so that when it is changed inside the step, the engine that was just running doesn't get bogus values
		TimingInfo::delta last=TimingInfo::getNow(); // actually does something only if TimingInfo::enabled, no need to put the condition here
		// ** 2. ** engines
		for(const shared_ptr<Engine>& e : engines){
			e->scene=this;
			if(e->dead || !e->isActivated()) continue;
			e->action();
			if(TimingInfo_enabled) {TimingInfo::delta now=TimingInfo::getNow(); e->timingInfo.nsec+=now-last; e->timingInfo.nExec+=1; last=now;}
		}
		// ** 3. ** epilogue
				// Calculation speed
		if (iter==0) {				//For the first time
			prevTime = std::chrono::system_clock::now();
		} else {
			auto timeNow = std::chrono::system_clock::now();
			auto dif = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - prevTime).count();
			SpeedElements(iter%nSpeedIter,0)=1000000.0 / dif;

			speed = SpeedElements.mean();

			prevTime = timeNow;
		}

		iter++;
		time+=dt;
		subStep=-1;
	} else {
		/* IMPORTANT: take care to copy EXACTLY the same sequence as is in the block above !! */
		if(TimingInfo::enabled){ TimingInfo::enabled=false; LOG_INFO("O.timingEnabled disabled, since O.subStepping is used."); }
		if(subStep<-1 || subStep>(int)engines.size()){ LOG_ERROR("Invalid value of Scene::subStep ("<<subStep<<"), setting to -1 (prologue will be run)."); subStep=-1; }
		// if subStepping is disabled, it means we have not yet finished last step completely; in that case, do that here by running all remaining substeps at once
		// if subStepping is enabled, just run the step we need (the loop is traversed only once, with subs==subStep)
		int maxSubStep=subStep;
		if(!subStepping){ maxSubStep=engines.size(); LOG_INFO("Running remaining sub-steps ("<<subStep<<"…"<<maxSubStep<<") before disabling sub-stepping."); }
		for(int subs=subStep; subs<=maxSubStep; subs++){
			assert(subs>=-1 && subs<=(int)engines.size());
			// ** 1. ** prologue
			if(subs==-1){ if(isPeriodic) cell->integrateAndUpdate(dt); }
			// ** 2. ** engines
			else if(subs>=0 && subs<(int)engines.size()){ const shared_ptr<Engine>& e(engines[subs]); e->scene=this; if(!e->dead && e->isActivated()) e->action(); }
			// ** 3. ** epilogue
			else if(subs==(int)engines.size()){ iter++; time+=dt; /* gives -1 along with the increment afterwards */ subStep=-2; }
			// (?!)
			else { /* never reached */ assert(false); }
		}
		subStep++; // if not substepping, this will make subStep=-2+1=-1, which is what we want
	}
}



shared_ptr<Engine> Scene::engineByName(const string& s){
	for(const shared_ptr<Engine> e : engines){
		if(e->getClassName()==s) return e;
	}
	return shared_ptr<Engine>();
}

bool Scene::timeStepperPresent(){
	int n=0;
	for(const shared_ptr<Engine>&e : engines){ if(dynamic_cast<TimeStepper*>(e.get())) n++; }
	if(n>1) throw std::runtime_error(string("Multiple ("+std::to_string(n)+") TimeSteppers in the simulation?!").c_str());
	return n>0;
}

bool Scene::timeStepperActive(){
	int n=0; bool ret=false;
	for(const shared_ptr<Engine>&e : engines){
		TimeStepper* ts=dynamic_cast<TimeStepper*>(e.get()); if(ts) { ret=ts->active; n++; }
	}
	if(n>1) throw std::runtime_error(string("Multiple ("+std::to_string(n)+") TimeSteppers in the simulation?!").c_str());
	return ret;
}

bool Scene::timeStepperActivate(bool a){
	int n=0;
	for(const shared_ptr<Engine> e : engines){
		TimeStepper* ts=dynamic_cast<TimeStepper*>(e.get());
		if(ts) { ts->setActive(a); n++; }
	}
	if(n>1) throw std::runtime_error(string("Multiple ("+std::to_string(n)+") TimeSteppers in the simulation?!").c_str());
	return n>0;
}



void Scene::checkStateTypes(){
	for(const shared_ptr<Body>& b : *bodies){
		if(!b || !b->material) continue;
		if(b->material && !b->state) throw std::runtime_error("Body #"+std::to_string(b->getId())+": has Body::material, but NULL Body::state.");
		if(!b->material->stateTypeOk(b->state.get())){
			throw std::runtime_error("Body #"+std::to_string(b->getId())+": Body::material type "+b->material->getClassName()+" doesn't correspond to Body::state type "+b->state->getClassName()+" (should be "+b->material->newAssocState()->getClassName()+" instead).");
		}
	}
}

void Scene::updateBound(){
	if(!bound) bound=shared_ptr<Bound>(new Bound);
	const Real& inf=std::numeric_limits<Real>::infinity();
	Vector2r mx(-inf,-inf);
	Vector2r mn(inf,inf);
	for(const shared_ptr<Body>& b : *bodies){
		if(!b) continue;
		if(b->bound){
			for(int i=0; i<2; i++){
				if(!std::isinf(b->bound->max[i])) mx[i]=max(mx[i],b->bound->max[i]);
				if(!std::isinf(b->bound->min[i])) mn[i]=min(mn[i],b->bound->min[i]);
			}
		} else {
 			mx=mx.cwiseMax(b->state->pos);
 			mn=mn.cwiseMin(b->state->pos);
		}
	}
	bound->min=mn;
	bound->max=mx;
}

void Scene::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Scene");
	pybind11::class_<Scene, Serializable, std::shared_ptr<Scene>> _classObj(_module, "Scene", "Object comprising the whole simulation.");
	_classObj.def(pybind11::init<>([](){
		auto s = std::make_shared<Scene>();
		s->fillDefaultTags();
		s->interactions->postLoad__calledFromScene(s->bodies);
		s->SpeedElements.Zero();
		return s;
	}));
	_classObj.def_readwrite("dt", &Scene::dt, "Current timestep for integration.");
	_classObj.def_readonly("iter", &Scene::iter, "Current iteration (computational step) number");
	_classObj.def_readwrite("subStepping", &Scene::subStepping, "Whether we currently advance by one engine in every step (rather than by single run through all engines).");
	_classObj.def_readonly("subStep", &Scene::subStep, "Number of sub-step; not to be changed directly. -1 means to run loop prologue (cell integration), 0…n-1 runs respective engines (n is number of engines), n runs epilogue (increment step number and time.");
	_classObj.def_readonly("time", &Scene::time, "Simulation time (virtual time) [s]");
	_classObj.def_readonly("speed", &Scene::speed, "Current calculation speed [iter/s]");
	_classObj.def_readwrite("stopAtIter", &Scene::stopAtIter, "Iteration after which to stop the simulation.");
	_classObj.def_readwrite("stopAtTime", &Scene::stopAtTime, "Time after which to stop the simulation");
	_classObj.def_readonly("isPeriodic", &Scene::isPeriodic, "Whether periodic boundary conditions are active.");
	_classObj.def_readonly("trackEnergy", &Scene::trackEnergy, "Whether energies are being traced.");
	_classObj.def_readonly("doSort", &Scene::doSort, "Used, when new body is added to the scene.");
	_classObj.def_readwrite("runInternalConsistencyChecks", &Scene::runInternalConsistencyChecks, "Run internal consistency check, right before the very first simulation step.");
	_classObj.def_readwrite("selectedBody", &Scene::selectedBody, "Id of body that is selected by the user");
	_classObj.def_readonly("flags", &Scene::flags, "Various flags of the scene; 1 (Scene::LOCAL_COORDS): use local coordinate system rather than global one for per-interaction quantities (set automatically from the functor).");
	_classObj.def_readwrite("tags", &Scene::tags, "Arbitrary key=value associations (tags like mp3 tags: author, date, version, description etc.)");
	_classObj.def_readwrite("engines", &Scene::engines, "Engines sequence in the simulation.");
	_classObj.def_readwrite("_nextEngines", &Scene::_nextEngines, "Engines to be used from the next step on; is returned transparently by O.engines if in the middle of the loop (controlled by subStep>=0).");
	_classObj.def_readwrite("bodies", &Scene::bodies, "Bodies contained in the scene.");
	_classObj.def_readwrite("interactions", &Scene::interactions, "All interactions between bodies.");
	_classObj.def_readwrite("energy", &Scene::energy, "Energy values, if energy tracking is enabled.");
	_classObj.def_readwrite("materials", &Scene::materials, "Container of shared materials. Add elements using Scene::addMaterial, not directly. Do NOT remove elements from here unless you know what you are doing!");
	_classObj.def_readwrite("bound", &Scene::bound, "Bounding box of the scene (only used for rendering and initialized if needed).");
	_classObj.def_readwrite("cell", &Scene::cell, "Information on periodicity; only should be used if Scene::isPeriodic.");
	_classObj.def_readwrite("miscParams", &Scene::miscParams, "Store for arbitrary Serializable objects; will set static parameters during deserialization (primarily for GLDraw functors which otherwise have no attribute access)");
	_classObj.def_readwrite("dispParams", &Scene::dispParams, "'hash maps' of display parameters (since sudodem::serialization had no support for maps, emulate it via vector of strings in format key=value)");
	_classObj.def_property("localCoords", &Scene::usesLocalCoords, &Scene::setLocalCoords, "Whether local coordianate system is used on interactions (set by :yref:`IGeomFunctor`).");
	_classObj.def_property("compressionNegative", &Scene::compressionNegative, &Scene::setCompressionNegative, "Whether the convention is that compression has negative sign (set by :yref:`IGeomFunctor`).");
	_classObj.def("addMaterial", &Scene::addMaterial, "Adds material to Scene::materials. It also sets id of the material accordingly and returns it.");
	_classObj.def("checkStateTypes", &Scene::checkStateTypes, "Checks that type of Body::state satisfies Material::stateTypeOk. Throws runtime_error if not. (Is called from BoundDispatcher the first time it runs)");
	_classObj.def("updateBound", &Scene::updateBound, "update our bound; used directly instead of a BoundFunctor, since we don't derive from Body anymore");
	_classObj.def("fillDefaultTags", &Scene::fillDefaultTags, "initialize tags (author, date, time)");
	_classObj.def("moveToNextTimeStep", &Scene::moveToNextTimeStep, "advance by one iteration by running all engines");
	_classObj.def("timeStepperPresent", &Scene::timeStepperPresent, "return whether a TimeStepper is present");
	_classObj.def("timeStepperActive", &Scene::timeStepperActive, "true if TimeStepper is present and active, false otherwise");
	_classObj.def("timeStepperActivate", &Scene::timeStepperActivate, "activate/deactivate TimeStepper; returns whether the operation was successful (i.e. whether a TimeStepper was found)");
	_classObj.def("engineByName", &Scene::engineByName, "get engine by name");
}