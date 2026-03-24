#include<sudodem/core/Scene.hpp>
#include<sudodem/pkg/common/KinematicEngines.hpp>
#include<sudodem/pkg/dem/Shop.hpp>
#include<sudodem/lib/smoothing/LinearInterpolate.hpp>

// pyRegisterClass implementations moved from KinematicEngines.hpp
void CombinedKinematicEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("CombinedKinematicEngine");
	pybind11::class_<CombinedKinematicEngine, PartialEngine, std::shared_ptr<CombinedKinematicEngine>> _classObj(_module, "CombinedKinematicEngine", "Engine for applying combined displacements on pre-defined bodies. Constructed using ``+`` operator on regular :yref:`KinematicEngines<KinematicEngine>`. The ``ids`` operated on are those of the first engine in the combination (assigned automatically).");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("comb", &CombinedKinematicEngine::comb, "Kinematic engines that will be combined by this one, run in the order given.");
	_classObj.def("__add__", &CombinedKinematicEngine::appendOne);
}

void KinematicEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("KinematicEngine");
	pybind11::class_<KinematicEngine, PartialEngine, std::shared_ptr<KinematicEngine>> _classObj(_module, "KinematicEngine", "Abstract engine for applying prescribed displacement.\n\n.. note:: Derived classes should override the ``apply`` with given list of ``ids`` (not ``action`` with :yref:`PartialEngine.ids`), so that they work when combined together; :yref:`velocity<State.vel>` and :yref:`angular velocity<State.angVel>` of all subscribed bodies is reset before the ``apply`` method is called, it should therefore only increment those quantities.");
	_classObj.def(pybind11::init<>());
	_classObj.def("__add__", &CombinedKinematicEngine::fromTwo);
}

void TranslationEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("TranslationEngine");
	pybind11::class_<TranslationEngine, KinematicEngine, std::shared_ptr<TranslationEngine>> _classObj(_module, "TranslationEngine", "This engine is the base class for different engines, which require any kind of motion.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("velocity", &TranslationEngine::velocity, "Velocity [m/s]");
	_classObj.def_readwrite("translationAxis", &TranslationEngine::translationAxis, "Direction [Vector3]");
}

void HarmonicMotionEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("HarmonicMotionEngine");
	pybind11::class_<HarmonicMotionEngine, KinematicEngine, std::shared_ptr<HarmonicMotionEngine>> _classObj(_module, "HarmonicMotionEngine", "This engine implements the harmonic oscillation of bodies. http://en.wikipedia.org/wiki/Simple_harmonic_motion#Dynamics_of_simple_harmonic_motion");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("A", &HarmonicMotionEngine::A, "Amplitude [m]");
	_classObj.def_readwrite("f", &HarmonicMotionEngine::f, "Frequency [hertz]");
	_classObj.def_readwrite("fi", &HarmonicMotionEngine::fi, "Initial phase [radians]. By default, the body oscillates around initial position.");
}

void RotationEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("RotationEngine");
	pybind11::class_<RotationEngine, KinematicEngine, std::shared_ptr<RotationEngine>> _classObj(_module, "RotationEngine", "Engine applying rotation (by setting angular velocity) to subscribed bodies. If :yref:`rotateAroundZero<RotationEngine.rotateAroundZero>` is set, then each body is also displaced around :yref:`zeroPoint<RotationEngine.zeroPoint>`.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("angularVelocity", &RotationEngine::angularVelocity, "Angular velocity. [rad/s]");
	_classObj.def_readwrite("rotationAxis", &RotationEngine::rotationAxis, "Axis of rotation (direction); will be normalized automatically.");
	_classObj.def_readwrite("rotateAroundZero", &RotationEngine::rotateAroundZero, "If True, bodies will not rotate around their centroids, but rather around ``zeroPoint``.");
	_classObj.def_readwrite("zeroPoint", &RotationEngine::zeroPoint, "Point around which bodies will rotate if ``rotateAroundZero`` is True");
}

void HelixEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("HelixEngine");
	pybind11::class_<HelixEngine, RotationEngine, std::shared_ptr<HelixEngine>> _classObj(_module, "HelixEngine", "Engine applying both rotation and translation, along the same axis, whence the name HelixEngine");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("linearVelocity", &HelixEngine::linearVelocity, "Linear velocity [m/s]");
	_classObj.def_readwrite("angleTurned", &HelixEngine::angleTurned, "How much have we turned so far. [rad]");
}

void InterpolatingHelixEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("InterpolatingHelixEngine");
	pybind11::class_<InterpolatingHelixEngine, HelixEngine, std::shared_ptr<InterpolatingHelixEngine>> _classObj(_module, "InterpolatingHelixEngine", "Engine applying spiral motion, finding current angular velocity by linearly interpolating in times and velocities and translation by using slope parameter.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("times", &InterpolatingHelixEngine::times, "List of time points at which velocities are given; must be increasing [s]");
	_classObj.def_readwrite("angularVelocities", &InterpolatingHelixEngine::angularVelocities, "List of angular velocities; manadatorily of same length as times. [rad/s]");
	_classObj.def_readwrite("wrap", &InterpolatingHelixEngine::wrap, "Wrap t if t>times_n, i.e. t_wrapped=t-N*(times_n-times_0)");
	_classObj.def_readwrite("slope", &InterpolatingHelixEngine::slope, "Axial translation per radian turn (can be negative) [m/rad]");
}

void HarmonicRotationEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("HarmonicRotationEngine");
	pybind11::class_<HarmonicRotationEngine, RotationEngine, std::shared_ptr<HarmonicRotationEngine>> _classObj(_module, "HarmonicRotationEngine", "This engine implements the harmonic-rotation oscillation of bodies.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("A", &HarmonicRotationEngine::A, "Amplitude [rad]");
	_classObj.def_readwrite("f", &HarmonicRotationEngine::f, "Frequency [hertz]");
	_classObj.def_readwrite("fi", &HarmonicRotationEngine::fi, "Initial phase [radians]. By default, the body oscillates around initial position.");
}

void ServoPIDController::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ServoPIDController");
	pybind11::class_<ServoPIDController, TranslationEngine, std::shared_ptr<ServoPIDController>> _classObj(_module, "ServoPIDController", "PIDController servo-engine for applying prescribed force on bodies. http://en.wikipedia.org/wiki/PID_controller");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("maxVelocity", &ServoPIDController::maxVelocity, "Velocity [m/s]");
	_classObj.def_readwrite("axis", &ServoPIDController::axis, "Unit vector along which apply the velocity [-]");
	_classObj.def_readwrite("target", &ServoPIDController::target, "Target value for the controller [N]");
	_classObj.def_readwrite("current", &ServoPIDController::current, "Current value for the controller [N]");
	_classObj.def_readwrite("kP", &ServoPIDController::kP, "Proportional gain/coefficient for the PID-controller [-]");
	_classObj.def_readwrite("kI", &ServoPIDController::kI, "Integral gain/coefficient for the PID-controller [-]");
	_classObj.def_readwrite("kD", &ServoPIDController::kD, "Derivative gain/coefficient for the PID-controller [-]");
	_classObj.def_readwrite("iTerm", &ServoPIDController::iTerm, "Integral term [N]");
	_classObj.def_readwrite("curVel", &ServoPIDController::curVel, "Current applied velocity [m/s]");
	_classObj.def_readwrite("errorCur", &ServoPIDController::errorCur, "Current error [N]");
	_classObj.def_readwrite("errorPrev", &ServoPIDController::errorPrev, "Previous error [N]");
	_classObj.def_readwrite("iterPeriod", &ServoPIDController::iterPeriod, "Periodicity criterion of velocity correlation [-]");
	_classObj.def_readwrite("iterPrevStart", &ServoPIDController::iterPrevStart, "Previous iteration of velocity correlation [-]");
}

void BicyclePedalEngine::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("BicyclePedalEngine");
	pybind11::class_<BicyclePedalEngine, KinematicEngine, std::shared_ptr<BicyclePedalEngine>> _classObj(_module, "BicyclePedalEngine", "Engine applying the linear motion of ``bicycle pedal`` e.g. moving points around the axis without rotation");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("angularVelocity", &BicyclePedalEngine::angularVelocity, "Angular velocity. [rad/s]");
	_classObj.def_readwrite("rotationAxis", &BicyclePedalEngine::rotationAxis, "Axis of rotation (direction); will be normalized automatically.");
	_classObj.def_readwrite("radius", &BicyclePedalEngine::radius, "Rotation radius. [m]");
	_classObj.def_readwrite("fi", &BicyclePedalEngine::fi, "Initial phase [radians]");
}

CREATE_LOGGER(KinematicEngine);

