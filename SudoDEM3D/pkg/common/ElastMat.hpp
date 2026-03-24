// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/Material.hpp>

/*! Elastic material */
class ElastMat: public Material{
	public:
	virtual ~ElastMat() {};
	Real Kn;
	Real Ks;
	Real young;  // Young's modulus
	Real poisson; // Poisson's ratio
	
	ElastMat() : Kn(1e5), Ks(0.7e5), young(1e8), poisson(0.3) { createIndex(); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(ElastMat,Material)
};
REGISTER_SERIALIZABLE_BASE(ElastMat, Material);

/*! Granular material */
class FrictMat: public ElastMat{
	public:
	virtual ~FrictMat() {};
	Real frictionAngle;
	
	FrictMat() : frictionAngle(0.5) { createIndex(); }
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(FrictMat,ElastMat)
};
REGISTER_SERIALIZABLE_BASE(FrictMat, ElastMat);
