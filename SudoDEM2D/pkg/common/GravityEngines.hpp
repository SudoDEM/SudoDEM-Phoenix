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
		Vector2r gravity;
		int gravPotIx;
		int mask;
		bool warnOnce;
		
		DECLARE_LOGGER;
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<GravityEngine, FieldApplier, std::shared_ptr<GravityEngine>> _classObj(_module, "GravityEngine", "Engine applying constant acceleration to all bodies. DEPRECATED, use :yref:`Newton::gravity` unless you need energy tracking or selective gravity application using groupMask).");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("gravity", &GravityEngine::gravity, "Acceleration [kgms⁻²]");
			_classObj.def_readwrite("gravPotIx", &GravityEngine::gravPotIx, "Index for gravPot energy");
			_classObj.def_readwrite("mask", &GravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
			_classObj.def_readwrite("warnOnce", &GravityEngine::warnOnce, "For deprecation warning once.");
			_classObj.def("action", &GravityEngine::action);
		}
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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<CentralGravityEngine, FieldApplier, std::shared_ptr<CentralGravityEngine>> _classObj(_module, "CentralGravityEngine", "Engine applying acceleration to all bodies, towards a central body.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("centralBody", &CentralGravityEngine::centralBody, "The :yref:`body<Body>` towards which all other bodies are attracted.");
			_classObj.def_readwrite("accel", &CentralGravityEngine::accel, "Acceleration magnitude [kgms⁻²]");
			_classObj.def_readwrite("reciprocal", &CentralGravityEngine::reciprocal, "If true, acceleration will be applied on the central body as well.");
			_classObj.def_readwrite("mask", &CentralGravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
			_classObj.def("action", &CentralGravityEngine::action);
		}
};
REGISTER_SERIALIZABLE_BASE(CentralGravityEngine, GravityEngine);

/*! Apply acceleration (independent of distance) directed towards an axis.
 *
 */
class AxialGravityEngine: public FieldApplier {
	public:
	virtual void action() override;
	Vector2r axisPoint;
	Vector2r axisDirection;
	Real acceleration;
	int mask;
	
	virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<AxialGravityEngine, FieldApplier, std::shared_ptr<AxialGravityEngine>> _classObj(_module, "AxialGravityEngine", "Apply acceleration (independent of distance) directed towards an axis.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("axisPoint", &AxialGravityEngine::axisPoint, "Point through which the axis is passing.");
		_classObj.def_readwrite("axisDirection", &AxialGravityEngine::axisDirection, "direction of the gravity axis (will be normalized automatically)");
		_classObj.def_readwrite("acceleration", &AxialGravityEngine::acceleration, "Acceleration magnitude [kgms⁻²]");
		_classObj.def_readwrite("mask", &AxialGravityEngine::mask, "If mask defined, only bodies with corresponding groupMask will be affected by this engine. If 0, all bodies will be affected.");
		_classObj.def("action", &AxialGravityEngine::action);
	}
};
REGISTER_SERIALIZABLE_BASE(AxialGravityEngine, GravityEngine);
