#pragma once
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/IPhys.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/pkg/common/Sphere.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/DemXDofGeom.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
#endif

/*

L3Geom Ig2 functor cooperation:

1. functors define genericGo function, which take ``bool is6Dof`` as the first arg; they are called from functors returning both L3Geom and L6Geom (this only applies to sphere+sphere now, since Ig2_{Facet,Wall}_Sphere_L6Geom has not been written yet for lack of interest, though would be trivial)
2. genericGo function computes several parameter specific to shape types; then it calls (if L3GEOM_SPHERESLIKE is defined) handleSpheresLikeContact which contains the common code, given all data necessary.

Note that:

(a) although L3GEOM_SPHERESLIKE is enabled by default, its performance impact has not been measured yet (the compiler should be smart enough, since it is just factoring out common code).
(b) L3Geom only contains contPt and normal, which supposes (in L3Geom::applyLocalForce) that particles' centroids and the contact point are colinear; while this is true for spheres and mostly OK for facets&walls (since they are non-dynamic), it might be adjusted in the future -- L3Geom_Something deriving from L3Geom will be created, and exact branch vectors contained in it will be gotten via virtual method from L3Geom::applyLocalForce. This would be controlled via some approxMask (in the Law2 functor perhaps) so that it is only optional
(c) Ig2_Facet_Sphere_L3Geom is only enabled with L3GEOM_SPHERESLIKE

*/

#define L3GEOM_SPHERESLIKE

struct L3Geom: public GenericSpheresContact{
	Vector3r u;
	Vector3r u0;
	Matrix3r trsf;
	Vector3r F;
	
	const Real& uN;
	const Vector2r uT;
	
	virtual ~L3Geom();

	// utility function
	// TODO: currently supposes body's centroids are conencted with distance*normal
	// that will not be true for sphere+facet and others, watch out!
	// the force is oriented as applied to particle #1
	void applyLocalForce(const Vector3r& f, const Interaction* I, Scene* scene, NormShearPhys* nsp=NULL) const;
	void applyLocalForceTorque(const Vector3r& f, const Vector3r& t, const Interaction* I, Scene* scene, NormShearPhys* nsp=NULL) const;

	Vector3r relU() const{ return u-u0; }

	// Explicit constructor with initial values
	L3Geom(): u(Vector3r::Zero()), u0(Vector3r::Zero()), trsf(Matrix3r::Identity()), F(Vector3r::Zero()), uN(u[0]), uT(Vector2r::Map(&u[1])) {
		createIndex();
	}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(GenericSpheresContact, u, u0, trsf, F);
	
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(L3Geom,GenericSpheresContact)
};
REGISTER_SERIALIZABLE_BASE(L3Geom, GenericSpheresContact);

struct L6Geom: public L3Geom{
	Vector3r phi;
	Vector3r phi0;
	
	virtual ~L6Geom();
	
	// Explicit constructor with initial values
	L6Geom(): phi(Vector3r::Zero()), phi0(Vector3r::Zero()) {
		createIndex();
	}
	
	Vector3r relPhi() const{ return phi-phi0; }
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(L3Geom, phi, phi0);
	
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_INDEX_H(L6Geom,L3Geom)
};
REGISTER_SERIALIZABLE_BASE(L6Geom, L3Geom);

#ifdef SUDODEM_OPENGL
struct Gl1_L3Geom: public GlIGeomFunctor{
	bool axesLabels;
	Real axesScale;
	Real axesWd;
	Real uPhiWd;
	Real uScale;
	
	RENDERS(L3Geom);
	void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool);
	void draw(const shared_ptr<IGeom>&, bool isL6Geom=false, const Real& phiScale=0);
	
	// Explicit constructor with initial values
	Gl1_L3Geom(): axesLabels(false), axesScale(1.), axesWd(1.), uPhiWd(2.), uScale(1.) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(GlIGeomFunctor, axesLabels, axesScale, axesWd, uPhiWd, uScale);
	
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(Gl1_L3Geom, GlIGeomFunctor);

struct Gl1_L6Geom: public Gl1_L3Geom{
	Real phiScale;
	
	RENDERS(L6Geom);
	void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool);
	
	// Explicit constructor with initial values
	Gl1_L6Geom(): phiScale(1.) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Gl1_L3Geom, phiScale);
	
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(Gl1_L6Geom, Gl1_L3Geom);
#endif

