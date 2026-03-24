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
#include<pybind11/pybind11.h>

class IGeom : public Serializable, public Indexable
{
	REGISTER_INDEX_COUNTER_H(IGeom)
public:
	// Cereal serialization
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int version) {
		ar(cereal::base_class<Serializable>(this));
	}

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		checkPyClassRegistersItself("IGeom");
		pybind11::class_<IGeom, Serializable, std::shared_ptr<IGeom>> _classObj(_module, "IGeom", "Geometrical configuration of interaction");
		_classObj.def(pybind11::init<>());
		_classObj.def_property_readonly("dispIndex", [](std::shared_ptr<IGeom> g){ return Indexable_getClassIndex(g); }, "Return class index of this instance.");
		_classObj.def("dispHierarchy", &Indexable_getClassIndices<IGeom>, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
	}
};
REGISTER_SERIALIZABLE(IGeom);