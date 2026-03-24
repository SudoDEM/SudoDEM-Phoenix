// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/lib/multimethods/Indexable.hpp>
#include<sudodem/core/Dispatcher.hpp>

// delete later and remove relevant code, to not support old State.blockedDOFs=['x','y','rz'] syntax anymore
//#define SUDODEM_DEPREC_DOF_LIST

class State: public Serializable, public Indexable{
	public:
		/// linear motion (references to inside se3)
		Vector2r& pos;
		/// rotational motion (reference to inside se3)
		Rotationr& ori;

		//! mutex for updating the parameters from within the interaction loop (only used rarely)
		std::mutex updateMutex;

		// bits for blockedDOFs
		enum {DOF_NONE=0,DOF_X=1,DOF_Y=2,DOF_RZ=4};
		//! shorthand for all DOFs blocked
		static const unsigned DOF_ALL=DOF_X|DOF_Y|DOF_RZ;
		//! shorthand for all displacements blocked
		static const unsigned DOF_XY=DOF_X|DOF_Y;
		//! shorthand for all rotations blocked
		//static const unsigned DOF_RZ=DOF_RX|DOF_RY|DOF_RZ;

		//! Return DOF_* constant for given axis∈{0,1} and rotationalDOF∈{false(default),true}; e.g. axisDOF(0,true)==DOF_RX
		static unsigned axisDOF(int axis, bool rotationalDOF=false){return 1<<(axis+(rotationalDOF?3:0));}
		//! set DOFs according to two Vector3r arguments (blocked is when disp[i]==1.0 or rot[i]==1.0)
		void setDOFfromVector3r(Vector3r disp_rot);//disp_rot(X,Y,RZ)
		//! Getter of blockedDOFs for list of strings (e.g. DOF_X | DOR_RX | DOF_RZ → 'xXZ')
		std::string blockedDOFs_vec_get() const;
		//! Setter of blockedDOFs from string ('xXZ' → DOF_X | DOR_RX | DOF_RZ)
		#ifdef SUDODEM_DEPREC_DOF_LIST
			void blockedDOFs_vec_set(const python::object&);
		#else
			void blockedDOFs_vec_set(const std::string& dofs);
		#endif

		//! Return displacement (current-reference position)
		Vector2r displ() const {return pos-refPos;}
		//! Return rotation (current-reference orientation, as Vector3r)//debug for 2D
		Real rot() const { return ori.angle(); }

		// python access functions: pos and ori are references to inside Se3r and cannot be pointed to directly
		Vector2r pos_get() const {return pos;}
		void pos_set(const Vector2r p) {pos=p;}
		Rotationr ori_get() const {return ori; }
		void ori_set(const Rotationr o){ori=o;}

	// Attribute declarations for serialization
		Se2r se2; Vector2r vel; Real mass; Real angVel; Real angMom; Real inertia; Vector2r refPos; Rotationr refOri; unsigned blockedDOFs; bool isDamped; Real densityScaling;

		State() : pos(se2.position), ori(se2.rotation), se2(Se2r(Vector2r::Zero(),Rotationr::Identity())), vel(Vector2r::Zero()), mass(0), angVel(0.0), angMom(0.0), inertia(0.0), refPos(Vector2r::Zero()), refOri(Rotationr::Identity()), blockedDOFs(0), isDamped(true), densityScaling(1) {
			pos = se2.position;
			ori = se2.rotation;
		}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(se2));
			ar(CEREAL_NVP(vel));
			ar(CEREAL_NVP(mass));
			ar(CEREAL_NVP(angVel));
			ar(CEREAL_NVP(angMom));
			ar(CEREAL_NVP(inertia));
			ar(CEREAL_NVP(refPos));
			ar(CEREAL_NVP(refOri));
			ar(CEREAL_NVP(blockedDOFs));
			ar(CEREAL_NVP(isDamped));
			ar(CEREAL_NVP(densityScaling));
		}

		virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_INDEX_COUNTER(State);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(State);
