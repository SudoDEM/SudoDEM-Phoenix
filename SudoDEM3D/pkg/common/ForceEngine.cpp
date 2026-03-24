// 2004 © Janek Kozicki <cosurgi@berlios.de>
// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
// 2014 © Raphael Maurin <raphael.maurin@irstea.fr>

#include <sudodem/pkg/common/ForceEngine.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/lib/smoothing/LinearInterpolate.hpp>
#include<sudodem/pkg/dem/Shop.hpp>

#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/IPhys.hpp>

#include <random>
#include <random>
#include <random>

// pyRegisterClass implementations moved from ForceEngine.hpp
void ForceEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<ForceEngine, PartialEngine, std::shared_ptr<ForceEngine>> _classObj(_module, "ForceEngine", "Apply contact force on some particles at each step.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("force", &ForceEngine::force, "Force to apply.");
	_classObj.def("action", &ForceEngine::action);
}

void InterpolatingDirectedForceEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<InterpolatingDirectedForceEngine, ForceEngine, std::shared_ptr<InterpolatingDirectedForceEngine>> _classObj(_module, "InterpolatingDirectedForceEngine", "Engine for applying force of varying magnitude but constant direction on subscribed bodies. times and magnitudes must have the same length, direction (normalized automatically) gives the orientation. \n\n\
	\
	As usual with interpolating engines: the first magnitude is used before the first time point, last magnitude is used after the last time point. Wrap specifies whether time wraps around the last time point to the first time point.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("times", &InterpolatingDirectedForceEngine::times, "Time readings [s]");
	_classObj.def_readwrite("magnitudes", &InterpolatingDirectedForceEngine::magnitudes, "Force magnitudes readings [N]");
	_classObj.def_readwrite("direction", &InterpolatingDirectedForceEngine::direction, "Contact force direction (normalized automatically)");
	_classObj.def_readwrite("wrap", &InterpolatingDirectedForceEngine::wrap, "wrap to the beginning of the sequence if beyond the last time point");
	_classObj.def("action", &InterpolatingDirectedForceEngine::action);
}

void RadialForceEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<RadialForceEngine, PartialEngine, std::shared_ptr<RadialForceEngine>> _classObj(_module, "RadialForceEngine", "Apply force of given magnitude directed away from spatial axis.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("axisPt", &RadialForceEngine::axisPt, "Point on axis");
	_classObj.def_readwrite("axisDir", &RadialForceEngine::axisDir, "Axis direction (normalized automatically)");
	_classObj.def_readwrite("fNorm", &RadialForceEngine::fNorm, "Applied force magnitude");
	_classObj.def("action", &RadialForceEngine::action);
	_classObj.def("postLoad", &RadialForceEngine::postLoad);
}

void DragEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<DragEngine, PartialEngine, std::shared_ptr<DragEngine>> _classObj(_module, "DragEngine", "Apply `drag force <http://en.wikipedia.org/wiki/Drag_equation>`__ on some particles at each step, decelerating them proportionally to their linear velocities. The applied force reads\n\n.. math:: F_{d}=-\\frac{\\vec{v}}{|\\vec{v}|}\\frac{1}{2}\\rho|\\vec{v}|^2 C_d A\n\nwhere $\\rho$ is the medium density (:yref:`density<DragEngine.Rho>`), $v$ is particle's velocity,  $A$ is particle projected area (sphere), $C_d$ is the drag coefficient (0.47 for :yref:`Sphere`), \n\n.. note:: Drag force is only applied to spherical particles, listed in ids.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("Rho", &DragEngine::Rho, "Density of the medium (fluid or air), by default - the density of the air.");
	_classObj.def_readwrite("Cd", &DragEngine::Cd, "Drag coefficient <http://en.wikipedia.org/wiki/Drag_coefficient>`_.");
	_classObj.def("action", &DragEngine::action);
}

void LinearDragEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<LinearDragEngine, PartialEngine, std::shared_ptr<LinearDragEngine>> _classObj(_module, "LinearDragEngine", "Apply `viscous resistance or linear drag <http://en.wikipedia.org/wiki/Drag_%28physics%29#Very_low_Reynolds_numbers_.E2.80.94_Stokes.27_drag>`__ on some particles at each step, decelerating them proportionally to their linear velocities. The applied force reads\n\n.. math:: F_{d}=-b{\\vec{v}} \n\nwhere $b$ is the linear drag, $\\vec{v}$ is particle's velocity. \n\n.. math:: b=6\\pi\\nu r \n\nwhere $\\nu$ is the medium viscosity, $r$ is the `Stokes radius <http://en.wikipedia.org/wiki/Stokes_radius>`__ of the particle (but in this case we accept it equal to sphere radius for simplification), \n\n.. note:: linear drag is only applied to spherical particles, listed in ids.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("nu", &LinearDragEngine::nu, "Viscosity of the medium.");
	_classObj.def("action", &LinearDragEngine::action);
}

void HydroForceEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<HydroForceEngine, PartialEngine, std::shared_ptr<HydroForceEngine>> _classObj(_module, "HydroForceEngine", "Apply drag and lift due to a fluid flow vector (1D) to each sphere + the buoyant weight.\n The applied drag force reads\n\n.. math:: F_{d}=\\frac{1}{2} C_d A\\rho^f|\\vec{v_f - v}| vec{v_f - v} \n\n where $\\rho$ is the medium density (:yref:`density<HydroForceEngine.rhoFluid>`), $v$ is particle's velocity,  $v_f$ is the velocity of the fluid at the particle center,  $A$ is particle projected area (sphere), $C_d$ is the drag coefficient. The formulation of the drag coefficient depends on the local particle reynolds number and the solid volume fraction. The formulation of the drag is [Dallavalle1948]_ [RevilBaudard2013]_ with a correction of Richardson-Zaki [Richardson1954]_ to take into account the hindrance effect. This law is classical in sediment transport. It is possible to activate a fluctuation of the drag force for each particle which account for the turbulent fluctuation of the fluid velocity (:yref:`velFluct`). The model implemented for the turbulent velocity fluctuation is a simple discrete random walk which takes as input the reynolds stress tensor Re_{xz} in function of the depth and allows to recover the main property of the fluctuations by imposing <u_x'u_z'> (z) = <Re>(z)/rho^f. It requires as input <Re>(z)/rho^f called :yref:`simplifiedReynoldStresses` in the code. \n The formulation of the lift is taken from [Wiberg1985]_ and is such that : \n\n.. math:: F_{L}=\\frac{1}{2} C_L A\\rho^f((v_f - v)^2{top} - (v_f - v)^2{bottom}) \n\n Where the subscript top and bottom means evaluated at the top (respectively the bottom) of the sphere considered. This formulation of the lift account for the difference of pressure at the top and the bottom of the particle inside a turbulent shear flow. As this formulation is controversial when approaching the threshold of motion [Schmeeckle2007]_ it is possible to desactivate it with the variable...");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("rhoFluid", &HydroForceEngine::rhoFluid, "Density of the fluid, by default - density of water");
	_classObj.def_readwrite("viscoDyn", &HydroForceEngine::viscoDyn, "Dynamic viscosity of the fluid, by default - viscosity of water");
	_classObj.def_readwrite("zRef", &HydroForceEngine::zRef, "Position of the reference point which correspond to the first value of the fluid velocity");
	_classObj.def_readwrite("nCell", &HydroForceEngine::nCell, "Size of the vector of the fluid velocity");
	_classObj.def_readwrite("deltaZ", &HydroForceEngine::deltaZ, "width of the discretization cell ");
	_classObj.def_readwrite("expoRZ", &HydroForceEngine::expoRZ, "Value of the Richardson-Zaki exponent, for the correction due to hindrance");
	_classObj.def_readwrite("lift", &HydroForceEngine::lift, "Option to activate or not the evaluation of the lift");
	_classObj.def_readwrite("Cl", &HydroForceEngine::Cl, "Value of the lift coefficient taken from [Wiberg1985]_");
	_classObj.def_readwrite("gravity", &HydroForceEngine::gravity, "Gravity vector (may depend on the slope).");
	_classObj.def_readwrite("vxFluid", &HydroForceEngine::vxFluid, "Discretized streamwise fluid velocity profile in function of the depth");
	_classObj.def_readwrite("phiPart", &HydroForceEngine::phiPart, "Discretized solid volume fraction profile in function of the depth");
	_classObj.def_readwrite("velFluct", &HydroForceEngine::velFluct, "If true, activate the determination of turbulent fluid velocity fluctuation at the position of each particle, using a simple discrete random walk model based on the Reynolds stresses :yref:`turbStress<HydroForceEngine.squaredAverageTurbfluct>`");
	_classObj.def_readwrite("simplifiedReynoldStresses", &HydroForceEngine::simplifiedReynoldStresses, "Vector of size equal to :yref:`turbStress<HydroForceEngine.nCell>` containing the Reynolds stresses divided by the fluid density in function of the depth. simplifiedReynoldStresses(z) =  <u_x'u_z'>(z)^2 ");
	_classObj.def_readwrite("bedElevation", &HydroForceEngine::bedElevation, "Elevation of the bed above which the fluid flow is turbulent and the particles undergo turbulent velocity fluctuation.");
	_classObj.def("action", &HydroForceEngine::action);
}


