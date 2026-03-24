// © 2007 Janek Kozicki <cosurgi@mail.berlios.de>
// © 2008 Václav Šmilauer <eudoxos@arcig.cz>
#pragma once

#include<sudodem/core/IPhys.hpp>

class NormPhys:public IPhys {
	public:
		virtual ~NormPhys() {};
		Real kn;
		Vector2r normalForce;
		
		NormPhys() : kn(0), normalForce(Vector2r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhys, kn, normalForce);
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<NormPhys, IPhys, std::shared_ptr<NormPhys>> _classObj(_module, "NormPhys", "Abstract class for interactions that have normal stiffness.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("kn", &NormPhys::kn, "Normal stiffness");
			_classObj.def_readwrite("normalForce", &NormPhys::normalForce, "Normal force after previous step (in global coordinates).");
		}
	REGISTER_CLASS_INDEX(NormPhys,IPhys);
};
REGISTER_SERIALIZABLE_BASE(NormPhys, IPhys);

class NormShearPhys: public NormPhys{
	public:
		virtual ~NormShearPhys() {};
		Real ks;
		Vector2r shearForce;
		
		NormShearPhys() : ks(0), shearForce(Vector2r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(NormPhys, ks, shearForce);
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<NormShearPhys, NormPhys, std::shared_ptr<NormShearPhys>> _classObj(_module, "NormShearPhys", "Abstract class for interactions that have shear stiffnesses, in addition to normal stiffness. This class is used in the PFC3d-style stiffness timestepper.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("ks", &NormShearPhys::ks, "Shear stiffness");
			_classObj.def_readwrite("shearForce", &NormShearPhys::shearForce, "Shear force after previous step (in global coordinates).");
		}
	REGISTER_CLASS_INDEX(NormShearPhys,NormPhys);
};
REGISTER_SERIALIZABLE_BASE(NormShearPhys, NormPhys);
