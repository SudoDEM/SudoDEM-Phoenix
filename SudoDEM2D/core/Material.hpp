// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/lib/multimethods/Indexable.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Dispatcher.hpp>


class Scene;
/*! Material properties associated with a body.

Historical note: this used to be part of the PhysicalParameters class.
The other data are now in the State class.
*/
class Material: public Serializable, public Indexable{
	public:
		virtual ~Material() {};

		//! Function to return empty default-initialized instance of State that
		// is supposed to go along with this Material. Don't override unless you need
		// something else than basic State.
		virtual shared_ptr<State> newAssocState() const { return shared_ptr<State>(new State); }
		/*! Function that returns true if given State instance is what this material expects.

			Base Material class has no requirements, but the check would normally look like this:

				return (bool)dynamic_cast<State*> state;
		*/
		virtual bool stateTypeOk(State*) const { return true; }

		static const shared_ptr<Material> byId(int id, Scene* scene=NULL);
		static const shared_ptr<Material> byId(int id, shared_ptr<Scene> scene) {return byId(id,scene.get());}
		static const shared_ptr<Material> byLabel(const std::string& label, Scene* scene=NULL);
		static const shared_ptr<Material> byLabel(const std::string& label, shared_ptr<Scene> scene) {return byLabel(label,scene.get());}
		// return index of material, given its label
		static const int byLabelIndex(const std::string& label, Scene* scene=NULL);

	// Attribute declarations for serialization
		int id; string label; Real density;

		Material() : id(-1), density(1000) {}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(id));
			ar(CEREAL_NVP(label));
			ar(CEREAL_NVP(density));
		}

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_INDEX_COUNTER_H(Material)
};
REGISTER_SERIALIZABLE(Material);