void ForceEngine::action(){
	for(Body::id_t id : ids){
		if (!(scene->bodies->exists(id))) continue;
		scene->forces.addForce(id,force);
	}
}

void InterpolatingDirectedForceEngine::action(){
	Real virtTime=wrap ? Shop::periodicWrap(scene->time,*times.begin(),*times.rbegin()) : scene->time;
	direction.normalize();
	force=linearInterpolate<Real,Real>(virtTime,times,magnitudes,_pos)*direction;
	ForceEngine::action();
}

void RadialForceEngine::postLoad(RadialForceEngine&){ axisDir.normalize(); }

void RadialForceEngine::action(){
	for(Body::id_t id : ids){
		if (!(scene->bodies->exists(id))) continue;
		const Vector3r& pos=Body::byId(id,scene)->state->pos;
		Vector3r radial=(pos - (axisPt+axisDir * /* t */ ((pos-axisPt).dot(axisDir)))).normalized();
		if(radial.squaredNorm()==0) continue;
		scene->forces.addForce(id,fNorm*radial);
	}
}

void DragEngine::action(){
	for(Body::id_t id : ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Real A = sphere->radius*sphere->radius*Mathr::PI;	//Crossection of the sphere
			Vector3r velSphTemp = b->state->vel;
			Vector3r dragForce = Vector3r::Zero();

			if (velSphTemp != Vector3r::Zero()) {
				dragForce = -0.5*Rho*A*Cd*velSphTemp.squaredNorm()*velSphTemp.normalized();
			}
			scene->forces.addForce(id,dragForce);
		}
	}
}

void LinearDragEngine::action(){
	for(Body::id_t id : ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Vector3r velSphTemp = b->state->vel;
			Vector3r dragForce = Vector3r::Zero();

			Real b_val = 6*Mathr::PI*nu*sphere->radius;

			if (velSphTemp != Vector3r::Zero()) {
				dragForce = -b_val*velSphTemp;
			}
			scene->forces.addForce(id,dragForce);
		}
	}
}

