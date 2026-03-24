#pragma once
/*
 * =====================================================================================
 *
 *       Filename:  PolySuperellipsoid.h
 *
 *    Description:  PolySuperellipsoid have been defined.
 *
 *        Version:  1.0
 *        Created:  07/22/2015 05:16:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  zhswee (zhswee@gmail.com)
 *   Organization: South China University of Technology
 *
 * =====================================================================================
 */

#include<vector>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Material.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>
#include<sudodem/lib/base/Math.hpp>
//#include <math.h>
//#include <cmath>
//#include <stdlib.h>
//#include <iostream>

//#include <Eigen/Dense>

//typedef Eigen::Quaternion<Real> Quaternionr;
//typedef Eigen::Matrix<Real,3,3> Matrix3r;
typedef Eigen::Matrix<Real, 3, 2> Mat32r;
typedef Eigen::Matrix<Real, 2, 3> Mat23r;
//typedef Eigen::Matrix<Real, 1,2> Vector2r;
typedef Eigen::Matrix<Real, 2,2> Matrix2d;//move to Math.hpp later

//using namespace Eigen;
//**********************************************************************************
class PolySuperellipsoid: public Shape{
	public:
		//constructor
		PolySuperellipsoid(Vector2r eps, Vector6r halflen);//{/*createIndex();*/ rxyz = Vector3r(x,y,z); eps = Vector2r(ep1,ep2); Initial();};
		/*PolySuperellipsoid(Vector3r xyz, Vector2r _eps):rxyz(xyz),eps(_eps)
		{
		createIndex();

	        //initialize to calculate some basic geometric parameters.
	        Initial();
	        };*/
		virtual ~PolySuperellipsoid();
		void Initial();				//calculate some basic geometric parameters.
		bool IsInitialized(){return init;}
		/////////get
		//double getrx(){return rx;}
		//double getry(){return ry;}
		//double getrz(){return rz;}
		Vector6r getrxyz(){return rxyz;}
		Vector6r getrxyz_ref(){return rxyz_ref;}
		//double geteps1(){return eps1;}
		//double geteps2(){return eps2;}
		Vector2r geteps(){return eps;}
		double getr_max(){return r_max;}
		double getr_max_ref(){return r_max_ref;}
		Vector3r getPosition(){return Position;}
		double getVolume(){Initial();return Volume;}
		Vector3r getMassCenter(){Initial();return massCenter;}
		Vector3r getMassCenter_ref(){return massCenter_ref;}
		Vector3r getInertia(){Initial();return Inertia;}
		Quaternionr getOrientation(){return Orientation;}
    bool isInside(Vector3r p);

		void setPosition(Vector3r p){Position = p;}
		void setOrientation(Quaternionr Ori){
		Ori.normalize();Orientation = Ori;
		//rotation matrices
    rot_mat2local = (Orientation).conjugate().toRotationMatrix();//to particle's system
	  rot_mat2global = (Orientation).toRotationMatrix();//to global system
		}
		void setRxyz(Vector6r rxyz_in){rxyz = rxyz_in;}
		void setMassCenter(Vector3r mc){massCenter = mc;}
		void setRmax(double rm){r_max = rm;}

    //rotation matrices
    Matrix3r rot_mat2local; //(Orientation).conjugate().toRotationMatrix();//to particle's system
	  Matrix3r rot_mat2global; // (Orientation).toRotationMatrix();//to global system
		Vector3r getSurface(Vector2r phi) const;//at the local with respect to the geometric center
		Vector3r getSurfaceMC(Vector2r phi) const;//at the local with respect to the mass center
		Vector3r getNormal(Vector2r);
		Vector2r Normal2Phi(Vector3r);
		Vector3r Normal2SurfaceMC(Vector3r n) const;
		Vector3r Normal2SurfaceMCgl(Vector3r gn) const;
		Vector3r support(Vector3r n) const;
	protected:
		//double rx, ry, rz, eps1, eps2;//the main parameters of a superquadric
		//r_max:the maximum r in the three axes
	  double r_max, r_max_ref;
	  //sign of performed initialization
		bool init;
		Vector3r Position;			//the center position of a superquadric, i.e (x, y, z)
		double Volume;
		Vector6r rxyz_ref;
		Vector3r massCenter;
		Vector3r massCenter_ref;//used in the expansion method
		Vector3r Inertia;			//the pricipal moments of inetia
		Quaternionr Orientation;	//orientation
		Vector3r Surface;			//surface equations represented by the sureface parameters Phi1 and Phi2, where Phi1 belongs [-pi,pi], Phi2 in [-pi/2., pi/2.].
		Vector3r CurrentP;			//the local current point (x,y,z) that is used to calculate some parameters.
		Vector2r CurrentPhi;		//the local current phi1,phi2
		Vector3r CurrentNorm;		//the local current n1,n2,n3
		Vector3r ContactNorm;		//the vector of contact direction
public:
    int m_GL_slices;//
    int m_glSolidListNum;//glList number
    int m_glWireListNum;//glList number
		Vector6r rxyz;
		double eps1;
		double eps2;
		bool isSphere;
		Vector2r eps;

