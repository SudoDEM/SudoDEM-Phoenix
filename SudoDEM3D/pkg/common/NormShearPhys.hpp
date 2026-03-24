// © 2007 Janek Kozicki <cosurgi@mail.berlios.de>
// © 2008 Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once

#include<sudodem/core/IPhys.hpp>

class NormPhys:public IPhys {
	public:
		virtual ~NormPhys() {};
		Real kn;
		Vector3r normalForce;
		
		NormPhys() : kn(0), normalForce(Vector3r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhys, kn, normalForce);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(NormPhys,IPhys)
};
REGISTER_SERIALIZABLE_BASE(NormPhys, IPhys);

class NormShearPhys: public NormPhys{
	public:
		virtual ~NormShearPhys() {};
		Real ks;
		Vector3r shearForce;
		
		NormShearPhys() : ks(0), shearForce(Vector3r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(NormPhys, ks, shearForce);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(NormShearPhys,NormPhys)
};
REGISTER_SERIALIZABLE_BASE(NormShearPhys, NormPhys);
