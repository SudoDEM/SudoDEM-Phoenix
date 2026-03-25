#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>
// Include pybind11 headers for Python bindings
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/IPhys.hpp>
#include<sudodem/core/Body.hpp>
#include<pybind11/pybind11.h>


class IGeomFunctor;
class IPhysFunctor;
class LawFunctor;
class Scene;

class Interaction: public Serializable{
	private:
		friend class IPhysDispatcher;
		friend class InteractionLoop;
	public:
		bool isReal() const {return (bool)geom && (bool)phys;}
		//! If this interaction was just created in this step (for the constitutive law, to know that it is the first time there)
		bool isFresh(Scene* rb);
		bool isActive;

		Interaction() = default;
		Interaction(Body::id_t newId1,Body::id_t newId2);

		const Body::id_t& getId1() const {return id1;};
		const Body::id_t& getId2() const {return id2;};

		//! swaps order of bodies within the interaction
		void swapOrder();

		bool operator<(const Interaction& other) const { return getId1()<other.getId1() || (getId1()==other.getId1() && getId2()<other.getId2()); }

		//! cache functors that are called for this interaction. Currently used by InteractionLoop.
		struct {
			// Whether geometry dispatcher exists at all; this is different from !geom, since that can mean we haven't populated the cache yet.
			// Therefore, geomExists must be initialized to true first (done in Interaction::reset() called from ctor).
			bool geomExists;
			// shared_ptr's are initialized to NULLs automagically
			shared_ptr<IGeomFunctor> geom;
			shared_ptr<IPhysFunctor> phys;
			shared_ptr<LawFunctor> constLaw;
		} functorCache;

		//! Reset interaction to the intial state (keep only body ids)
		void reset();
		//! common initialization called from both constructor and reset()
		void init();

		// Member variables for serialization
		Body::id_t id1 = 0;
		Body::id_t id2 = 0;
		long iterMadeReal = -1;
		long iterLastSeen = -1;
		shared_ptr<IGeom> geom;
		shared_ptr<IPhys> phys;
		Vector2i cellDist = Vector2i(0,0);
		int linIx = -1;
		long iterBorn = -1;

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(id1));
			ar(CEREAL_NVP(id2));
			ar(CEREAL_NVP(iterMadeReal));
			ar(CEREAL_NVP(iterLastSeen));
			ar(CEREAL_NVP(geom));
			ar(CEREAL_NVP(phys));
			ar(CEREAL_NVP(cellDist));
			ar(CEREAL_NVP(iterBorn));
			// Note: linIx and functorCache are not serialized (runtime state)
		}

	public:
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE(Interaction);