		PolySuperellipsoid() : rxyz(Vector6r::Ones()), eps1(1), eps2(1), isSphere(false), eps(Vector2r::Ones()) {
			createIndex();
			init = false;
			m_glSolidListNum = -1;
			m_glWireListNum = -1;
			m_GL_slices = 10;
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, rxyz, rxyz_ref, eps1, eps2, isSphere, eps);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(PolySuperellipsoid);
		REGISTER_CLASS_INDEX_H(PolySuperellipsoid,Shape)
};
REGISTER_SERIALIZABLE_BASE(PolySuperellipsoid, Shape);


//***************************************************************************
/*! Collision configuration for PolySuperellipsoid and something.
 * This is expressed as penetration volume properties: centroid, volume, depth ...
 *
 * Self-contained. */
class PolySuperellipsoidGeom: public IGeom{
	public:
		virtual ~PolySuperellipsoidGeom();

		//precompute data for shear evaluation
		void precompute(const State& rbp1, const State& rbp2, const Scene* scene, const shared_ptr<Interaction>& c, const Vector3r& currentNormal, bool isNew, const Vector3r& shift2);
		Vector3r& rotate(Vector3r& shearForce) const;

		Real PenetrationDepth;
		Vector2r contactAngle;
		Vector3r contactPoint;
		Vector3r shearInc;
		Vector3r relativeVn;
		Vector3r relativeVs;
		Vector3r normal;
		Vector3r twist_axis;
		Vector3r point1;
		Vector3r point2;
		Vector3r point11;
		Vector3r point22;
		bool isShearNew;
		Vector3r orthonormal_axis;

		PolySuperellipsoidGeom() : PenetrationDepth(0.), contactAngle(Vector2r::Zero()), contactPoint(Vector3r::Zero()), shearInc(Vector3r::Zero()), relativeVn(Vector3r::Zero()), relativeVs(Vector3r::Zero()), normal(Vector3r::Zero()), twist_axis(Vector3r::Zero()), point1(Vector3r::Zero()), point2(Vector3r::Zero()), point11(Vector3r::Zero()), point22(Vector3r::Zero()), isShearNew(true), orthonormal_axis(Vector3r::Zero()) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeom, PenetrationDepth, contactAngle, contactPoint, shearInc, relativeVn, relativeVs, normal, twist_axis, point1, point2, point11, point22, isShearNew, orthonormal_axis);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(PolySuperellipsoidGeom);
		REGISTER_CLASS_INDEX_H(PolySuperellipsoidGeom,IGeom)
};
REGISTER_SERIALIZABLE_BASE(PolySuperellipsoidGeom, IGeom);
//***************************************************************************
/*! Creates Aabb from PolySuperellipsoid.
 *
 * Self-contained. */
class Bo1_PolySuperellipsoid_Aabb: public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& ig, shared_ptr<Bound>& bv, const Se3r& se3, const Body*);
		FUNCTOR1D(PolySuperellipsoid);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Bo1_PolySuperellipsoid_Aabb);
};
REGISTER_SERIALIZABLE_BASE(Bo1_PolySuperellipsoid_Aabb, BoundFunctor);

//***************************************************************************
/*! Elastic material */
class PolySuperellipsoidMat: public Material{
	public:
		 PolySuperellipsoidMat(double N, double S, double F){Kn=N; Ks=S; frictionAngle=F;};
		 double GetStrength(){return strength;};
	virtual ~PolySuperellipsoidMat(){};

