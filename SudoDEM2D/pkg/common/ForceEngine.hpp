// 2004 © Janek Kozicki <cosurgi@berlios.de>
// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
// 2014 © Raphael Maurin <raphael.maurin@irstea.fr>

#pragma once

#include<sudodem/core/PartialEngine.hpp>

class ForceEngine: public PartialEngine{
	public:
		virtual void action() override;
		Vector2r force;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<ForceEngine, PartialEngine, std::shared_ptr<ForceEngine>> _classObj(_module, "ForceEngine", "Apply contact force on some particles at each step.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("force", &ForceEngine::force, "Force to apply.");
			_classObj.def("action", &ForceEngine::action);
		}
};
REGISTER_SERIALIZABLE_BASE(ForceEngine, PartialEngine);

/* Engine for applying force of varying magnitude but constant direction
 * on subscribed bodies. times and magnitudes must have the same length,
 * direction (normalized automatically) gives the orientation.
 *
 * As usual with interpolating engines: the first magnitude is used before the first
 * time point, last magnitude is used after the last time point. Wrap specifies whether
 * time wraps around the last time point to the first time point.
 */
class InterpolatingDirectedForceEngine: public ForceEngine{
	size_t _pos;
	public:
		virtual void action() override;
		vector<Real> times;
		vector<Real> magnitudes;
		Vector2r direction;
		bool wrap;
		
		InterpolatingDirectedForceEngine() : _pos(0), wrap(false), direction(1.0, 0) {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
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
};
REGISTER_SERIALIZABLE_BASE(InterpolatingDirectedForceEngine, ForceEngine);

struct RadialForceEngine: public PartialEngine{
	virtual void action() override;
	virtual void postLoad(RadialForceEngine&);
	Vector2r axisPt;
	Vector2r axisDir;
	Real fNorm;
	
	virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<RadialForceEngine, PartialEngine, std::shared_ptr<RadialForceEngine>> _classObj(_module, "RadialForceEngine", "Apply force of given magnitude directed away from spatial axis.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("axisPt", &RadialForceEngine::axisPt, "Point on axis");
		_classObj.def_readwrite("axisDir", &RadialForceEngine::axisDir, "Axis direction (normalized automatically)");
		_classObj.def_readwrite("fNorm", &RadialForceEngine::fNorm, "Applied force magnitude");
		_classObj.def("action", &RadialForceEngine::action);
		_classObj.def("postLoad", &RadialForceEngine::postLoad);
	}
};
REGISTER_SERIALIZABLE_BASE(RadialForceEngine, ForceEngine);

class DragEngine: public PartialEngine{
	public:
		virtual void action() override;
		Real Rho;
		Real Cd;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<DragEngine, PartialEngine, std::shared_ptr<DragEngine>> _classObj(_module, "DragEngine", "Apply `drag force <http://en.wikipedia.org/wiki/Drag_equation>`__ on some particles at each step, decelerating them proportionally to their linear velocities. The applied force reads\n\n.. math:: F_{d}=-\\frac{\\vec{v}}{|\\vec{v}|}\\frac{1}{2}\\rho|\\vec{v}|^2 C_d A\n\nwhere $\\rho$ is the medium density (:yref:`density<DragEngine.Rho>`), $v$ is particle's velocity,  $A$ is particle projected area (disc), $C_d$ is the drag coefficient (0.47 for :yref:`Disk`), \n\n.. note:: Drag force is only applied to spherical particles, listed in ids.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("Rho", &DragEngine::Rho, "Density of the medium (fluid or air), by default - the density of the air.");
			_classObj.def_readwrite("Cd", &DragEngine::Cd, "Drag coefficient <http://en.wikipedia.org/wiki/Drag_coefficient>`_.");
			_classObj.def("action", &DragEngine::action);
		}
};
REGISTER_SERIALIZABLE_BASE(DragEngine, ForceEngine);

class LinearDragEngine: public PartialEngine{
	public:
		virtual void action() override;
		Real nu;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<LinearDragEngine, PartialEngine, std::shared_ptr<LinearDragEngine>> _classObj(_module, "LinearDragEngine", "Apply `viscous resistance or linear drag <http://en.wikipedia.org/wiki/Drag_%28physics%29#Very_low_Reynolds_numbers_.E2.80.94_Stokes.27_drag>`__ on some particles at each step, decelerating them proportionally to their linear velocities. The applied force reads\n\n.. math:: F_{d}=-b{\\vec{v}} \n\nwhere $b$ is the linear drag, $\\vec{v}$ is particle's velocity. \n\n.. math:: b=6\\pi\\nu r \n\nwhere $\\nu$ is the medium viscosity, $r$ is the `Stokes radius <http://en.wikipedia.org/wiki/Stokes_radius>`__ of the particle (but in this case we accept it equal to disk radius for simplification), \n\n.. note:: linear drag is only applied to spherical particles, listed in ids.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("nu", &LinearDragEngine::nu, "Viscosity of the medium.");
			_classObj.def("action", &LinearDragEngine::action);
		}
};
REGISTER_SERIALIZABLE_BASE(LinearDragEngine, DragEngine);


class HydroForceEngine: public PartialEngine{
	private:
		vector<Vector2r> vFluct;
	public:
		virtual void action() override;
		Real rhoFluid;
		Real viscoDyn;
		Real zRef;
		Real nCell;
		Real deltaZ;
		Real expoRZ;
		bool lift;
		Real Cl;
		Vector2r gravity;
		vector<Real> vxFluid;
		vector<Real> phiPart;
		bool velFluct;
		vector<Real> simplifiedReynoldStresses;
		Real bedElevation;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<HydroForceEngine, PartialEngine, std::shared_ptr<HydroForceEngine>> _classObj(_module, "HydroForceEngine", "Apply drag and lift due to a fluid flow vector (1D) to each disk + the buoyant weight.\n The applied drag force reads\n\n.. math:: F_{d}=\\frac{1}{2} C_d A\\rho^f|\\vec{v_f - v}| vec{v_f - v} \n\n where $\\rho$ is the medium density (:yref:`density<HydroForceEngine.rhoFluid>`), $v$ is particle's velocity,  $v_f$ is the velocity of the fluid at the particle center,  $A$ is particle projected area (disc), $C_d$ is the drag coefficient. The formulation of the drag coefficient depends on the local particle reynolds number and the solid volume fraction. The formulation of the drag is [Dallavalle1948]_ [RevilBaudard2013]_ with a correction of Richardson-Zaki [Richardson1954]_ to take into account the hindrance effect. This law is classical in sediment transport. It is possible to activate a fluctuation of the drag force for each particle which account for the turbulent fluctuation of the fluid velocity (:yref:`velFluct`). The model implemented for the turbulent velocity fluctuation is a simple discrete random walk which takes as input the reynolds stress tensor Re_{xz} in function of the depth and allows to recover the main property of the fluctuations by imposing <u_x'u_z'> (z) = <Re>(z)/rho^f. It requires as input <Re>(z)/rho^f called :yref:`simplifiedReynoldStresses` in the code. \n The formulation of the lift is taken from [Wiberg1985]_ and is such that : \n\n.. math:: F_{L}=\\frac{1}{2} C_L A\\rho^f((v_f - v)^2{top} - (v_f - v)^2{bottom}) \n\n Where the subscript top and bottom means evaluated at the top (respectively the bottom) of the disk considered. This formulation of the lift account for the difference of pressure at the top and the bottom of the particle inside a turbulent shear flow. As this formulation is controversial when approaching the threshold of motion [Schmeeckle2007]_ it is possible to desactivate it with the variable :yref:`lift`.\n The buoyancy is taken into account through the buoyant weight : \n\n.. math:: F_{buoyancy}= - rho^f V^p g \n\n, where g is the gravity vector along the vertical, and V^p is the volume of the particle.");
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
};
REGISTER_SERIALIZABLE_BASE(HydroForceEngine, ForceEngine);
