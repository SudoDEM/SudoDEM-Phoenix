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
		int lastUpdateIter; Vector2r refPos; Real sweepLength; Vector3r color; Vector2r min; Vector2r max;

		Bound() : lastUpdateIter(0), refPos(Vector2r::Constant(NaN)), sweepLength(0), color(Vector3r(1,1,1)), min(Vector2r(NaN,NaN)), max(Vector2r(NaN,NaN)) {}

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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_INDEX_COUNTER_H(Bound)
};
REGISTER_SERIALIZABLE(Bound);
