// =======================================================
// Some plugins from removed CPP-fiels

//#include <sudodem/pkg/dem/DemXDofGeom.hpp>
#include <sudodem/pkg/common/TorqueEngine.hpp>
#include <sudodem/pkg/common/ForceResetter.hpp>
#include <sudodem/pkg/common/FieldApplier.hpp>
#include <sudodem/pkg/common/Callbacks.hpp>
#include <sudodem/pkg/common/BoundaryController.hpp>
#include <sudodem/pkg/common/NormShearPhys.hpp>
#include <sudodem/pkg/common/Recorder.hpp>
#include <sudodem/pkg/common/Box.hpp>
#include <sudodem/pkg/common/Facet.hpp>
#include <sudodem/pkg/common/StepDisplacer.hpp>
#include <sudodem/pkg/common/PeriodicEngines.hpp>
#include <sudodem/pkg/common/ElastMat.hpp>
#include <sudodem/pkg/common/PyRunner.hpp>
#include <sudodem/pkg/common/Sphere.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include <sudodem/pkg/common/KinematicEngines.hpp>

// pyRegisterClass implementations moved from ElastMat.hpp
void ElastMat::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<ElastMat, Material, std::shared_ptr<ElastMat>> _classObj(_module, "ElastMat", "Purely elastic material. The material parameters may have different meanings depending on the :yref:`IPhysFunctor` used : true Young and Poisson in :yref:`Ip2_FrictMat_FrictMat_MindlinPhys`, or contact stiffnesses in :yref:`Ip2_FrictMat_FrictMat_FrictPhys`.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("Kn", &ElastMat::Kn, "Contact normal stiffness [N/m].");
	_classObj.def_readwrite("Ks", &ElastMat::Ks, "Contact tangential stiffness [N/m].");
	_classObj.def_readwrite("young", &ElastMat::young, "Young's modulus [Pa].");
	_classObj.def_readwrite("poisson", &ElastMat::poisson, "Poisson's ratio.");
}

void FrictMat::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<FrictMat, ElastMat, std::shared_ptr<FrictMat>> _classObj(_module, "FrictMat", "Elastic material with contact friction. See also :yref:`ElastMat`.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("frictionAngle", &FrictMat::frictionAngle, "Contact friction angle (in radians). Hint : use 'radians(degreesValue)' in python scripts.");
}

// pyRegisterClass implementations moved from NormShearPhys.hpp
void NormPhys::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<NormPhys, IPhys, std::shared_ptr<NormPhys>> _classObj(_module, "NormPhys", "Abstract class for interactions that have normal stiffness.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("kn", &NormPhys::kn, "Normal stiffness");
	_classObj.def_readwrite("normalForce", &NormPhys::normalForce, "Normal force after previous step (in global coordinates).");
}

void NormShearPhys::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<NormShearPhys, NormPhys, std::shared_ptr<NormShearPhys>> _classObj(_module, "NormShearPhys", "Abstract class for interactions that have shear stiffnesses, in addition to normal stiffness. This class is used in the PFC3d-style stiffness timestepper.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("ks", &NormShearPhys::ks, "Shear stiffness");
	_classObj.def_readwrite("shearForce", &NormShearPhys::shearForce, "Shear force after previous step (in global coordinates).");
}

// pyRegisterClass implementations moved from StepDisplacer.hpp
void StepDisplacer::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<StepDisplacer, PartialEngine, std::shared_ptr<StepDisplacer>> _classObj(_module, "StepDisplacer", "Apply generalized displacement (displacement or rotation) stepwise on subscribed bodies. Could be used for purposes of contact law tests (by moving one sphere compared to another), but in this case, see rather :yref:`LawTester`");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("mov", &StepDisplacer::mov, "Linear displacement step to be applied per iteration, by addition to :yref:`State.pos`.");
	_classObj.def_readwrite("rot", &StepDisplacer::rot, "Rotation step to be applied per iteration (via rotation composition with :yref:`State.ori`).");
	_classObj.def_readwrite("setVelocities", &StepDisplacer::setVelocities, "If false, positions and orientations are directly updated, without changing the speeds of concerned bodies. If true, only velocity and angularVelocity are modified. In this second case :yref:`integrator<NewtonIntegrator>` is supposed to be used, so that, thanks to this Engine, the bodies will have the prescribed jump over one iteration (dt).");
}

