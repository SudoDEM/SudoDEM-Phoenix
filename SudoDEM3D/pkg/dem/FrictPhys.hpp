/*************************************************************************
*  Copyright (C) 2007 by Bruno CHAREYRE                                  *
*  bruno.chareyre@hmg.inpg.fr                                            *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once

#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/pkg/common/MatchMaker.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>

class FrictPhys: public NormShearPhys
{
	public :
		virtual ~FrictPhys() {};
		Real tangensOfFrictionAngle;
		
		FrictPhys() : tangensOfFrictionAngle(NaN) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(NormShearPhys, tangensOfFrictionAngle);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(FrictPhys,NormShearPhys)
};
REGISTER_SERIALIZABLE_BASE(FrictPhys, NormShearPhys);

class ViscoFrictPhys: public FrictPhys
{
	public :
	virtual ~ViscoFrictPhys() {};
	Vector3r creepedShear;
	
	ViscoFrictPhys() : creepedShear(Vector3r::Zero()) { createIndex(); }
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(FrictPhys, creepedShear);
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(ViscoFrictPhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(ViscoFrictPhys, FrictPhys);

// The following code was moved from Ip2_FrictMat_FrictMat_FrictPhys.hpp

class Ip2_FrictMat_FrictMat_FrictPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
		shared_ptr<MatchMaker> frictAngle;
		
		Ip2_FrictMat_FrictMat_FrictPhys() : frictAngle(nullptr) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IPhysFunctor, frictAngle);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_FrictPhys, IPhysFunctor);

class Ip2_FrictMat_FrictMat_ViscoFrictPhys: public Ip2_FrictMat_FrictMat_FrictPhys{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_ViscoFrictPhys, IPhysFunctor);