	Real Kn;
	Real Ks;
	Real frictionAngle;
	Real betan;
	Real betas;
	bool IsSplitable;
	double strength;

	PolySuperellipsoidMat() : Kn(1e8), Ks(1e5), frictionAngle(.5), betan(0.0), betas(0.0), IsSplitable(false), strength(100) {
		createIndex();
	}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Material, Kn, Ks, frictionAngle, betan, betas, IsSplitable, strength);

	REGISTER_CLASS_NAME_DERIVED(PolySuperellipsoidMat);
	REGISTER_CLASS_INDEX_H(PolySuperellipsoidMat,Material)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PolySuperellipsoidMat, FrictMat);
//***************************************************************************
//use FrictPhys as the basic class
class PolySuperellipsoidPhys: public FrictPhys{
	public:
	virtual ~PolySuperellipsoidPhys(){};

	Vector3r normalViscous;
	Vector3r shearViscous;
	Real betan;
	Real betas;

	PolySuperellipsoidPhys() : normalViscous(Vector3r::Zero()), shearViscous(Vector3r::Zero()), betan(0.0), betas(0.0) {
		createIndex();
	}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(FrictPhys, normalViscous, shearViscous, betan, betas);

	REGISTER_CLASS_NAME_DERIVED(PolySuperellipsoidPhys);
	REGISTER_CLASS_INDEX_H(PolySuperellipsoidPhys,FrictPhys)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PolySuperellipsoidPhys, FrictPhys);
//***************************************************************************
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
	#include<sudodem/lib/opengl/GLUtils.hpp>
	#ifdef __APPLE__
		#include<OpenGL/glu.h>
	#else
		#ifdef __APPLE__
	#include<OpenGL/glu.h>
#else
	#include<GL/glu.h>
#endif
	#endif
	#include<sudodem/pkg/dem/Shop.hpp>

	/*! Draw PolySuperellipsoid using OpenGL */
	class Gl1_PolySuperellipsoid: public GlShapeFunctor{
		private:
			//static vector<Vector3r> vertices;
            static int glSolidList;
		    static int glWireList;
            void initSolidGlList(PolySuperellipsoid*);
            void initWireGlList(PolySuperellipsoid*);
            static int pre_slices;
			void drawSlice(Vector3r &p1,Vector3r &p2,Vector3r &p3,Vector3r &p4);

		public:
			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
			// Static attributes
			static bool wire;
			static int slices;

			RENDERS(PolySuperellipsoid);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_PolySuperellipsoid, GlShapeFunctor);

	struct Gl1_PolySuperellipsoidGeom: public GlIGeomFunctor{
		RENDERS(PolySuperellipsoidGeom);
		void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool);
		void draw(const shared_ptr<IGeom>&);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_PolySuperellipsoidGeom, GlIGeomFunctor);
#endif


//***************************************************************************
class Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
	FUNCTOR2D(PolySuperellipsoidMat,PolySuperellipsoidMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys);
};
REGISTER_SERIALIZABLE_BASE(Ip2_PolySuperellipsoidMat_PolySuperellipsoidMat_PolySuperellipsoidPhys, IPhysFunctor);
//***************************************************************************
/*! Calculate physical response based on penetration configuration given by TTetraGeom. */

class PolySuperellipsoidLaw: public LawFunctor{
	OpenMPAccumulator<Real> plasticDissipation;
	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
	Real elasticEnergy ();
	Real getPlasticDissipation();
	void initPlasticDissipation(Real initVal=0);

	Vector3r shearForce;
	bool traceEnergy;
	int plastDissipIx;
	int elastPotentialIx;

public:
	PolySuperellipsoidLaw() : shearForce(Vector3r::Zero()), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(LawFunctor, shearForce, traceEnergy, plastDissipIx, elastPotentialIx);

	FUNCTOR2D(PolySuperellipsoidGeom,PolySuperellipsoidPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(PolySuperellipsoidLaw);
};
REGISTER_SERIALIZABLE_BASE(PolySuperellipsoidLaw, LawFunctor);
//********************************************************************************
//shared_ptr<Body> NewPolySuperellipsoid(double x, double y, double z, double ep1, double ep2, shared_ptr<Material> mat,bool rotate);
