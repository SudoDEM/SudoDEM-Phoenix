#pragma once
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/core/PartialEngine.hpp>



struct KinematicEngine;

struct CombinedKinematicEngine: public PartialEngine{
	vector<shared_ptr<KinematicEngine>> comb;
	
	virtual void action() override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	static const shared_ptr<CombinedKinematicEngine> appendOne(const shared_ptr<CombinedKinematicEngine>& self, const shared_ptr<KinematicEngine>& other){ self->comb.push_back(other); return self; }
	static const shared_ptr<CombinedKinematicEngine> fromTwo(const shared_ptr<KinematicEngine>& first, const shared_ptr<KinematicEngine>& second);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(CombinedKinematicEngine, PartialEngine);

struct KinematicEngine: public PartialEngine{
	virtual void action() override;
	virtual void apply(const vector<Body::id_t>& ids){ LOG_ERROR("KinematicEngine::apply called, derived class ("<<getClassName()<<") did not override that method?"); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(KinematicEngine, PartialEngine);


struct TranslationEngine: public KinematicEngine{
	Real velocity;
	Vector3r translationAxis;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	void postLoad(TranslationEngine&){ translationAxis.normalize(); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(TranslationEngine, KinematicEngine);

struct HarmonicMotionEngine: public KinematicEngine{
	Vector3r A;
	Vector3r f;
	Vector3r fi;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(HarmonicMotionEngine, KinematicEngine);

struct RotationEngine: public KinematicEngine{
	Real angularVelocity;
	Vector3r rotationAxis;
	bool rotateAroundZero;
	Vector3r zeroPoint;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	void postLoad(RotationEngine&){ rotationAxis.normalize(); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(RotationEngine, KinematicEngine);

struct HelixEngine:public RotationEngine{
	Real linearVelocity;
	Real angleTurned;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(HelixEngine, RotationEngine);

struct InterpolatingHelixEngine: public HelixEngine{
	vector<Real> times;
	vector<Real> angularVelocities;
	bool wrap;
	Real slope;
	size_t _pos;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(InterpolatingHelixEngine, HelixEngine);

struct HarmonicRotationEngine: public RotationEngine{
	Real A;
	Real f;
	Real fi;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(HarmonicRotationEngine, RotationEngine);

struct ServoPIDController: public TranslationEngine{
	Real maxVelocity;
	Vector3r axis;
	Real target;
	Vector3r current;
	Real kP;
	Real kI;
	Real kD;
	Real iTerm;
	Real curVel;
	Real errorCur;
	Real errorPrev;
	long iterPeriod;
	long iterPrevStart;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(ServoPIDController, TranslationEngine);

struct BicyclePedalEngine: public KinematicEngine{
	Real angularVelocity;
	Vector3r rotationAxis;
	Real radius;
	Real fi;
	
	virtual void apply(const vector<Body::id_t>& ids) override;
	void postLoad(BicyclePedalEngine&){ rotationAxis.normalize(); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(BicyclePedalEngine, KinematicEngine);