// pyRegisterClass implementations moved from ForceResetter.hpp
void ForceResetter::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<ForceResetter, GlobalEngine, std::shared_ptr<ForceResetter>> _classObj(_module, "ForceResetter", "Reset all forces stored in Scene::forces (``O.forces`` in python). Typically, this is the first engine to be run at every step. In addition, reset those energies that should be reset, if energy tracing is enabled.");
	_classObj.def(pybind11::init<>());
}

// pyRegisterClass implementations moved from TorqueEngine.hpp
void TorqueEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<TorqueEngine, PartialEngine, std::shared_ptr<TorqueEngine>> _classObj(_module, "TorqueEngine", "Apply given torque (momentum) value at every subscribed particle, at every step.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("moment", &TorqueEngine::moment, "Torque value to be applied.");
}

// pyRegisterClass implementations moved from Callbacks.hpp
void IntrCallback::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<IntrCallback, Serializable, std::shared_ptr<IntrCallback>> _classObj(_module, "IntrCallback", "Abstract callback object which will be called for every (real) :yref:`Interaction` after the interaction has been processed by :yref:`InteractionLoop`.\n\nAt the beginning of the interaction loop, ``stepInit`` is called, initializing the object; it returns either ``NULL`` (to deactivate the callback during this time step) or pointer to function, which will then be passed (1) pointer to the callback object itself and (2) pointer to :yref:`Interaction`.\n\n.. note::\n\t(NOT YET DONE) This functionality is accessible from python by passing 4th argument to :yref:`InteractionLoop` constructor, or by appending the callback object to :yref:`InteractionLoop::callbacks`.\n");
	_classObj.def(pybind11::init<>());
}

#ifdef SUDODEM_BODY_CALLBACKS
void BodyCallback::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<BodyCallback, Serializable, std::shared_ptr<BodyCallback>> _classObj(_module, "BodyCallback", "Abstract callback object which will be called for every :yref:`Body` after being processed by :yref:`NewtonIntegrator`. See :yref:`IntrCallback` for details.");
	_classObj.def(pybind11::init<>());
}
#endif

// pyRegisterClass implementations moved from BoundaryController.hpp
void BoundaryController::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<BoundaryController, GlobalEngine, std::shared_ptr<BoundaryController>> _classObj(_module, "BoundaryController", "Base for engines controlling boundary conditions of simulations. Not to be used directly.");
	_classObj.def(pybind11::init<>());
}

// pyRegisterClass implementations moved from FieldApplier.hpp
void FieldApplier::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<FieldApplier, GlobalEngine, std::shared_ptr<FieldApplier>> _classObj(_module, "FieldApplier", "Base for engines applying force files on particles. Not to be used directly.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("fieldWorkIx", &FieldApplier::fieldWorkIx, "Index for the work done by this field, if tracking energies.");
	_classObj.def("action", &FieldApplier::action);
}

// pyRegisterClass implementations moved from PyRunner.hpp
void PyRunner::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<PyRunner, PeriodicEngine, std::shared_ptr<PyRunner>> _classObj(_module, "PyRunner", "Execute a python command periodically, with defined (and adjustable) periodicity. See :yref:`PeriodicEngine` documentation for details.");
	_classObj.def(pybind11::init<>());
	_classObj.def(pybind11::init([](const std::string& command, Real virtPeriod, Real realPeriod, long iterPeriod, long nDo, bool initRun, const std::string& label, bool dead) {
		auto runner = std::make_shared<PyRunner>();
		runner->command = command;
		runner->virtPeriod = virtPeriod;
		runner->realPeriod = realPeriod;
		runner->iterPeriod = iterPeriod;
		runner->nDo = nDo;
		runner->initRun = initRun;
		runner->label = label;
		runner->dead = dead;
		return runner;
	}), pybind11::arg("command") = "", pybind11::arg("virtPeriod") = 0, pybind11::arg("realPeriod") = 0, pybind11::arg("iterPeriod") = 0, pybind11::arg("nDo") = -1, pybind11::arg("initRun") = false, pybind11::arg("label") = "", pybind11::arg("dead") = false);
	_classObj.def_readwrite("command", &PyRunner::command, "Command to be run by python interpreter. Not run if empty.");
}

