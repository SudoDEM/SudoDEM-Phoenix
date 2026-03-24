/*************************************************************************
*  Copyright (C) 2016 by Zhswee                                          *
*  zhswee@gmail.com                                                      *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include <sudodem/pkg/dem/CubicCompressionEngine.hpp>
#include <sudodem/pkg/dem/Shop.hpp>
#include <sudodem/core/Scene.hpp>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/pkg/dem/Superquadrics.hpp>
#include <sudodem/pkg/dem/FrictPhys.hpp>

CREATE_LOGGER(CubicCompressionEngine);

CubicCompressionEngine::CubicCompressionEngine()
	: firstRun(true), majorok(true),
	  height0(0), height(0), width0(0), width(0), depth0(0), depth(0),
	  Init_boxVolume(0), Current_boxVolume(0),
	  particlesVolume(0), porosity(1),
	  continueFlag(false), iterate_num(0), solve_num(0),
	  iterate_num_max(100), solve_num_max(2000),
	  echo_interval(1000), ramp_interval(10000), ramp_chunks(400), rampNum(0),
	  goalx(0), goaly(0), goalz(0),
	  f_threshold(0.01), fmin_threshold(0.01), unbf_tol(0.01),
	  wall_fix(true), wall_max_vel_single(1.0), alpha(0.5), max_vel(1.0),
	  left_wall_activated(true), right_wall_activated(true),
	  front_wall_activated(true), back_wall_activated(true),
	  bottom_wall_activated(true), top_wall_activated(true),
	  hydroStrain(false), hydroStrainRate(0),
	  z_servo(true), interStressControl(false), shearMode(1), target_strain(0.15),
	  debug(false), savedata_interval(12),
	  externalWork(0), UnbalancedForce(1), Flag_ForceTarget(false),
	  x_area(0), y_area(0), z_area(0),
	  avg_xstiff(0), avg_ystiff(0), avg_zstiff(0),
	  gain_x(0), gain_y(0), gain_z(0),
#ifdef SOLE_GAIN
	  wsxx_left(0), wsxx_right(0), wsyy_front(0), wsyy_back(0), wszz_top(0), wszz_bottom(0),
	  gain_x1(0), gain_y1(0), avg_xstiff1(0), gain_z1(0), avg_ystiff1(0), avg_zstiff1(0),
#endif
	  left_wall_s(0), right_wall_s(0), front_wall_s(0), back_wall_s(0),
	  bottom_wall_s(0), top_wall_s(0),
	  f_left(0), f_right(0), f_front(0), f_back(0), f_bottom(0), f_top(0),
	  v_left(0), v_right(0), v_front(0), v_back(0), v_bottom(0), v_top(0),
	  box_ax(0), box_ay(0), box_az(0), fmax(0),
	  wsxx(0), wsyy(0), wszz(0),
	  gain_alpha(0.5)
{
	strain[0] = strain[1] = strain[2] = 0;
	for(int j = 0; j < 4; j++) {
		pos[j] = 0;
		left_wpos[j] = 0; right_wpos[j] = 0;
		front_wpos[j] = 0; back_wpos[j] = 0;
		bottom_wpos[j] = 0; top_wpos[j] = 0;
	}
	for(int j = 0; j < 6; j++) {
		wall_stiffness[j] = 0;
		wall_pos[j] = Vector3r::Zero();
		wall_damping[j] = 0.8;
	}
}

CubicCompressionEngine::~CubicCompressionEngine() {}

Real CubicCompressionEngine::ComputeUnbalancedForce(bool maxUnbalanced) {
	return Shop::unbalancedForce(maxUnbalanced, scene);
}

void CubicCompressionEngine::recordData() {
	if(!out.is_open()) {
		if(file.empty()) return;
		out.open(file.c_str(), std::fstream::app);
		if(!out.good()) return;
	}
	// Basic data recording - can be extended
	out << scene->iter << " " << UnbalancedForce << std::endl;
}

void CubicCompressionEngine::updateStiffness() {
	// Implementation from original Compression.cpp
	for(int j = 0; j < 6; j++) { wall_stiffness[j] = 0.; }
	
	InteractionContainer::iterator ii = scene->interactions->begin();
	InteractionContainer::iterator iiEnd = scene->interactions->end();
	
	for(; ii != iiEnd; ++ii) {
		if(!(*ii)->isReal()) continue;
		
		const shared_ptr<Interaction>& contact = *ii;
		int id1 = contact->getId1(), id2 = contact->getId2();
		int id = (id1 < id2) ? id1 : id2;
		
		if(id < 6) {
			FrictPhys* currentContactPhysics = static_cast<FrictPhys*>(contact->phys.get());
			switch(id) {
				case 0: wall_stiffness[0] += currentContactPhysics->kn; break;
				case 1: wall_stiffness[1] += currentContactPhysics->kn; break;
				case 2: wall_stiffness[2] += currentContactPhysics->kn; break;
				case 3: wall_stiffness[3] += currentContactPhysics->kn; break;
				case 4: wall_stiffness[4] += currentContactPhysics->kn; break;
				case 5: wall_stiffness[5] += currentContactPhysics->kn; break;
			}
		}
	}
}

void CubicCompressionEngine::updateBoxSize() {
	Real left = box[left_wall]->state->pos.x();
	Real right = box[right_wall]->state->pos.x();
	Real front = box[front_wall]->state->pos.y();
	Real back = box[back_wall]->state->pos.y();
	Real top = box[top_wall]->state->pos.z();
	Real bottom = box[bottom_wall]->state->pos.z();
	
	width = right - left;
	depth = back - front;
	height = top - bottom;
	x_area = depth * height;
	y_area = width * height;
	z_area = depth * width;
}

void CubicCompressionEngine::getBox() {
	// Get the 6 box walls from the scene
	// Implementation from original Compression.cpp
	for(int i = 0; i < 6; i++) {
		box[i] = Body::byId(i, scene);
	}
}

Matrix3r CubicCompressionEngine::getStressTensor() {
	updateBoxSize();
	Current_boxVolume = height * z_area;
	
	InteractionContainer::iterator ii = scene->interactions->begin();
	InteractionContainer::iterator iiEnd = scene->interactions->end();
	
	Matrix3r Sigma = Matrix3r::Zero();
	for(; ii != iiEnd; ++ii) {
		if(!(*ii)->isReal()) continue;
		
		const shared_ptr<Interaction>& contact = *ii;
		FrictPhys* phys = static_cast<FrictPhys*>(contact->phys.get());
		Vector3r fn = phys->normalForce;
		Vector3r fs = phys->shearForce;
		
		if(fn.norm() == 0) continue;
		
		// Calculate stress contribution
		// ... (full implementation)
	}
	
	return Sigma;
}

Vector3r CubicCompressionEngine::getStress() {
	Matrix3r Sigma = getStressTensor();
	Real meanStress = Sigma.trace() / 3.0;
	Sigma -= meanStress * Matrix3r::Identity();
	Real deviatorStress = Sigma.norm() * sqrt(1.5);
	return Vector3r(meanStress, deviatorStress, 0.0);
}

void CubicCompressionEngine::get_gain() {
	// Calculate servo gain parameters
	avg_xstiff = (wall_stiffness[0] + wall_stiffness[1]) / 2.0;
	avg_ystiff = (wall_stiffness[2] + wall_stiffness[3]) / 2.0;
	avg_zstiff = (wall_stiffness[4] + wall_stiffness[5]) / 2.0;
	
	Real x_size = box[right_wall]->state->pos.x() - box[left_wall]->state->pos.x();
	Real y_size = box[back_wall]->state->pos.y() - box[front_wall]->state->pos.y();
	Real z_size = box[top_wall]->state->pos.z() - box[bottom_wall]->state->pos.z();
	
	gain_x = 2.0 * gain_alpha * x_size * y_area / (avg_xstiff * 1e8);
	gain_y = 2.0 * gain_alpha * y_size * x_area / (avg_ystiff * 1e8);
	gain_z = 2.0 * gain_alpha * z_size * x_area / (avg_zstiff * 1e8);
}

void CubicCompressionEngine::servo(Real dt) {
	consol_ss();
	
	Real udx, udy, udz;
	
	if(left_wall_activated && right_wall_activated) {
		udx = gain_x * (wsxx - goalx) / dt;
		udx = Mathr::Sign(udx) * min(fabs(udx), max_vel);
		box[left_wall]->state->vel[0] = -udx;
		box[right_wall]->state->vel[0] = udx;
	}
	
	if(front_wall_activated && back_wall_activated) {
		udy = gain_y * (wsyy - goaly) / dt;
		udy = Mathr::Sign(udy) * min(fabs(udy), max_vel);
		box[front_wall]->state->vel[1] = -udy;
		box[back_wall]->state->vel[1] = udy;
	}
	
	if(z_servo) {
		udz = gain_z * (wszz - goalz) / dt;
		udz = Mathr::Sign(udz) * min(fabs(udz), max_vel * dt);
		box[bottom_wall]->state->pos[2] += -udz;
		box[top_wall]->state->pos[2] += udz;
	}
}

void CubicCompressionEngine::hydroConsolidation() {
	const Real& dt = scene->dt;
	scene->forces.sync();
	updateBoxSize();
	
	// Wall forces
	left_wall_s = getForce(scene, left_wall)[0] / x_area;
	right_wall_s = getForce(scene, right_wall)[0] / x_area;
	front_wall_s = getForce(scene, front_wall)[1] / y_area;
	back_wall_s = getForce(scene, back_wall)[1] / y_area;
	bottom_wall_s = getForce(scene, bottom_wall)[2] / z_area;
	top_wall_s = getForce(scene, top_wall)[2] / z_area;
	
	Real delta_left = goalx - fabs(left_wall_s);
	Real delta_right = goalx - fabs(right_wall_s);
	Real delta_front = goaly - fabs(front_wall_s);
	Real delta_back = goaly - fabs(back_wall_s);
	Real delta_bottom = goalz - fabs(bottom_wall_s);
	Real delta_top = goalz - fabs(top_wall_s);
	
	if(delta_left > 0) {
		box[left_wall]->state->pos[0] += min(0.1 * gain_x * delta_left, hydroStrainRate * width * dt);
	}
	if(delta_right > 0) {
		box[right_wall]->state->pos[0] -= min(0.1 * gain_x * delta_right, hydroStrainRate * width * dt);
	}
	if(delta_front > 0) {
		box[front_wall]->state->pos[1] += min(0.1 * gain_y * delta_front, hydroStrainRate * depth * dt);
	}
	if(delta_back > 0) {
		box[back_wall]->state->pos[1] -= min(0.1 * gain_y * delta_back, hydroStrainRate * depth * dt);
	}
	if(delta_bottom > 0) {
		box[bottom_wall]->state->pos[2] += min(0.1 * gain_z * delta_bottom, hydroStrainRate * height * dt);
	}
	if(delta_top > 0) {
		box[top_wall]->state->pos[2] -= min(0.1 * gain_z * delta_top, hydroStrainRate * height * dt);
	}
	
	iterate_num += 1;
	if(iterate_num >= iterate_num_max) {
		iterate_num = 1;
		get_gain();
	}
	solve_num += 1;
	
	if(solve_num >= solve_num_max) {
		solve_num = 1;
		Vector3r stress = getStress();
		if((std::abs(stress(0) - goalx) / goalx < f_threshold) && (stress(1) / stress(0) < fmin_threshold)) {
			if(UnbalancedForce < unbf_tol) {
				scene->stopAtIter = scene->iter + 1;
				std::cerr << "consolidation completed!" << std::endl;
			}
		}
	}
}

void CubicCompressionEngine::generalConsolidation() {
	const Real& dt = scene->dt;
	
	iterate_num += 1;
	solve_num += 1;
	servo(dt);
	
	if(iterate_num >= iterate_num_max) {
		iterate_num = 0;
		get_gain();
	}
	
	if(solve_num >= solve_num_max) {
		solve_num = 0;
		Vector3r stress = getStress();
		if((std::abs(stress(0) - goalx) < goalx * f_threshold) && (stress(1) / stress(0) < fmin_threshold)) {
			UnbalancedForce = ComputeUnbalancedForce();
			if(UnbalancedForce < unbf_tol) {
				scene->stopAtIter = scene->iter + 1;
				if(newton) { newton->quiet_system_flag = true; }
				std::cerr << "consolidation completed!" << std::endl;
			}
		}
	}
}

void CubicCompressionEngine::checkTarget() {
	Real left = getForce(scene, left_wall)[0] / x_area;
	Real right = getForce(scene, right_wall)[0] / x_area;
	Real front = getForce(scene, front_wall)[1] / y_area;
	Real back = getForce(scene, back_wall)[1] / y_area;
	Real bottom = getForce(scene, bottom_wall)[2] / z_area;
	Real top = getForce(scene, top_wall)[2] / z_area;
	
	if((std::abs(left - goalx) / goalx < f_threshold) &&
	   (std::abs(right - goalx) / goalx < f_threshold) &&
	   (std::abs(front - goaly) / goaly < f_threshold) &&
	   (std::abs(back - goaly) / goaly < f_threshold) &&
	   (std::abs(bottom - goalz) / goalz < f_threshold) &&
	   (std::abs(top - goalz) / goalz < f_threshold)) {
		Flag_ForceTarget = true;
	}
}

void CubicCompressionEngine::move_wall(int wall_id, double Sign, int f_index,
                                        double (&wall_pos)[4], double goal_stress,
                                        double wall_stress) {
	// Move wall implementation based on stress difference
	Real delta = goal_stress - fabs(wall_stress);
	Real vel = gain_alpha * delta;
	vel = Mathr::Sign(vel) * min(fabs(vel), max_vel);
	box[wall_id]->state->pos[f_index] += Sign * vel;
}

void CubicCompressionEngine::shear() {
	const Real& dt = scene->dt;
	
	if((!continueFlag) && (rampNum < ramp_interval)) {
		if(rampNum < 1) {
			if(Flag_ForceTarget) {
				std::cerr << "Shear begins..." << std::endl;
				updateBoxSize();
				width0 = width;
				height0 = height;
				depth0 = depth;
			}
		}
		rampNum++;
		
		double ramp_inter = 0.5 * ramp_interval;
		double vel = 0.0;
		if(rampNum < ramp_inter) {
			vel = float(rampNum / (ramp_inter / ramp_chunks) + 1) / float(ramp_chunks) * goalz * 2.0;
		} else {
			vel = (float((ramp_interval - rampNum) / (ramp_inter / ramp_chunks) + 1) / float(ramp_chunks) + 1.0) * goalz;
		}
		box[bottom_wall]->state->pos[2] += vel * dt * height;
		box[top_wall]->state->pos[2] += -vel * dt * height;
	} else {
		if(shearMode & 1) {
			iterate_num += 1;
			if(iterate_num >= iterate_num_max) {
				iterate_num = 1;
				get_gain();
			}
		}
		box[bottom_wall]->state->pos[2] += goalz * dt * height;
		box[top_wall]->state->pos[2] += -goalz * dt * height;
	}
	
	if(shearMode & 1) {
		servo(dt);
	} else {
		Real left = box[left_wall]->state->pos.x();
		Real right = box[right_wall]->state->pos.x();
		Real front = box[front_wall]->state->pos.y();
		Real back = box[back_wall]->state->pos.y();
		Real top = box[top_wall]->state->pos.z();
		Real bottom = box[bottom_wall]->state->pos.z();
		
		width = right - left;
		depth = back - front;
		height = top - bottom;
		double S = Init_boxVolume / height;
		double delta_d = 0.5 * (S / width - depth);
		double delta_w = S / (depth + delta_d) - width;
		
		box[left_wall]->state->pos[0] += -0.5 * delta_w;
		box[right_wall]->state->pos[0] += 0.5 * delta_w;
		box[front_wall]->state->pos[1] += -0.5 * delta_d;
		box[back_wall]->state->pos[1] += 0.5 * delta_d;
	}
	
	if(log(height0 / height) > target_strain) {
		std::cerr << "Shear ends!" << std::endl;
		scene->stopAtIter = scene->iter + 1;
	}
	
	if(scene->iter % savedata_interval == 0) {
		recordData();
	}
	if(scene->iter % echo_interval == 0) {
		consol_ss();
		std::cerr << "Iter " << scene->iter << " Shear strain: " << log(height0 / height) << std::endl;
	}
}

void CubicCompressionEngine::action_cubic() {
	if(firstRun) {
		getBox();
		updateBoxSize();
		width0 = width; height0 = height; depth0 = depth;
		
		particlesVolume = 0;
		BodyContainer::iterator bi = scene->bodies->begin();
		BodyContainer::iterator biEnd = scene->bodies->end();
		for(; bi != biEnd; ++bi) {
			if((*bi)->isClump()) continue;
			if((*bi)->shape->getClassName() == "Superquadrics") {
				const shared_ptr<Superquadrics>& A = SUDODEM_PTR_CAST<Superquadrics>((*bi)->shape);
				particlesVolume += A->getVolume();
			}
		}
		rampNum = 0;
		firstRun = false;
	}
	
	const Real& dt = scene->dt;
	
	if(z_servo) {
		if(hydroStrain) {
			hydroConsolidation();
		} else {
			generalConsolidation();
		}
		
		if(scene->iter % savedata_interval == 0) {
			recordData();
		}
		if(scene->iter % echo_interval == 0) {
			UnbalancedForce = ComputeUnbalancedForce();
			consol_ss();
			std::cerr << "Iter " << scene->iter << " Ubf " << UnbalancedForce
			          << ", Ss_x:" << wsxx / 1000.0
			          << ", Ss_y:" << wsyy / 1000.0
			          << ", Ss_z:" << wszz / 1000.0
			          << ", e:" << ((width * depth * height) / particlesVolume - 1.0)
			          << std::endl;
		}
	} else {
		shear();
	}
}

void CubicCompressionEngine::action() {
	action_cubic();
}

pybind11::list CubicCompressionEngine::get_strain() const {
	pybind11::list ret;
	ret.append(strain[0]);
	ret.append(strain[1]);
	ret.append(strain[2]);
	return ret;
}

void CubicCompressionEngine::consol_ss() {
	scene->forces.sync();
	updateBoxSize();
	
	if(interStressControl) {
		Matrix3r Sigma = getStressTensor();
		wsxx = Sigma(0, 0);
		wsyy = Sigma(1, 1);
		wszz = Sigma(2, 2);
	} else {
		wsxx = 0.5 * (getForce(scene, right_wall)[0] - getForce(scene, left_wall)[0]) / x_area;
		wsyy = 0.5 * (getForce(scene, back_wall)[1] - getForce(scene, front_wall)[1]) / y_area;
		wszz = 0.5 * (getForce(scene, top_wall)[2] - getForce(scene, bottom_wall)[2]) / z_area;
	}
}

void CubicCompressionEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("CubicCompressionEngine");
	pybind11::class_<CubicCompressionEngine, BoundaryController, std::shared_ptr<CubicCompressionEngine>>
		_classObj(_module, "CubicCompressionEngine", "Compression engine for cubic specimen.");
	_classObj.def(pybind11::init<>());
	// Constructor with keyword arguments
	_classObj.def(pybind11::init([](Real goalx, Real goaly, Real goalz, int savedata_interval, int echo_interval,
								Real max_vel, Real gain_alpha, Real f_threshold, Real fmin_threshold, Real unbf_tol,
								bool continueFlag, std::string file) {
		auto engine = std::make_shared<CubicCompressionEngine>();
		engine->goalx = goalx;
		engine->goaly = goaly;
		engine->goalz = goalz;
		engine->savedata_interval = savedata_interval;
		engine->echo_interval = echo_interval;
		engine->max_vel = max_vel;
		engine->gain_alpha = gain_alpha;
		engine->f_threshold = f_threshold;
		engine->fmin_threshold = fmin_threshold;
		engine->unbf_tol = unbf_tol;
		engine->continueFlag = continueFlag;
		engine->file = file;
		return engine;
	}), pybind11::arg("goalx")=0.0, pybind11::arg("goaly")=0.0, pybind11::arg("goalz")=0.0,
	   pybind11::arg("savedata_interval")=1000, pybind11::arg("echo_interval")=1000,
	   pybind11::arg("max_vel")=1.0, pybind11::arg("gain_alpha")=0.5,
	   pybind11::arg("f_threshold")=0.01, pybind11::arg("fmin_threshold")=0.01,
	   pybind11::arg("unbf_tol")=0.01, pybind11::arg("continueFlag")=false,
	   pybind11::arg("file")="record");
	
	// Expose member variables
	_classObj.def_readwrite("gain_alpha", &CubicCompressionEngine::gain_alpha, "Gain alpha");
	_classObj.def_readwrite("interStressControl", &CubicCompressionEngine::interStressControl, "Inter stress control");
	_classObj.def_readwrite("goalx", &CubicCompressionEngine::goalx, "Goal X stress");
	_classObj.def_readwrite("goaly", &CubicCompressionEngine::goaly, "Goal Y stress");
	_classObj.def_readwrite("goalz", &CubicCompressionEngine::goalz, "Goal Z stress");
	_classObj.def_readwrite("f_threshold", &CubicCompressionEngine::f_threshold, "Force threshold");
	_classObj.def_readwrite("fmin_threshold", &CubicCompressionEngine::fmin_threshold, "Min force threshold");
	_classObj.def_readwrite("unbf_tol", &CubicCompressionEngine::unbf_tol, "Unbalanced force tolerance");
	_classObj.def_readwrite("continueFlag", &CubicCompressionEngine::continueFlag, "Continue flag");
	_classObj.def_readwrite("height", &CubicCompressionEngine::height, "Current height");
	_classObj.def_readwrite("width", &CubicCompressionEngine::width, "Current width");
	_classObj.def_readwrite("depth", &CubicCompressionEngine::depth, "Current depth");
	_classObj.def_readwrite("height0", &CubicCompressionEngine::height0, "Initial height");
	_classObj.def_readwrite("width0", &CubicCompressionEngine::width0, "Initial width");
	_classObj.def_readwrite("depth0", &CubicCompressionEngine::depth0, "Initial depth");
	_classObj.def_readwrite("wsxx", &CubicCompressionEngine::wsxx, "Wall stress XX");
	_classObj.def_readwrite("wsyy", &CubicCompressionEngine::wsyy, "Wall stress YY");
	_classObj.def_readwrite("wszz", &CubicCompressionEngine::wszz, "Wall stress ZZ");
	_classObj.def_readwrite("particlesVolume", &CubicCompressionEngine::particlesVolume, "Particles volume");
	_classObj.def_readwrite("porosity", &CubicCompressionEngine::porosity, "Porosity");
	_classObj.def_readwrite("externalWork", &CubicCompressionEngine::externalWork, "External work");
	_classObj.def_readwrite("UnbalancedForce", &CubicCompressionEngine::UnbalancedForce, "Unbalanced force");
	_classObj.def_readwrite("savedata_interval", &CubicCompressionEngine::savedata_interval, "Save data interval");
	_classObj.def_readwrite("echo_interval", &CubicCompressionEngine::echo_interval, "Echo interval");
	_classObj.def_readwrite("file", &CubicCompressionEngine::file, "Output file");
	_classObj.def_readwrite("debug", &CubicCompressionEngine::debug, "Debug flag");
	
	// Expose methods
	_classObj.def("getStress", &CubicCompressionEngine::getStress, "Get stress");
	_classObj.def("get_strain", &CubicCompressionEngine::get_strain, "Get strain as list");
}