void HydroForceEngine::action(){
	/* Velocity fluctuation determination (not usually done at each dt, that is why it not placed in the other loop) */
	if (velFluct == true){
		/* check size */
		size_t size=vFluct.size();
		if(size<scene->bodies->size()){
			size=scene->bodies->size();
			vFluct.resize(size);
		}
		/* reset stored values to zero */
		memset(& vFluct[0],0,sizeof(Vector3r)*size);

		/* Create a random number generator rnd() with a gaussian distribution of mean 0 and stdev 1.0 */
		/* Uses C++11 random (std::normal_distribution) - see Numerical Recipes in C, chapter 7 (1992) */
		static std::minstd_rand randGen((int)TimingInfo::getNow(true));
		static std::normal_distribution<Real> dist(0.0, 1.0);
		auto rnd = [&]() { return dist(randGen); };

		double rand1 = 0.0;
		double rand2 = 0.0;
		/* Attribute a fluid velocity fluctuation to each body above the bed elevation */
		for(Body::id_t id : ids){
			Body* b=Body::byId(id,scene).get();
			if (!b) continue;
			if (!(scene->bodies->exists(id))) continue;
			const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
			if (sphere){
				Vector3r posSphere = b->state->pos;//position vector of the sphere
				int p = floor((posSphere[2]-zRef)/deltaZ); //cell number in which the particle is
				// if the particle is inside the water and above the bed elevation, so inside the turbulent flow, evaluate a turbulent fluid velocity fluctuation which will be used to apply the drag.
				if ((p<nCell)&&(posSphere[2]-zRef>bedElevation)) {
					Real uStar2 = simplifiedReynoldStresses[p];
					if (uStar2>0.0){
						Real uStar = sqrt(uStar2);
						rand1 = rnd();
						rand2 = -rand1 + rnd();
						vFluct[id] = Vector3r(rand1*uStar,rand2*uStar,0);
					}
				}
			}

		}
		velFluct = false;
	}

	/* Application of hydrodynamical forces */
	for(Body::id_t id : ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Vector3r posSphere = b->state->pos;//position vector of the sphere
			int p = floor((posSphere[2]-zRef)/deltaZ); //cell number in which the particle is
			if ((p<nCell)&&(p>0)) {
				Vector3r liftForce = Vector3r::Zero();
				Vector3r dragForce = Vector3r::Zero();
				Vector3r fluctVelBody = vFluct[id];//fluid velocity fluctuation associated to the particle's position considered.
				Vector3r vFluid(vxFluid[p]+fluctVelBody.x(),fluctVelBody.y(),0); //fluid velocity at the particle's position
				Vector3r vPart = b->state->vel;//particle velocity
				Vector3r vRel = vFluid - vPart;//fluid-particle relative velocity

				//Drag force calculation
				Real Rep = vRel.norm()*sphere->radius*2*rhoFluid/viscoDyn; //particles reynolds number
				Real A = sphere->radius*sphere->radius*Mathr::PI;	//Crossection of the sphere
				if (vRel.norm()!=0.0) {
					Real hindranceF = pow(1-phiPart[p],-expoRZ); //hindrance function
					Real Cd = (0.44 + 24.4/Rep)*hindranceF; //drag coefficient
					dragForce = 0.5*rhoFluid*A*Cd*vRel.squaredNorm()*vRel.normalized();
				}
				//lift force calculation due to difference of fluid pressure between top and bottom of the particle
				int intRadius = floor(sphere->radius/deltaZ);
				if ((p+intRadius<nCell)&&(p-intRadius>0)&&(lift==true)) {
					Real vRelTop = vxFluid[p+intRadius] - vPart[0]; // relative velocity of the fluid wrt the particle at the top of the particle
					Real vRelBottom = vxFluid[p-intRadius] - vPart[0]; // same at the bottom
					liftForce[2] = 0.5*rhoFluid*A*Cl*(vRelTop*vRelTop-vRelBottom*vRelBottom);
				}
				//buoyant weight force calculation
				Vector3r buoyantForce = -4.0/3.0*Mathr::PI*sphere->radius*sphere->radius*sphere->radius*rhoFluid*gravity;
				//add the hydro forces to the particle
				scene->forces.addForce(id,dragForce+liftForce+buoyantForce);
			}
		}
	}
}