// pyRegisterClass implementations moved from Recorder.hpp
void Recorder::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<Recorder, PeriodicEngine, std::shared_ptr<Recorder>> _classObj(_module, "Recorder", "Engine periodically storing some data to (one) external file. In addition PeriodicEngine, it handles opening the file as needed. See :yref:`PeriodicEngine` for controlling periodicity.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("file", &Recorder::file, "Name of file to save to; must not be empty.");
	_classObj.def_readwrite("truncate", &Recorder::truncate, "Whether to delete current file contents, if any, when opening (false by default)");
	_classObj.def_readwrite("addIterNum", &Recorder::addIterNum, "Adds an iteration number to the file name, when the file was created. Useful for creating new files at each call (false by default)");
}

// pyRegisterClass implementations moved from PeriodicEngines.hpp
void PeriodicEngine::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<PeriodicEngine, GlobalEngine, std::shared_ptr<PeriodicEngine>> _classObj(_module, "PeriodicEngine", 
		"Run Engine::action with given fixed periodicity real time (=wall clock time, computation time), \
		virtual time (simulation time), iteration number), by setting any of those criteria \
		(virtPeriod, realPeriod, iterPeriod) to a positive value. They are all negative (inactive)\
		by default.\n\n\
		\
		\
		The number of times this engine is activated can be limited by setting nDo>0. If the number of activations \
		will have been already reached, no action will be called even if an active period has elapsed. \n\n\
		\
		If initRun is set (false by default), the engine will run when called for the first time; otherwise it will only \
		start counting period (realLast etc interal variables) from that point, but without actually running, and will run \
		only once a period has elapsed since the initial run. \n\n\
		\
		This class should not be used directly; rather, derive your own engine which you want to be run periodically. \n\n\
		\
		Derived engines should override Engine::action(), which will be called periodically. If the derived Engine \
		overrides also Engine::isActivated, it should also take in account return value from PeriodicEngine::isActivated, \
		since otherwise the periodicity will not be functional. \n\n\
		\
		Example with :yref:`PyRunner`, which derives from PeriodicEngine; likely to be encountered in python scripts:: \n\n\
		\
			PyRunner(realPeriod=5,iterPeriod=10000,command='print O.iter')	\n\n\
		\
		will print iteration number every 10000 iterations or every 5 seconds of wall clock time, whiever comes first since it was \
		last run.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("virtPeriod", &PeriodicEngine::virtPeriod, "Periodicity criterion using virtual (simulation) time (deactivated if <= 0)");
	_classObj.def_readwrite("realPeriod", &PeriodicEngine::realPeriod, "Periodicity criterion using real (wall clock, computation, human) time (deactivated if <=0)");
	_classObj.def_readwrite("iterPeriod", &PeriodicEngine::iterPeriod, "Periodicity criterion using step number (deactivated if <= 0)");
	_classObj.def_readwrite("nDo", &PeriodicEngine::nDo, "Limit number of executions by this number (deactivated if negative)");
	_classObj.def_readwrite("initRun", &PeriodicEngine::initRun, "Run the first time we are called as well.");
	_classObj.def_readwrite("virtLast", &PeriodicEngine::virtLast, "Tracks virtual time of last run |yupdate|.");
	_classObj.def_readwrite("realLast", &PeriodicEngine::realLast, "Tracks real time of last run |yupdate|.");
	_classObj.def_readwrite("iterLast", &PeriodicEngine::iterLast, "Tracks step number of last run |yupdate|.");
	_classObj.def_readwrite("nDone", &PeriodicEngine::nDone, "Track number of executions (cummulative) |yupdate|.");
	_classObj.def("isActivated", &PeriodicEngine::isActivated);
	_classObj.def("getClock", &PeriodicEngine::getClock);
}

#ifdef SUDODEM_BODY_CALLBACK
#else
#endif

#ifdef SUDODEM_OPENGL
#include <sudodem/lib/opengl/OpenGLWrapper.hpp>
//#include <sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/GLDrawFunctors.hpp>
#endif
