/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once

#include<sudodem/lib/base/Math.hpp>
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/lib/multimethods/Indexable.hpp>
#include<sudodem/core/Dispatcher.hpp>

/*! Interface for approximate body locations in space

	Note: the min and max members refer to shear coordinates, in periodic
	and sheared space, not cartesian coordinates in the physical space.

*/

class Bound: public Serializable, public Indexable{
	public:
		// Attribute declarations for serialization
		int lastUpdateIter; Vector3r refPos; Real sweepLength; Vector3r color; Vector3r min; Vector3r max;

		Bound() : lastUpdateIter(0), refPos(Vector3r::Constant(NaN)), sweepLength(0), color(Vector3r(1,1,1)), min(Vector3r::Constant(NaN)), max(Vector3r::Constant(NaN)) {}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(lastUpdateIter));
			ar(CEREAL_NVP(refPos));
			ar(CEREAL_NVP(sweepLength));
			ar(CEREAL_NVP(color));
			ar(CEREAL_NVP(min));
			ar(CEREAL_NVP(max));
		}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("Bound");
			pybind11::class_<Bound, Serializable, std::shared_ptr<Bound>> _classObj(_module, "Bound", "Object bounding part of space taken by associated body; might be larger, used to optimalize collision detection");
			_classObj.def(pybind11::init<>());
			// Register attributes (readonly)
			_classObj.def_readonly("lastUpdateIter", &Bound::lastUpdateIter, "record iteration of last reference position update |yupdate|");
			_classObj.def_readonly("refPos", &Bound::refPos, "Reference position, updated at current body position each time the bound dispatcher update bounds |yupdate|");
			_classObj.def_readonly("sweepLength", &Bound::sweepLength, "The length used to increase the bounding boxe size, can be adjusted on the basis of previous displacement if :yref:`BoundDispatcher::targetInterv`>0. |yupdate|");
			_classObj.def_readwrite("color", &Bound::color, "Color for rendering this object");
			_classObj.def_readonly("min", &Bound::min, "Lower corner of box containing this bound (and the :yref:`Body` as well)");
			_classObj.def_readonly("max", &Bound::max, "Upper corner of box containing this bound (and the :yref:`Body` as well)");
			// Python-specific properties
			_classObj.def_property_readonly("dispIndex", [](std::shared_ptr<Bound> b){ return Indexable_getClassIndex(b); }, "Return class index of this instance.");
			_classObj.def("dispHierarchy", [](std::shared_ptr<Bound> b, bool names=true){ return Indexable_getClassIndices(b, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
		}
	REGISTER_INDEX_COUNTER_H(Bound)
};
REGISTER_SERIALIZABLE(Bound);
