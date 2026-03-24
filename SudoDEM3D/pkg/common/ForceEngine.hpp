// 2004 © Janek Kozicki <cosurgi@berlios.de>
// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
// 2014 © Raphael Maurin <raphael.maurin@irstea.fr>

#pragma once

#include<sudodem/core/PartialEngine.hpp>

class ForceEngine: public PartialEngine{
	public:
		virtual void action() override;
		Vector3r force;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ForceEngine, PartialEngine);

/* Engine for applying force of varying magnitude but constant direction
 * on subscribed bodies. times and magnitudes must have the same length,
 * direction (normalized automatically) gives the orientation.
 *
 * As usual with interpolating engines: the first magnitude is used before the first
 * time point, last magnitude is used after the last time point. Wrap specifies whether
 * time wraps around the last time point to the first time point.
 */
class InterpolatingDirectedForceEngine: public ForceEngine{
	size_t _pos;
	public:
		virtual void action() override;
		vector<Real> times;
		vector<Real> magnitudes;
		Vector3r direction;
		bool wrap;
		
		InterpolatingDirectedForceEngine() : _pos(0), wrap(false), direction(1.0, 0, 0) {}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(InterpolatingDirectedForceEngine, ForceEngine);

struct RadialForceEngine: public PartialEngine{
	virtual void action() override;
	virtual void postLoad(RadialForceEngine&);
	Vector3r axisPt;
	Vector3r axisDir;
	Real fNorm;
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(RadialForceEngine, ForceEngine);

class DragEngine: public PartialEngine{
	public:
		virtual void action() override;
		Real Rho;
		Real Cd;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(DragEngine, ForceEngine);

class LinearDragEngine: public PartialEngine{
	public:
		virtual void action() override;
		Real nu;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(LinearDragEngine, DragEngine);


class HydroForceEngine: public PartialEngine{
	private:
		vector<Vector3r> vFluct;
	public:
		virtual void action() override;
		Real rhoFluid;
		Real viscoDyn;
		Real zRef;
		Real nCell;
		Real deltaZ;
		Real expoRZ;
		bool lift;
		Real Cl;
		Vector3r gravity;
		vector<Real> vxFluid;
		vector<Real> phiPart;
		bool velFluct;
		vector<Real> simplifiedReynoldStresses;
		Real bedElevation;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(HydroForceEngine, ForceEngine);
