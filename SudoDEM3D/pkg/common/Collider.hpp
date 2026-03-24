/*************************************************************************
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/core/Bound.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/GlobalEngine.hpp>

#include<sudodem/pkg/common/Dispatching.hpp>

class Collider: public GlobalEngine {
	public:
		static int avoidSelfInteractionMask;
		shared_ptr<BoundDispatcher> boundDispatcher = std::make_shared<BoundDispatcher>();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlobalEngine, boundDispatcher);
		
		/*! Probe the Bound on a bodies presense. Returns list of body ids with which there is potential overlap. */
		virtual  vector<Body::id_t> probeBoundingVolume(const Bound&){throw;}
		/*! Tell whether given bodies may interact, for other than spatial reasons.
		 *
		 * Concrete collider implementations should call this function if
		 * the bodies are in potential interaction geometrically. */
		static bool mayCollide(const Body*, const Body*);
		/*! Invalidate all persistent data (if the collider has any), forcing reinitialization at next run.
		The default implementation does nothing, colliders should override it if it is applicable.

		Currently used from Shop::flipCell, which changes cell information for bodies.
		*/
		virtual void invalidatePersistentData(){}

		// ctor with functors for the integrated BoundDispatcher
		virtual void pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d) override;

		// backwards-compatibility func, can be removed later
		void findBoundDispatcherInEnginesIfNoFunctorsAndWarn();

		int get_avoidSelfInteractionMask(){return avoidSelfInteractionMask;}
		void set_avoidSelfInteractionMask(const int &v){avoidSelfInteractionMask = v;}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Collider, GlobalEngine);