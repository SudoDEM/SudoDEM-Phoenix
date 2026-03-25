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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		
	REGISTER_CLASS_NAME_DERIVED(Shape);
REGISTER_INDEX_COUNTER_H(Shape)
};
REGISTER_SERIALIZABLE(Shape);

