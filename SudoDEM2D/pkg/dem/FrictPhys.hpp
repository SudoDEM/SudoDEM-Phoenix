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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<FrictPhys, NormShearPhys, std::shared_ptr<FrictPhys>> _classObj(_module, "FrictPhys", "The simple linear elastic-plastic interaction with friction angle, like in the traditional [CundallStrack1979]_");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("tangensOfFrictionAngle", &FrictPhys::tangensOfFrictionAngle, "tan of angle of friction");
		}
	REGISTER_CLASS_INDEX(FrictPhys,NormShearPhys);
};
REGISTER_SERIALIZABLE_BASE(FrictPhys, NormShearPhys);

class ViscoFrictPhys: public FrictPhys
{
	public :
	virtual ~ViscoFrictPhys() {};
	Vector2r creepedShear;
	
	ViscoFrictPhys() : creepedShear(Vector2r(0,0)) { createIndex(); }
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(FrictPhys, creepedShear);
	
	virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<ViscoFrictPhys, FrictPhys, std::shared_ptr<ViscoFrictPhys>> _classObj(_module, "ViscoFrictPhys", "Temporary version of :yref:`FrictPhys` for compatibility with e.g. :yref:`Law2_ScGeom6D_NormalInelasticityPhys_NormalInelasticity`");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("creepedShear", &ViscoFrictPhys::creepedShear, "Creeped force (parallel)");
	}
	REGISTER_CLASS_INDEX(ViscoFrictPhys,FrictPhys);
};
REGISTER_SERIALIZABLE_BASE(ViscoFrictPhys, FrictPhys);

// The following code was moved from Ip2_FrictMat_FrictMat_FrictPhys.hpp

class Ip2_FrictMat_FrictMat_FrictPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction) override;
		shared_ptr<MatchMaker> frictAngle;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ip2_FrictMat_FrictMat_FrictPhys, IPhysFunctor, std::shared_ptr<Ip2_FrictMat_FrictMat_FrictPhys>> _classObj(_module, "Ip2_FrictMat_FrictMat_FrictPhys", "Create a :yref:`FrictPhys` from two :yref:`FrictMats<FrictMat>`. The compliance of one disk under point load is defined here as $1/(E.D)$, with $E$ the stiffness of the disk and $D$ its diameter. The compliance of the contact itself will be the sum of compliances from each disk, i.e. $1/(E_1.D_1)+1/(E_2.D_2)$ in the general case, or $2/(E.D)$ in the special case of equal sizes and equal stiffness. Note that summing compliances corresponds to an harmonic average of stiffnesss (as in e.g. [Scholtes2009a]_), which is how kn is actually computed in the :yref:`Ip2_FrictMat_FrictMat_FrictPhys` functor:\n\n $k_n = \\frac{E_1D_1*E_2D_2}{E_1D_1+E_2D_2}=\\frac{k_1*k_2}{k_1+k_2}$, with $k_i=E_iD_i$.\n\n The shear stiffness ks of one disk is defined via the material parameter :yref:`ElastMat::poisson`, as ks=poisson*kn, and the resulting shear stiffness of the interaction will be also an harmonic average. In the case of a contact between a :yref:`ViscElMat` and a :yref:`FrictMat`, be sure to set :yref:`FrictMat::young` and :yref:`FrictMat::poisson`, otherwise the default value will be used.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("frictAngle", &Ip2_FrictMat_FrictMat_FrictPhys::frictAngle, "Instance of :yref:`MatchMaker` determining how to compute interaction's friction angle. If ``None``, minimum value is used.");
		}
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_FrictPhys, IPhysFunctor);

class Ip2_FrictMat_FrictMat_ViscoFrictPhys: public Ip2_FrictMat_FrictMat_FrictPhys{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction) override;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ip2_FrictMat_FrictMat_ViscoFrictPhys, Ip2_FrictMat_FrictMat_FrictPhys, std::shared_ptr<Ip2_FrictMat_FrictMat_ViscoFrictPhys>> _classObj(_module, "Ip2_FrictMat_FrictMat_ViscoFrictPhys", "Create a :yref:`FrictPhys` from two :yref:`FrictMats<FrictMat>`. The compliance of one disk under symetric point loads is defined here as 1/(E.r), with E the stiffness of the disk and r its radius, and corresponds to a compliance 1/(2.E.r)=1/(E.D) from each contact point. The compliance of the contact itself will be the sum of compliances from each disk, i.e. 1/(E.D1)+1/(E.D2) in the general case, or 1/(E.r) in the special case of equal sizes. Note that summing compliances corresponds to an harmonic average of stiffnesss, which is how kn is actually computed in the :yref:`Ip2_FrictMat_FrictMat_FrictPhys` functor.\n\nThe shear stiffness ks of one disk is defined via the material parameter :yref:`ElastMat::poisson`, as ks=poisson*kn, and the resulting shear stiffness of the interaction will be also an harmonic average.");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Ip2_FrictMat_FrictMat_ViscoFrictPhys, IPhysFunctor);
