// 2008 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Scene.hpp>

class StepDisplacer: public PartialEngine {
	public:
		Vector2r mov;
		Rotationr rot;
		bool setVelocities;
		
		virtual void action() override {
			for(Body::id_t id : ids){
				const shared_ptr<Body>& b=Body::byId(id,scene);
				if(setVelocities){
					const Real& dt=scene->dt;
					b->state->vel=mov/dt;
					b->state->angVel=rot.angle()/dt;
				}
				if(!setVelocities){
					b->state->pos+=mov;
					b->state->ori=rot*b->state->ori;
				}
			}
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<StepDisplacer, PartialEngine, std::shared_ptr<StepDisplacer>> _classObj(_module, "StepDisplacer", "Apply generalized displacement (displacement or rotation) stepwise on subscribed bodies. Could be used for purposes of contact law tests (by moving one disk compared to another), but in this case, see rather :yref:`LawTester`");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("mov", &StepDisplacer::mov, "Linear displacement step to be applied per iteration, by addition to :yref:`State.pos`.");
			_classObj.def_readwrite("rot", &StepDisplacer::rot, "Rotation step to be applied per iteration (via rotation composition with :yref:`State.ori`).");
			_classObj.def_readwrite("setVelocities", &StepDisplacer::setVelocities, "If false, positions and orientations are directly updated, without changing the speeds of concerned bodies. If true, only velocity and angularVelocity are modified. In this second case :yref:`integrator<NewtonIntegrator>` is supposed to be used, so that, thanks to this Engine, the bodies will have the prescribed jump over one iteration (dt).");
		}
		DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(StepDisplacer, PartialEngine);
