/*************************************************************************
*  Copyright (C) 2016 by Zhswee                                          *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include <sudodem/pkg/dem/CylindricalCompressionEngine.hpp>
#include <sudodem/pkg/dem/Shop.hpp>
#include <sudodem/core/Scene.hpp>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/pkg/dem/Superquadrics.hpp>
#include <sudodem/pkg/dem/PolySuperellipsoid.hpp>
#include <sudodem/pkg/dem/FrictPhys.hpp>

CREATE_LOGGER(CylindricalCompressionEngine);

CylindricalCompressionEngine::CylindricalCompressionEngine()
	: firstRun(true), majorok(true),
	  height0(0), height(0), Init_boxVolume(0), Current_boxVolume(0),
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
	  wall_radius(5.0), wall_radius0(0), left_facet_pos(0), loading_area(0),
	  cylinder_stress(0), cylinder_force(0), num_pwall(0),
	  gain_x(0), avg_xstiff(0), gain_alpha(0.5),
	  wszz(0), gain_z(0), avg_zstiff(0)
{
	strain[0] = strain[1] = strain[2] = 0;
	for(int j = 0; j < 6; j++) {
		wall_stiffness[j] = 0;
		wall_max_vel[j] = 1e5;
	}
}

CylindricalCompressionEngine::~CylindricalCompressionEngine() {}

void CylindricalCompressionEngine::updateStiffness() {
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
				case 4: wall_stiffness[4] += currentContactPhysics->kn; break;
				case 5: wall_stiffness[5] += currentContactPhysics->kn; break;
			}
		}
	}
}

void CylindricalCompressionEngine::updateBoxSize() {
	// Cylinder uses only top/bottom for height
	Real top = box[top_wall]->state->pos.z();
	Real bottom = box[bottom_wall]->state->pos.z();
	height = top - bottom;
	loading_area = Mathr::PI * wall_radius * wall_radius;
}

void CylindricalCompressionEngine::getBox() {
	// Get the box walls - only need top and bottom for cylinder
	for(int i = 0; i < 6; i++) {
		box[i] = Body::byId(i, scene);
	}
}

Matrix3r CylindricalCompressionEngine::getStressTensor() {
	updateBoxSize();
	Current_boxVolume = height * loading_area;
	
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
		
		// Calculate stress contribution for cylindrical geometry
		// ... (implementation)
	}
	
	return Sigma;
}

Vector3r CylindricalCompressionEngine::getStress() {
	Matrix3r Sigma = getStressTensor();
	Real meanStress = Sigma.trace() / 3.0;
	Sigma -= meanStress * Matrix3r::Identity();
	Real deviatorStress = Sigma.norm() * sqrt(1.5);
	return Vector3r(meanStress, deviatorStress, 0.0);
}

void CylindricalCompressionEngine::get_gain() {
	// Cylinder specific gain calculation
	avg_xstiff = wall_stiffness[0];
	avg_zstiff = (wall_stiffness[4] + wall_stiffness[5]) / 2.0;
	
	gain_x = 2.0 * gain_alpha * height * Mathr::PI * 2.0 * wall_radius / (num_pwall * 1e8);
	gain_z = 2.0 * gain_alpha * height / (avg_zstiff * 1e8);
}

void CylindricalCompressionEngine::servo(Real dt) {
	// For cylinder, use servo_cylinder instead
	servo_cylinder(dt);
}

void CylindricalCompressionEngine::servo_cylinder(Real dt) {
	consol_ss_cylinder();
	Real udr, udz;
	gain_x = 2.0 * gain_alpha * height * Mathr::PI * 2.0 * wall_radius / (num_pwall * 1e8);
	udr = gain_x * (cylinder_stress - goalx);
	wall_radius += udr;
	
	if(z_servo) {
		udz = gain_z * (wszz - goalz) / dt;
		box[bottom_wall]->state->vel[2] = -udz;
		box[top_wall]->state->vel[2] = udz;
	}
}

void CylindricalCompressionEngine::consol_ss_cylinder() {
	cylinder_force = 0.0;
	num_pwall = 0;
	
	BodyContainer::iterator bb = scene->bodies->begin();
	BodyContainer::iterator iiEnd = scene->bodies->end();
	
	for(; bb != iiEnd; ++bb) {
		const Body::id_t& id = (*bb)->getId();
		if(id < 2) continue;
		
		State* state = (*bb)->state.get();
		Vector3r B_pos = state->pos;
		Vector3r dist = Vector3r(B_pos(0), B_pos(1), 0.0);
		
		if(dist.norm() > wall_radius) {
			continue;
		}
		
		num_pwall++;
		Vector3r force = scene->forces.getForce(id);
		Vector3r normal = Vector3r(B_pos(0), B_pos(1), 0.0);
		normal.normalize();
		cylinder_force += fabs(force.dot(normal));
	}
	
	cylinder_stress = cylinder_force / loading_area;
	
	if(interStressControl) {
		Matrix3r Sigma = getStressTensor();
		wszz = Sigma(2, 2);
	} else {
		Real bottom = getForce(scene, 0)[2] / loading_area;
		Real top = getForce(scene, 1)[2] / loading_area;
		wszz = 0.5 * (top - bottom);
	}
}

void CylindricalCompressionEngine::cylinwall_go() {
	cylinder_force = 0.0;
	num_pwall = 0;
	
	BodyContainer::iterator bb = scene->bodies->begin();
	BodyContainer::iterator iiEnd = scene->bodies->end();
	
	for(; bb != iiEnd; ++bb) {
		const Body::id_t& id = (*bb)->getId();
		if(id < 2) continue;
		
		State* state = (*bb)->state.get();
		Superquadrics* B = static_cast<Superquadrics*>((*bb)->shape.get());
		Vector3r dist = Vector3r(state->pos(0), state->pos(1), 0.0);
		Vector3r B_pos = state->pos;
		Vector3r Origin = Vector3r(0.0, 0.0, B_pos[2]);
		
		if((dist.norm() + B->getr_max()) < wall_radius) {
			continue;
		}
		
		double PenetrationDepth = 0;
		Vector3r contactpoint, normal;
		normal = Origin - B_pos;
		normal.normalize();
		
		if(B->isSphere) {
			contactpoint = B_pos;
			PenetrationDepth = dist.norm() + B->getr_max() - wall_radius;
			contactpoint = B_pos - normal * (B->getr_max() - 0.5 * PenetrationDepth);
		} else {
			Vector3r wall_pos = -normal * wall_radius;
			Matrix3r rot_mat1 = B->rot_mat2local;
			Matrix3r rot_mat2 = B->rot_mat2global;
			
			Vector2r phi = B->Normal2Phi(-rot_mat1 * normal);
			Vector3r point2 = rot_mat2 * (B->getSurface(phi)) + B_pos;
			
			Vector3r p1 = wall_pos;
			p1[2] = point2[2];
			Vector3r d = point2 - p1;
			PenetrationDepth = d.norm();
			
			if(normal.dot(d) >= 0.0) {
				PenetrationDepth = 0;
				continue;
			}
		}
		
		num_pwall++;
		Vector3r f = PenetrationDepth * 1e8 * normal;
		cylinder_force += f.norm();
		scene->forces.addForce(id, f);
	}
}

void CylindricalCompressionEngine::hydroConsolidation() {
	const Real& dt = scene->dt;
	scene->forces.sync();
	updateBoxSize();
	consol_ss_cylinder();
	
	Real delta = goalx - fabs(cylinder_stress);
	if(delta > 0) {
		Real udr = min(0.1 * gain_x * delta, hydroStrainRate * wall_radius * dt);
		wall_radius += udr;
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
		if(std::abs(stress(0) - goalx) / goalx < f_threshold) {
			if(UnbalancedForce < unbf_tol) {
				scene->stopAtIter = scene->iter + 1;
				std::cerr << "consolidation completed!" << std::endl;
			}
		}
	}
}

void CylindricalCompressionEngine::generalConsolidation() {
	const Real& dt = scene->dt;
	
	iterate_num += 1;
	solve_num += 1;
	servo_cylinder(dt);
	
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

void CylindricalCompressionEngine::checkTarget() {
	if(std::abs(cylinder_stress - goalx) / goalx < f_threshold) {
		// Target reached
	}
}

void CylindricalCompressionEngine::action_cylindrical() {
	cylinwall_go();
	if(wall_fix) return;
	
	if(firstRun) {
		box[4] = Body::byId(0);
		box[5] = Body::byId(1);
		
		Real top = box[top_wall]->state->pos.z();
		Real bottom = box[bottom_wall]->state->pos.z();
		
		height = height0 = top - bottom;
		wall_radius0 = wall_radius;
		loading_area = Mathr::PI * pow(wall_radius, 2.0);
		Init_boxVolume = height0 * loading_area;
		
		particlesVolume = 0;
		BodyContainer::iterator bi = scene->bodies->begin();
		BodyContainer::iterator biEnd = scene->bodies->end();
		for(; bi != biEnd; ++bi) {
			const shared_ptr<Body>& b = *bi;
			if(b->shape->getClassName() == "Superquadrics") {
				const shared_ptr<Superquadrics>& A = SUDODEM_PTR_CAST<Superquadrics>(b->shape);
				particlesVolume += A->getVolume();
			}
			if(b->shape->getClassName() == "PolySuperellipsoid") {
				const shared_ptr<PolySuperellipsoid>& A = SUDODEM_PTR_CAST<PolySuperellipsoid>(b->shape);
				particlesVolume += A->getVolume();
			}
		}
		rampNum = 0;
		firstRun = false;
		iterate_num = 0;
		solve_num = 0;
		get_gain();
		Flag_ForceTarget = false;
	}
	
	const Real& dt = scene->dt;
	
	if(z_servo) {
		if(!Flag_ForceTarget) {
			if(iterate_num < 1) {
				solve_num = 0;
				if(newton) { newton->quiet_system_flag = true; }
			}
			iterate_num += 1;
			servo_cylinder(dt);
			if(iterate_num >= iterate_num_max) {
				iterate_num = 1;
				get_gain();
				checkTarget();
			}
		} else {
			if(solve_num < 1) {
				if(newton) { newton->quiet_system_flag = true; }
				iterate_num = 0;
			}
			solve_num += 1;
			
			if(solve_num >= 1000) {
				UnbalancedForce = ComputeUnbalancedForce();
				solve_num = 1;
				consol_ss_cylinder();
				
				if((std::abs(cylinder_stress - goalx) / goalx > fmin_threshold) ||
				   (std::abs(wszz - goalz) / goalz > fmin_threshold)) {
					Flag_ForceTarget = false;
				}
				
				if(UnbalancedForce < unbf_tol) {
					checkTarget();
					if(Flag_ForceTarget) {
						scene->stopAtIter = scene->iter + 1;
						std::cerr << "consolidation completed!" << std::endl;
					}
				}
			}
		}
		
		if(scene->iter % savedata_interval == 0) {
			recordData();
		}
		if(scene->iter % echo_interval == 0) {
			UnbalancedForce = ComputeUnbalancedForce();
			consol_ss_cylinder();
			std::cerr << "Iter " << scene->iter << " Ubf " << UnbalancedForce
			          << ", conf:" << cylinder_stress / 1000.0
			          << ", Ss_z:" << wszz / 1000.0
			          << ", e:" << (height * loading_area / particlesVolume - 1.0)
			          << std::endl;
		}
	} else {
		updateBoxSize();
		
		if((!continueFlag) && (rampNum < ramp_interval)) {
			if(rampNum < 1) {
				if(Flag_ForceTarget) {
					std::cerr << "Shear begins..." << std::endl;
					updateBoxSize();
					height0 = height;
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
			servo_cylinder(dt);
		}
		
		if(log(height0 / height) > target_strain) {
			std::cerr << "Shear ends!" << std::endl;
			scene->stopAtIter = scene->iter + 1;
		}
		
		if(scene->iter % savedata_interval == 0) {
			recordData();
		}
		if(scene->iter % echo_interval == 0) {
			UnbalancedForce = ComputeUnbalancedForce();
			std::cerr << "Iter " << scene->iter << " Shear strain: " << log(height0 / height) << std::endl;
		}
	}
}

void CylindricalCompressionEngine::action() {
	action_cylindrical();
}

void CylindricalCompressionEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("CylindricalCompressionEngine");
	pybind11::class_<CylindricalCompressionEngine, BoundaryController, std::shared_ptr<CylindricalCompressionEngine>>
		_classObj(_module, "CylindricalCompressionEngine", "Compression engine for cylindrical specimen.");
	_classObj.def(pybind11::init<>());
	
	// Expose member variables
	_classObj.def_readwrite("wall_radius", &CylindricalCompressionEngine::wall_radius, "Wall radius");
	_classObj.def_readwrite("wall_radius0", &CylindricalCompressionEngine::wall_radius0, "Initial wall radius");
	_classObj.def_readwrite("cylinder_stress", &CylindricalCompressionEngine::cylinder_stress, "Cylinder stress");
	_classObj.def_readwrite("cylinder_force", &CylindricalCompressionEngine::cylinder_force, "Cylinder force");
	_classObj.def_readwrite("gain_alpha", &CylindricalCompressionEngine::gain_alpha, "Gain alpha");
	_classObj.def_readwrite("gain_x", &CylindricalCompressionEngine::gain_x, "Gain X");
	_classObj.def_readwrite("gain_z", &CylindricalCompressionEngine::gain_z, "Gain Z");
	_classObj.def_readwrite("goalx", &CylindricalCompressionEngine::goalx, "Goal X stress");
	_classObj.def_readwrite("goalz", &CylindricalCompressionEngine::goalz, "Goal Z stress");
	_classObj.def_readwrite("height", &CylindricalCompressionEngine::height, "Current height");
	_classObj.def_readwrite("height0", &CylindricalCompressionEngine::height0, "Initial height");
	_classObj.def_readwrite("particlesVolume", &CylindricalCompressionEngine::particlesVolume, "Particles volume");
	_classObj.def_readwrite("porosity", &CylindricalCompressionEngine::porosity, "Porosity");
	_classObj.def_readwrite("externalWork", &CylindricalCompressionEngine::externalWork, "External work");
	_classObj.def_readwrite("UnbalancedForce", &CylindricalCompressionEngine::UnbalancedForce, "Unbalanced force");
	_classObj.def_readwrite("savedata_interval", &CylindricalCompressionEngine::savedata_interval, "Save data interval");
	_classObj.def_readwrite("echo_interval", &CylindricalCompressionEngine::echo_interval, "Echo interval");
	_classObj.def_readwrite("file", &CylindricalCompressionEngine::file, "Output file");
	_classObj.def_readwrite("debug", &CylindricalCompressionEngine::debug, "Debug flag");
	
	// Expose methods
	_classObj.def("getStress", &CylindricalCompressionEngine::getStress, "Get stress");
}

void CylindricalCompressionEngine::recordData() {
	// Stub implementation - record data functionality
	// TODO: Implement data recording if needed
}

Real CylindricalCompressionEngine::ComputeUnbalancedForce(bool maxUnbalanced) {
	// Stub implementation - compute unbalanced force
	// TODO: Implement proper calculation
	return 0.0;
}