void KinematicEngine::action(){
	if (ids.size()>0) {
		for(Body::id_t id : ids){
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(b) b->state->vel=b->state->angVel=Vector3r::Zero();
		}
		apply(ids);
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}

void CombinedKinematicEngine::action(){
	if (ids.size()>0) {
		// reset first
		for(Body::id_t id : ids){
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(b) b->state->vel=b->state->angVel=Vector3r::Zero();
		}
		// apply one engine after another
		for(const shared_ptr<KinematicEngine>& e :  comb){
			if (e->dead) continue;
			e->scene=scene; e->apply(ids);
		}
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}

const shared_ptr<CombinedKinematicEngine> CombinedKinematicEngine::fromTwo(const shared_ptr<KinematicEngine>& first, const shared_ptr<KinematicEngine>& second){
	shared_ptr<CombinedKinematicEngine> ret(new CombinedKinematicEngine);
	ret->ids=first->ids; ret->comb.push_back(first); ret->comb.push_back(second);
	return ret;
}


void TranslationEngine::apply(const vector<Body::id_t>& ids){
	if (ids.size()>0) {
		#ifdef SUDODEM_OPENMP
		const long size=ids.size();
		#pragma omp parallel for schedule(static)
		for(long i=0; i<size; i++){
			const Body::id_t& id=ids[i];
		#else
		for(Body::id_t id : ids){
		#endif
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(!b) continue;
			b->state->vel+=velocity*translationAxis;
		}
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}

void HarmonicMotionEngine::apply(const vector<Body::id_t>& ids){
	if (ids.size()>0) {
		Vector3r w = f*2.0*Mathr::PI; 										 								//Angular frequency
		Vector3r velocity = (((w*scene->time + fi).array().sin())*(-1.0));
    velocity = velocity.cwiseProduct(A);
    velocity = velocity.cwiseProduct(w);
		for(Body::id_t id : ids){
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(!b) continue;
			b->state->vel+=velocity;
		}
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}


void InterpolatingHelixEngine::apply(const vector<Body::id_t>& ids){
	Real virtTime=wrap ? Shop::periodicWrap(scene->time,*times.begin(),*times.rbegin()) : scene->time;
	angularVelocity=linearInterpolate<Real,Real>(virtTime,times,angularVelocities,_pos);
	linearVelocity=angularVelocity*slope;
	HelixEngine::apply(ids);
}

void HelixEngine::apply(const vector<Body::id_t>& ids){
	if (ids.size()>0) {
		const Real& dt=scene->dt;
		angleTurned+=angularVelocity*dt;
		shared_ptr<BodyContainer> bodies = scene->bodies;
		for(Body::id_t id : ids){
			assert(id<(Body::id_t)bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(!b) continue;
			b->state->vel+=linearVelocity*rotationAxis;
		}
		rotateAroundZero=true;
		RotationEngine::apply(ids);
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}

void RotationEngine::apply(const vector<Body::id_t>& ids){
	if (ids.size()>0) {
		#ifdef SUDODEM_OPENMP
		const long size=ids.size();
		#pragma omp parallel for schedule(static)
		for(long i=0; i<size; i++){
			const Body::id_t& id=ids[i];
		#else
		for(Body::id_t id : ids){
		#endif
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(!b) continue;
			b->state->angVel+=rotationAxis*angularVelocity;
			if(rotateAroundZero){
				const Vector3r l=b->state->pos-zeroPoint;
				Quaternionr q(AngleAxisr(angularVelocity*scene->dt,rotationAxis));
				Vector3r newPos=q*l+zeroPoint;
				b->state->vel+=Vector3r(newPos-b->state->pos)/scene->dt;
			}
		}
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}

void HarmonicRotationEngine::apply(const vector<Body::id_t>& ids){
	const Real& time=scene->time;
	Real w = f*2.0*Mathr::PI; 			//Angular frequency
	angularVelocity = -1.0*A*w*sin(w*time + fi);
	RotationEngine::apply(ids);
}

void ServoPIDController::apply(const vector<Body::id_t>& ids){

  if (iterPrevStart < 0 || ((scene->iter-iterPrevStart)>=iterPeriod)) {

    Vector3r tmpForce = Vector3r::Zero();

    if (ids.size()>0) {
      for(Body::id_t id : ids){
        assert(id<(Body::id_t)scene->bodies->size());
        tmpForce += scene->forces.getForce(id);
      }
    } else {
      LOG_WARN("The list of ids is empty!");
    }

    axis.normalize();
    tmpForce = tmpForce.cwiseProduct(axis);   // Take into account given axis
    errorCur = tmpForce.norm() - target;      // Find error

    const Real pTerm = errorCur*kP;               // Calculate proportional term
    iTerm += errorCur*kI;                         // Calculate integral term
    const Real dTerm = (errorCur-errorPrev)*kD;   // Calculate derivative term

    errorPrev = errorCur;                         // Save the current value of the error

    curVel = (pTerm + iTerm + dTerm);             // Calculate current velocity

    if (std::abs(curVel) > std::abs(maxVelocity)) {
      curVel*=std::abs(maxVelocity)/std::abs(curVel);
    }

    iterPrevStart = scene->iter;
    current = tmpForce;
  }

  translationAxis = axis;
  velocity = curVel;

  TranslationEngine::apply(ids);
}


void BicyclePedalEngine::apply(const vector<Body::id_t>& ids){
	if (ids.size()>0) {
		Quaternionr qRotateZVec(Quaternionr().setFromTwoVectors(Vector3r(0,0,1), rotationAxis));

		Vector3r newPos = Vector3r(cos(fi + angularVelocity*scene->dt)*radius, sin(fi + angularVelocity*scene->dt)*radius, 0.0);
		Vector3r oldPos = Vector3r(cos(fi)*radius, sin(fi)*radius, 0.0);

		Vector3r newVel = (oldPos - newPos)/scene->dt;

		fi += angularVelocity*scene->dt;
		newVel =  qRotateZVec*newVel;

		#ifdef SUDODEM_OPENMP
		const long size=ids.size();
		#pragma omp parallel for schedule(static)
		for(long i=0; i<size; i++){
			const Body::id_t& id=ids[i];
		#else
		for(Body::id_t id : ids){
		#endif
			assert(id<(Body::id_t)scene->bodies->size());
			Body* b=Body::byId(id,scene).get();
			if(!b) continue;
			b->state->vel+=newVel;
		}
	} else {
		LOG_WARN("The list of ids is empty! Can't move any body.");
	}
}