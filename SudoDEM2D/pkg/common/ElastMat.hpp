// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/Material.hpp>

/*! Elastic material */
class ElastMat: public Material{
	public:
	virtual ~ElastMat() {};
	Real Kn;
	Real Ks;
	
	ElastMat() : Kn(1e5), Ks(0.7e5) { createIndex(); }
	
	virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<ElastMat, Material, std::shared_ptr<ElastMat>> _classObj(_module, "ElastMat", "Purely elastic material. The material parameters may have different meanings depending on the :yref:`IPhysFunctor` used : true Young and Poisson in :yref:`Ip2_FrictMat_FrictMat_MindlinPhys`, or contact stiffnesses in :yref:`Ip2_FrictMat_FrictMat_FrictPhys`.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("Kn", &ElastMat::Kn, "Contact normal stiffness [N/m].");
		_classObj.def_readwrite("Ks", &ElastMat::Ks, "Contact tangential stiffness [N/m].");
	}
	REGISTER_CLASS_INDEX(ElastMat,Material);
};
REGISTER_SERIALIZABLE_BASE(ElastMat, Material);

/*! Granular material */
class FrictMat: public ElastMat{
	public:
	virtual ~FrictMat() {};
	Real frictionAngle;
	
	FrictMat() : frictionAngle(0.5) { createIndex(); }
	
	virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<FrictMat, ElastMat, std::shared_ptr<FrictMat>> _classObj(_module, "FrictMat", "Elastic material with contact friction. See also :yref:`ElastMat`.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("frictionAngle", &FrictMat::frictionAngle, "Contact friction angle (in radians). Hint : use 'radians(degreesValue)' in python scripts.");
	}
	REGISTER_CLASS_INDEX(FrictMat,ElastMat);
};
REGISTER_SERIALIZABLE_BASE(FrictMat, ElastMat);
