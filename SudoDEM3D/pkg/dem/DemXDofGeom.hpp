// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/IGeom.hpp>

/*! Abstract class that unites ScGeom and L3Geom,
	created for the purposes of GlobalStiffnessTimeStepper.
	It might be removed in the future. */
class GenericSpheresContact: public IGeom{
	
public:
	Vector3r normal;
	Vector3r contactPoint;
	Real refR1;
	Real refR2;
	// Explicit constructor with initial values
	GenericSpheresContact(): refR1(0.), refR2(0.) {
		createIndex();
	}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(IGeom, normal, contactPoint, refR1, refR2);
	
	REGISTER_CLASS_INDEX_H(GenericSpheresContact,IGeom)

	virtual ~GenericSpheresContact() {};

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GenericSpheresContact, IGeom);