// © 2004 Olivier Galizzi <olivier.galizzi@imag.fr>
// © 2008 Vaclav Smilauer <eudoxos@arcig.cz>
// © 2006 Bruno Chareyre <bruno.chareyre@hmg.inpg.fr>

#pragma once
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/State.hpp>
//#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/dem/DemXDofGeom.hpp>
/*! Class representing geometry of two bodies in contact.
 *
 * The code under SCG_SHEAR is experimental and is used only if ElasticContactLaw::useShear is explicitly true
 */

#define SCG_SHEAR

class ScGeom: public GenericSpheresContact {
	private:
		//cached values
		Vector3r twist_axis;//rotation vector around normal
		Vector3r orthonormal_axis;//rotation vector in contact plane
	public:
		// inherited from GenericSpheresContact: Vector3r& normal;
		Real &radius1, &radius2;
		Real penetrationDepth;
		Vector3r shearInc;
		
		virtual ~ScGeom();
		inline ScGeom& operator= (const ScGeom& source){
			normal=source.normal; contactPoint=source.contactPoint;
			twist_axis=source.twist_axis; orthonormal_axis=source.orthonormal_axis;
			radius1=source.radius1; radius2=source.radius2;
			penetrationDepth=source.penetrationDepth; shearInc=source.shearInc;
			return *this;}

		//!precompute values of shear increment and interaction rotation data. Update contact normal to the currentNormal value. Precondition : the value of normal is not updated outside (and before) this function.
		void precompute(const State& rbp1, const State& rbp2, const Scene* scene, const shared_ptr<Interaction>& c, const Vector3r& currentNormal, bool isNew, const Vector3r& shift2, bool avoidGranularRatcheting=true);

		//! Rotates a "shear" vector to keep track of contact orientation. Returns reference of the updated vector.
		Vector3r& rotate(Vector3r& tangentVector) const;
		const Vector3r& shearIncrement() const {return shearInc;}

		// Add method which returns the relative velocity (then, inside the contact law, this can be split into shear and normal component). Handle periodicity.
		Vector3r getIncidentVel(const State* rbp1, const State* rbp2, Real dt, const Vector3r& shiftVel, const Vector3r& shift2, bool avoidGranularRatcheting=true);
		// Implement another version of getIncidentVel which does not handle periodicity.
		Vector3r getIncidentVel(const State* rbp1, const State* rbp2, Real dt, bool avoidGranularRatcheting=true);
		// Add function to get the relative angular velocity (useful to determine bending moment at the contact level)
		Vector3r getRelAngVel(const State* rbp1, const State* rbp2, Real dt);

		// convenience version to be called from python
		Vector3r getIncidentVel_py(shared_ptr<Interaction> i, bool avoidGranularRatcheting);
		Vector3r getRelAngVel_py(shared_ptr<Interaction> i);

		ScGeom() : radius1(GenericSpheresContact::refR1), radius2(GenericSpheresContact::refR2), penetrationDepth(NaN), shearInc(Vector3r::Zero()), twist_axis(Vector3r::Zero()), orthonormal_axis(Vector3r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GenericSpheresContact, penetrationDepth, shearInc);
		
	REGISTER_CLASS_INDEX_H(ScGeom,GenericSpheresContact)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ScGeom, GenericSpheresContact);

class ScGeom6D: public ScGeom {
	public:
		Quaternionr initialOrientation1;
		Quaternionr initialOrientation2;
		Quaternionr twistCreep;
		Real twist;
		Vector3r bending;
		
		virtual ~ScGeom6D();
		const Real& getTwist() const {return twist;}
		const Vector3r& getBending() const {return bending;}
		void precomputeRotations(const State& rbp1, const State& rbp2, bool isNew, bool creep=false);
		void initRotations(const State& rbp1, const State& rbp2);
		
		ScGeom6D() : initialOrientation1(Quaternionr(1.0,0.0,0.0,0.0)), initialOrientation2(Quaternionr(1.0,0.0,0.0,0.0)), twistCreep(Quaternionr(1.0,0.0,0.0,0.0)), twist(0), bending(Vector3r::Zero()) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(ScGeom, initialOrientation1, initialOrientation2, twistCreep, twist, bending);
		
	REGISTER_CLASS_INDEX_H(ScGeom6D,ScGeom)
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ScGeom6D, ScGeom);


class ChCylGeom6D: public ScGeom6D {
	public:
		State fictiousState1;
		State fictiousState2;
		Real relPos1;
		Real relPos2;
		
		virtual ~ChCylGeom6D();
		
		ChCylGeom6D() : relPos1(0), relPos2(0) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(ScGeom6D, fictiousState1, fictiousState2, relPos1, relPos2);
		
	REGISTER_CLASS_INDEX_H(ChCylGeom6D,ScGeom6D)
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(ChCylGeom6D, ScGeom6D);























