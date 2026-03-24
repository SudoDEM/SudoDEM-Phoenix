// 2004 © Janek Kozicki <cosurgi@berlios.de>
// 2007,2008 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/pkg/common/FieldApplier.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/Body.hpp>

/*! Homogeneous gravity field; applies gravity×mass force on all bodies. */
class GravityEngine: public FieldApplier{
	public:
		virtual void action() override;
		Vector3r gravity;
		int gravPotIx;
		int mask;
		bool warnOnce;
		
		DECLARE_LOGGER;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GravityEngine, GlobalEngine);


/*! Engine attracting all bodies towards a central body (doesn't depend on distance);
 *
 * @todo This code has not been yet tested at all.
 */
class CentralGravityEngine: public FieldApplier {
	public:
		virtual void action() override;
		Body::id_t centralBody;
		Real accel;
		bool reciprocal;
		int mask;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(CentralGravityEngine, GravityEngine);

/*! Apply acceleration (independent of distance) directed towards an axis.
 *
 */
class AxialGravityEngine: public FieldApplier {
	public:
	virtual void action() override;
	Vector3r axisPoint;
	Vector3r axisDirection;
	Real acceleration;
	int mask;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(AxialGravityEngine, GravityEngine);
