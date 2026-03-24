// 2008 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Scene.hpp>

class StepDisplacer: public PartialEngine {
	public:
		Vector3r mov;
		Quaternionr rot;
		bool setVelocities;
		
		virtual void action() override {
			for(Body::id_t id : ids){
				const shared_ptr<Body>& b=Body::byId(id,scene);
				if(setVelocities){
					const Real& dt=scene->dt;
					b->state->vel=mov/dt;
					// Convert quaternion to angle-axis to get angular velocity
					Eigen::AngleAxis<Real> aa(rot);
					b->state->angVel=aa.axis() * aa.angle() / dt;
				}
				if(!setVelocities){
					b->state->pos+=mov;
					b->state->ori=rot*b->state->ori;
				}
			}
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(StepDisplacer, PartialEngine);
