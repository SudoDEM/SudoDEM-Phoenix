/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/lib/multimethods/Indexable.hpp>
#include<sudodem/core/Dispatcher.hpp>

// #define BV_FUNCTOR_CACHE  // Disabled to fix scene pointer issue in multi-threaded context

class BoundFunctor;

class Shape: public Serializable, public Indexable {
	public:
		~Shape() {}; // vtable
		#ifdef BV_FUNCTOR_CACHE
			shared_ptr<BoundFunctor> boundFunctor;
		#endif

		// Attribute declarations for serialization
		Vector3r color; bool wire; bool highlight;

		Shape() : color(Vector3r(1,1,1)), wire(false), highlight(false) {}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(color));
			ar(CEREAL_NVP(wire));
			ar(CEREAL_NVP(highlight));
		}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("Shape");
			pybind11::class_<Shape, Serializable, std::shared_ptr<Shape>> _classObj(_module, "Shape", "Geometry of a body");
			_classObj.def(pybind11::init<>());
			// Register attributes
			_classObj.def_readwrite("color", &Shape::color, "Color for rendering (normalized RGB).");
			_classObj.def_readwrite("wire", &Shape::wire, "Whether this Shape is rendered using color surfaces, or only wireframe (can still be overridden by global config of the renderer).");
			_classObj.def_readwrite("highlight", &Shape::highlight, "Whether this Shape will be highlighted when rendered.");
			// Python-specific properties
			_classObj.def_property_readonly("dispIndex", [](std::shared_ptr<Shape> s){ return Indexable_getClassIndex(s); }, "Return class index of this instance.");
			_classObj.def("dispHierarchy", [](std::shared_ptr<Shape> s, bool names=true){ return Indexable_getClassIndices(s, names); }, pybind11::arg("names")=true, "Return list of dispatch classes (from down upwards), starting with the class instance itself, top-level indexable at last. If names is true (default), return class names rather than numerical indices.");
		}
	REGISTER_CLASS_NAME_DERIVED(Shape);
REGISTER_INDEX_COUNTER_H(Shape)
};
REGISTER_SERIALIZABLE(Shape);

