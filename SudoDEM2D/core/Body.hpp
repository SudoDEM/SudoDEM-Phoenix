/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once

#include <sudodem/lib/base/Math.hpp>//zhswee, fix _POXIC_C_SOURCE warning
#include <sudodem/core/Shape.hpp>
#include <sudodem/core/Bound.hpp>
#include <sudodem/core/State.hpp>
#include <sudodem/core/Material.hpp>

//#include<sudodem/lib/base/Math.hpp>
#include <sudodem/lib/serialization/Serializable.hpp>
#include <sudodem/lib/multimethods/Indexable.hpp>




class Scene;
class Interaction;

class Body: public Serializable{
	public:
		// numerical types for storing ids
		typedef int id_t;
		// internal structure to hold some interaction of a body; used by InteractionContainer;
		typedef std::map<Body::id_t, shared_ptr<Interaction> > MapId2IntrT;
		// groupMask type

		// bits for Body::flags
		enum { FLAG_BOUNDED=1, FLAG_ASPHERICAL=2 }; /* add powers of 2 as needed */
		//! symbolic constant for body that doesn't exist.
		inline static constexpr id_t ID_NONE = -1;
		//! get Body pointer given its id.
		static const shared_ptr<Body>& byId(Body::id_t _id,Scene* rb=NULL);
		static const shared_ptr<Body>& byId(Body::id_t _id,shared_ptr<Scene> rb);


		//! Whether this Body is a Clump.
		//! @note The following is always true: \code (Body::isClump() XOR Body::isClumpMember() XOR Body::isStandalone()) \endcode
		bool isClump() const {return clumpId!=ID_NONE && id==clumpId;}
		//! Whether this Body is member of a Clump.
		bool isClumpMember() const {return clumpId!=ID_NONE && id!=clumpId;}
		//! Whether this body is standalone (neither Clump, nor member of a Clump)
		bool isStandalone() const {return clumpId==ID_NONE;}

		//! Whether this body has all DOFs blocked
		// inline accessors
		// logic: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c
		bool isDynamic() const { assert(state); return state->blockedDOFs!=State::DOF_ALL; }
		void setDynamic(bool d){ assert(state); if(d){ state->blockedDOFs=State::DOF_NONE; } else { state->blockedDOFs=State::DOF_ALL; state->vel=Vector2r::Zero();state->angVel=0.0;} }
		bool isBounded() const {return flags & FLAG_BOUNDED; }
		void setBounded(bool d){ if(d) flags|=FLAG_BOUNDED; else flags&=~(FLAG_BOUNDED); }
		bool isAspherical() const {return flags & FLAG_ASPHERICAL; }
		void setAspherical(bool d){ if(d) flags|=FLAG_ASPHERICAL; else flags&=~(FLAG_ASPHERICAL); }

		/*! Hook for clump to update position of members when user-forced reposition and redraw (through GUI) occurs.
		 * This is useful only in cases when engines that do that in every iteration are not active - i.e. when the simulation is paused.
		 * (otherwise, GLViewer would depend on Clump and therefore Clump would have to go to core...) */
		virtual void userForcedDisplacementRedrawHook(){return;}

		pybind11::list py_intrs();

		Body::id_t getId() const {return id;};
		unsigned int coordNumber();  // Number of neighboring particles

		mask_t getGroupMask() const {return groupMask; };
		bool maskOk(int mask) const;
		bool maskCompatible(int mask) const;
#ifdef SUDODEM_MASK_ARBITRARY
		bool maskOk(const mask_t& mask) const;
		bool maskCompatible(const mask_t& mask) const;
#endif

		// only BodyContainer can set the id of a body
		friend class BodyContainer;

		// Attribute declarations for serialization
		Body::id_t id; mask_t groupMask; int flags; shared_ptr<Material> material; shared_ptr<State> state; shared_ptr<Shape> shape; shared_ptr<Bound> bound; MapId2IntrT intrs; int clumpId; long chain; long iterBorn; Real timeBorn;
#ifdef SUDODEM_SPH
		Real rho; Real rho0; Real press; Real Cs;
#endif
#ifdef SUDODEM_LIQMIGRATION
		Real Vf; Real Vmin;
#endif

		Body() : id(Body::ID_NONE), groupMask(1), flags(FLAG_BOUNDED), state(new State), clumpId(Body::ID_NONE), chain(-1), iterBorn(-1), timeBorn(-1)
#ifdef SUDODEM_SPH
			, rho(-1.0), rho0(-1.0), press(0.0), Cs(0.0)
#endif
#ifdef SUDODEM_LIQMIGRATION
			, Vf(0.0), Vmin(0.0)
#endif
		{}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(id));
			ar(CEREAL_NVP(groupMask));
			ar(CEREAL_NVP(flags));
			ar(CEREAL_NVP(material));
			ar(CEREAL_NVP(state));
			ar(CEREAL_NVP(shape));
			ar(CEREAL_NVP(bound));
			ar(CEREAL_NVP(clumpId));
			ar(CEREAL_NVP(chain));
			ar(CEREAL_NVP(iterBorn));
			ar(CEREAL_NVP(timeBorn));
#ifdef SUDODEM_SPH
			ar(CEREAL_NVP(rho));
			ar(CEREAL_NVP(rho0));
			ar(CEREAL_NVP(press));
			ar(CEREAL_NVP(Cs));
#endif
#ifdef SUDODEM_LIQMIGRATION
			ar(CEREAL_NVP(Vf));
			ar(CEREAL_NVP(Vmin));
#endif
		}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE(Body);
