/*************************************************************************
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include<sudodem/pkg/common/GravityEngines.hpp>
#include<sudodem/pkg/common/PeriodicEngines.hpp>
#include<sudodem/core/BodyContainer.hpp>
#include<sudodem/core/Scene.hpp>

// pyRegisterClass implementations moved from GravityEngines.hpp
void GravityEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<GravityEngine, FieldApplier, std::shared_ptr<GravityEngine>> _classObj(_module, "GravityEngine", "Engine applying constant acceleration to all bodies. DEPRECATED, use :yref:`Newton::gravity` unless you need energy tracking or selective gravity application using groupMask).");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("gravity", &GravityEngine::gravity, "Acceleration [kgms⁻²]");
	_classObj.def_readwrite("gravPotIx", &GravityEngine::gravPotIx, "Index for gravPot energy");
	_classObj.def_readwrite("mask", &GravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
	_classObj.def_readwrite("warnOnce", &GravityEngine::warnOnce, "For deprecation warning once.");
	_classObj.def("action", &GravityEngine::action);
}

void CentralGravityEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<CentralGravityEngine, FieldApplier, std::shared_ptr<CentralGravityEngine>> _classObj(_module, "CentralGravityEngine", "Engine applying acceleration to all bodies, towards a central body.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("centralBody", &CentralGravityEngine::centralBody, "The :yref:`body<Body>` towards which all other bodies are attracted.");
	_classObj.def_readwrite("accel", &CentralGravityEngine::accel, "Acceleration magnitude [kgms⁻²]");
	_classObj.def_readwrite("reciprocal", &CentralGravityEngine::reciprocal, "If true, acceleration will be applied on the central body as well.");
	_classObj.def_readwrite("mask", &CentralGravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
	_classObj.def("action", &CentralGravityEngine::action);
}

void AxialGravityEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<AxialGravityEngine, FieldApplier, std::shared_ptr<AxialGravityEngine>> _classObj(_module, "AxialGravityEngine", "Apply acceleration (independent of distance) directed towards an axis.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("axisPoint", &AxialGravityEngine::axisPoint, "Point through which the axis is passing.");
	_classObj.def_readwrite("axisDirection", &AxialGravityEngine::axisDirection, "direction of the gravity axis (will be normalized automatically)");
	_classObj.def_readwrite("acceleration", &AxialGravityEngine::acceleration, "Acceleration magnitude [kgms⁻²]");
	_classObj.def_readwrite("mask", &AxialGravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
	_classObj.def("action", &AxialGravityEngine::action);
}

CREATE_LOGGER(GravityEngine);

void GravityEngine::action(){
	if (warnOnce) {warnOnce=false; LOG_ERROR("GravityEngine is deprecated, consider using Newton::gravity instead (unless gravitational energy has to be tracked - not implemented with the newton attribute).")}
	const bool trackEnergy(scene->trackEnergy);
	const Real dt(scene->dt);
	for(const shared_ptr<Body>& b : *scene->bodies){
		// skip clumps, only apply forces on their constituents
		if(b->isClump()) continue;
		if(mask!=0 && !b->maskCompatible(mask)) continue;
		scene->forces.addForce(b->getId(),gravity*b->state->mass);
		// work done by gravity is "negative", since the energy appears in the system from outside
		if(trackEnergy) scene->energy->add(-gravity.dot(b->state->vel)*b->state->mass*dt,"gravWork",fieldWorkIx,/*non-incremental*/false);
	}
}

void CentralGravityEngine::action(){
	const Vector3r& centralPos=Body::byId(centralBody)->state->pos;
	for(const shared_ptr<Body>& b : *scene->bodies){
		if(b->isClump() || b->getId()==centralBody) continue; // skip clumps and central body
		if(mask!=0 && !b->maskCompatible(mask)) continue;
		Real F=accel*b->state->mass;
		Vector3r toCenter=centralPos-b->state->pos; toCenter.normalize();
		scene->forces.addForce(b->getId(),F*toCenter);
		if(reciprocal) scene->forces.addForce(centralBody,-F*toCenter);
	}
}

void AxialGravityEngine::action(){
	for(const shared_ptr<Body>&b : *scene->bodies){
		if(!b || b->isClump()) continue;
		if(mask!=0 && !b->maskCompatible(mask)) continue;
		/* http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html */
		const Vector3r& x0=b->state->pos;
		const Vector3r& x1=axisPoint;
		const Vector3r x2=axisPoint+axisDirection;
		Vector3r closestAxisPoint=x1+(x2-x1) * /* t */ (-(x1-x0).dot(x2-x1))/((x2-x1).squaredNorm());
		Vector3r toAxis=closestAxisPoint-x0; toAxis.normalize();
		if(toAxis.squaredNorm()==0) continue;
		scene->forces.addForce(b->getId(),acceleration*b->state->mass*toAxis);
	}
}

/*
Vector2i HdapsGravityEngine::readSysfsFile(const string& name){
	char buf[256];
	ifstream f(name.c_str());
	if(!f.is_open()) throw std::runtime_error(("HdapsGravityEngine: unable to open file "+name).c_str());
	f.read(buf,256);f.close();
	const std::regex re("\\(([0-9+-]+),([0-9+-]+)\\).*");
   std::cmatch matches;
	if(!std::regex_match(buf,matches,re)) throw std::runtime_error(("HdapsGravityEngine: error parsing data from "+name).c_str());
	//cerr<<matches[1]<<","<<matches[2]<<endl;
	return Vector2i(std::stoi(matches[1]),std::stoi(matches[2]));

}
*/
