// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/lib/multimethods/Indexable.hpp>
#include<sudodem/core/Dispatcher.hpp>

// delete later and remove relevant code, to not support old State.blockedDOFs=['x','y','rz'] syntax anymore
//#define SUDODEM_DEPREC_DOF_LIST

class State: public Serializable, public Indexable{
	public:
		/// linear motion (references to inside se3)
		Vector3r& pos;
		/// rotational motion (reference to inside se3)
		Quaternionr& ori;

		//! mutex for updating the parameters from within the interaction loop (only used rarely)
		std::mutex updateMutex;

		// bits for blockedDOFs
		enum {DOF_NONE=0,DOF_X=1,DOF_Y=2,DOF_Z=4,DOF_RX=8,DOF_RY=16,DOF_RZ=32};
		//! shorthand for all DOFs blocked
		static const unsigned DOF_ALL=DOF_X|DOF_Y|DOF_Z|DOF_RX|DOF_RY|DOF_RZ;
		//! shorthand for all displacements blocked
		static const unsigned DOF_XYZ=DOF_X|DOF_Y|DOF_Z;
		//! shorthand for all rotations blocked
		static const unsigned DOF_RXRYRZ=DOF_RX|DOF_RY|DOF_RZ;
		//! shorthand for all rotations blocked
		//static const unsigned DOF_RZ=DOF_RX|DOF_RY|DOF_RZ;

		//! Return DOF_* constant for given axis∈{0,1,2} and rotationalDOF∈{false(default),true}; e.g. axisDOF(0,true)==DOF_RX
		static unsigned axisDOF(int axis, bool rotationalDOF=false){return 1<<(axis+(rotationalDOF?3:0));}
		//! set DOFs according to two Vector3r arguments (blocked is when disp[i]==1.0 or rot[i]==1.0)
		void setDOFfromVector3r(Vector3r disp_rot);//disp_rot(X,Y,Z,RX,RY,RZ)
		//! Getter of blockedDOFs for list of strings (e.g. DOF_X | DOR_RX | DOF_RZ → 'xXZ')
		std::string blockedDOFs_vec_get() const;
		//! Setter of blockedDOFs from string ('xXZ' → DOF_X | DOR_RX | DOF_RZ)
		#ifdef SUDODEM_DEPREC_DOF_LIST
			void blockedDOFs_vec_set(const python::object&);
		#else
			void blockedDOFs_vec_set(const std::string& dofs);
		#endif

		//! Return displacement (current-reference position)
		Vector3r displ() const {return pos-refPos;}
		//! Return rotation angle (current-reference orientation)//debug for 3D
		Real rot() const { return Eigen::AngleAxis<Real>(ori).angle(); }

		// python access functions: pos and ori are references to inside Se3r and cannot be pointed to directly
		Vector3r pos_get() const {return pos;}
		void pos_set(const Vector3r p) {pos=p;}
		Quaternionr ori_get() const {return ori; }
		void ori_set(const Quaternionr o){ori=o;}

	// Attribute declarations for serialization
		Se3r se3; Vector3r vel; Real mass; Vector3r angVel; Vector3r angMom; Vector3r inertia; Vector3r refPos; Quaternionr refOri; unsigned blockedDOFs; bool isDamped; Real densityScaling;

		State() : pos(se3.position), ori(se3.orientation), se3(Se3r(Vector3r::Zero(),Quaternionr::Identity())), vel(Vector3r::Zero()), mass(0), angVel(Vector3r::Zero()), angMom(Vector3r::Zero()), inertia(Vector3r::Zero()), refPos(Vector3r::Zero()), refOri(Quaternionr::Identity()), blockedDOFs(0), isDamped(true), densityScaling(1) {
			pos = se3.position;
			ori = se3.orientation;
		}

		// Cereal serialization
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& ar, unsigned int version) {
			ar(cereal::base_class<Serializable>(this));
			ar(CEREAL_NVP(se3));
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

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_INDEX_COUNTER_H(State)
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(State);