struct Ig2_Sphere_Sphere_L3Geom: public IGeomFunctor{
	bool noRatch;
	Real distFactor;
	int trsfRenorm;
	int approxMask;
	
	virtual bool go(const shared_ptr<Shape>& s1, const shared_ptr<Shape>& s2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& I);
	virtual bool genericGo(bool is6Dof, const shared_ptr<Shape>& s1, const shared_ptr<Shape>& s2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& I);
	// common code for {sphere,facet,wall}+sphere contacts
	// facet&wall will get separated if L3Geom subclass with exact branch vector is created
	void handleSpheresLikeContact(const shared_ptr<Interaction>& I, const State& state1, const State& state2, const Vector3r& shift2, bool is6Dof, const Vector3r& normal, const Vector3r& contPt, Real uN, Real r1, Real r2);
	virtual void preStep(){ scene->setLocalCoords(true); }


	enum { APPROX_NO_MID_TRSF=1, APPROX_NO_MID_NORMAL=2, APPROX_NO_RENORM_MID_NORMAL=4 };

	// Explicit constructor with initial values
	Ig2_Sphere_Sphere_L3Geom(): noRatch(true), distFactor(1), trsfRenorm(100), approxMask(0) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(IGeomFunctor, noRatch, distFactor, trsfRenorm, approxMask);
	
	FUNCTOR2D(Sphere,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Sphere,Sphere);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Sphere_Sphere_L3Geom, IGeomFunctor);

struct Ig2_Wall_Sphere_L3Geom: public Ig2_Sphere_Sphere_L3Geom{
	virtual bool go(const shared_ptr<Shape>& s1, const shared_ptr<Shape>& s2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& I);
	
	FUNCTOR2D(Wall,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Wall,Sphere);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Wall_Sphere_L3Geom, Ig2_Sphere_Sphere_L3Geom);

#ifdef L3GEOM_SPHERESLIKE
struct Ig2_Facet_Sphere_L3Geom: public Ig2_Sphere_Sphere_L3Geom{
	// get point on segment A..B closest to P; algo: http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
	static Vector3r getClosestSegmentPt(const Vector3r& P, const Vector3r& A, const Vector3r& B){ Vector3r BA=B-A; Real u=(P.dot(BA)-A.dot(BA))/(BA.squaredNorm()); return A+min((Real)1.,max((Real)0.,u))*BA; }
	virtual bool go(const shared_ptr<Shape>& s1, const shared_ptr<Shape>& s2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& I);
	
	FUNCTOR2D(Facet,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Facet,Sphere);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Facet_Sphere_L3Geom, Ig2_Sphere_Sphere_L3Geom);
#endif

struct Ig2_Sphere_Sphere_L6Geom: public Ig2_Sphere_Sphere_L3Geom{
	virtual bool go(const shared_ptr<Shape>& s1, const shared_ptr<Shape>& s2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& I);
	
	FUNCTOR2D(Sphere,Sphere);
	DEFINE_FUNCTOR_ORDER_2D(Sphere,Sphere);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ig2_Sphere_Sphere_L6Geom, Ig2_Sphere_Sphere_L3Geom);


struct Law2_L3Geom_FrictPhys_ElPerfPl: public LawFunctor{
	bool noBreak;
	bool noSlip;
	int plastDissipIx;
	int elastPotentialIx;
	
	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
	
	// Explicit constructor with initial values
	Law2_L3Geom_FrictPhys_ElPerfPl(): noBreak(false), noSlip(false), plastDissipIx(-1), elastPotentialIx(-1) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(LawFunctor, noBreak, noSlip, plastDissipIx, elastPotentialIx);
	
	FUNCTOR2D(L3Geom,FrictPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_L3Geom_FrictPhys_ElPerfPl, LawFunctor);

struct Law2_L6Geom_FrictPhys_Linear: public Law2_L3Geom_FrictPhys_ElPerfPl{
	Real charLen;
	
	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
	
	// Explicit constructor with initial values
	Law2_L6Geom_FrictPhys_Linear(): charLen(1) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Law2_L3Geom_FrictPhys_ElPerfPl, charLen);
	
	FUNCTOR2D(L6Geom,FrictPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Law2_L6Geom_FrictPhys_Linear, Law2_L3Geom_FrictPhys_ElPerfPl);