#pragma once
/*
 * =====================================================================================
 *
 *       Filename:  superquadrics.h
 *
 *    Description:  superquadrics have been defined.
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
class Superquadrics: public Shape{
	public:
		//constructor
		Superquadrics(double x, double y, double z, double ep1, double ep2);//{/*createIndex();*/ rxyz = Vector3r(x,y,z); eps = Vector2r(ep1,ep2); Initial();};
		/*Superquadrics(Vector3r xyz, Vector2r _eps):rxyz(xyz),eps(_eps)
		{
		createIndex();

	        //initialize to calculate some basic geometric parameters.
	        Initial();
	        };*/
		virtual ~Superquadrics();
		void Initial();				//calculate some basic geometric parameters.
		bool IsInitialized(){return init;}
		/////////get
		//double getrx(){return rx;}
		//double getry(){return ry;}
		//double getrz(){return rz;}
		Vector3r getrxyz(){return rxyz;}
		//double geteps1(){return eps1;}
		//double geteps2(){return eps2;}
		Vector2r geteps(){return eps;}
		double getr_max(){return r_max;}
		Vector3r getPosition(){return Position;}
		double getVolume(){Initial();return Volume;}
		Vector3r getInertia(){Initial();return Inertia;}
		Quaternionr getOrientation(){return Orientation;}
        bool isInside(Vector3r p);
        //curvature related
	    Vector2r Curvatures(Vector2r phi,Vector3r &CurvatureDir_max,Vector3r &CurvatureDir_min,bool &curvatureMaxflag);//principal curvatures


		//void setrx(double x){rx = x;}
		//void setry(double y){ry = y;}
		//void setrz(double z){rz = z;}
		//void seteps1(double ep1){eps1 = ep1;}
		//void seteps2(double ep2){eps2 = ep2;}
		void setPosition(Vector3r p){Position = p;}
		void setOrientation(Quaternionr Ori){
		Ori.normalize();Orientation = Ori;
		//rotation matrices
                rot_mat2local = (Orientation).conjugate().toRotationMatrix();//to particle's system
	        rot_mat2global = (Orientation).toRotationMatrix();//to global system
		}

		////////////////////////////////////////////
		Vector3r getSurface(Vector2r phi) const;
		Vector3r getNormal(Vector2r);
		Vector2r Normal2Phi(Vector3r);
		void getCurrentP(Vector2r);      //get the coordinates of current point P.
		//deraivates
		Mat32r Derivate_P2Phi(Vector2r phi);  //3*2 matrix
		Vector3r Derivate_P2Phi_12(Vector2r phi);
		Vector3r Derivate_P2Phi_11(Vector2r phi);
		Vector3r Derivate_P2Phi_22(Vector2r phi);


		Mat23r Derivate_Phi2N(Vector2r phi, Vector3r n);
		Mat32r Derivate_C2Alpha(Vector2r alpha, Matrix3r RotationMat);
		//fuctions for contact detection
		Vector3r P_alpha12(Vector2r p, Matrix3r globalContact, int sign);
		Vector3r P_alpha12(Vector2r p, Vector3r &n, Matrix3r globalContact,Matrix3r RotationMat, int sign);   //surface represented with alpha1 and alpha2. Alpha1 and alpha2 determine the contact direction, i.e., contect vector c.
		Vector3r P_alpha12(Vector2r para,Matrix3r globalContact,Matrix3r RotationMat,int sign);
		Mat32r JacFunc(Vector2r para, Matrix3r globalContact, Matrix3r RotationMat, int sign);
		Mat32r JacFunc(Vector2r p, Matrix3r globalContact, int sign);		//Jacbian matrix
		Vector3r N_alpha12(Vector2r para, Matrix3r globalContact,int sign);
                //rotation matrices
                Matrix3r rot_mat2local; //(Orientation).conjugate().toRotationMatrix();//to particle's system
	        Matrix3r rot_mat2global; // (Orientation).toRotationMatrix();//to global system


	protected:
		//double rx, ry, rz, eps1, eps2;//the main parameters of a superquadric
		//r_max:the maximum r in the three axes
	        double r_max;
	        //sign of performed initialization
		bool init;
		Vector3r Position;			//the center position of a superquadric, i.e (x, y, z)
		double Volume;
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
		Vector3r rxyz;
		double rx;
		double ry;
		double rz;
		double eps1;
		double eps2;
		bool isSphere;
		Vector2r eps;

		Superquadrics() : rxyz(Vector3r::Ones()), rx(1), ry(1), rz(1), eps1(1), eps2(1), isSphere(false), eps(Vector2r::Ones()) {
			createIndex();
			init = false;
			m_glSolidListNum = -1;
			m_glWireListNum = -1;
			m_GL_slices = 10;
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, rxyz, rx, ry, rz, eps1, eps2, isSphere, eps);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(Superquadrics);
		REGISTER_CLASS_INDEX_H(Superquadrics,Shape)
};
REGISTER_SERIALIZABLE_BASE(Superquadrics, Shape);


//***************************************************************************
/*! Collision configuration for Superquadrics and something.
 * This is expressed as penetration volume properties: centroid, volume, depth ...
 *
 * Self-contained. */
class SuperquadricsGeom: public IGeom{
	public:
		virtual ~SuperquadricsGeom();

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

		SuperquadricsGeom() : PenetrationDepth(0.), contactAngle(Vector2r::Zero()), contactPoint(Vector3r::Zero()), shearInc(Vector3r::Zero()), relativeVn(Vector3r::Zero()), relativeVs(Vector3r::Zero()), normal(Vector3r::Zero()), twist_axis(Vector3r::Zero()), point1(Vector3r::Zero()), point2(Vector3r::Zero()), point11(Vector3r::Zero()), point22(Vector3r::Zero()), isShearNew(true), orthonormal_axis(Vector3r::Zero()) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeom, PenetrationDepth, contactAngle, contactPoint, shearInc, relativeVn, relativeVs, normal, twist_axis, point1, point2, point11, point22, isShearNew, orthonormal_axis);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsGeom);
		REGISTER_CLASS_INDEX_H(SuperquadricsGeom,IGeom)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsGeom, IGeom);

//SuperquadricsGeom2 is for Hertz-Mindlin model.
class SuperquadricsGeom2: public SuperquadricsGeom{
	public:
		virtual ~SuperquadricsGeom2();
        void HertzMindlin(Vector2r curvature1, Vector2r curvature2, double theta);

		Vector2r curvatures1;
		Vector2r curvatures2;
		Real curvatureAngle;
		Real alpha;
		Real K;
		Real E;
		Real curvatures_sum;
		bool isSphere;

		SuperquadricsGeom2() : curvatures1(Vector2r::Zero()), curvatures2(Vector2r::Zero()), curvatureAngle(0.), alpha(0.), K(0.), E(0.), curvatures_sum(0.), isSphere(false) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(SuperquadricsGeom, curvatures1, curvatures2, curvatureAngle, alpha, K, E, curvatures_sum, isSphere);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsGeom2);
		REGISTER_CLASS_INDEX_H(SuperquadricsGeom2,SuperquadricsGeom)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsGeom2, SuperquadricsGeom);
//***************************************************************************
/*! Creates Aabb from Superquadrics.
 *
 * Self-contained. */
class Bo1_Superquadrics_Aabb: public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& ig, shared_ptr<Bound>& bv, const Se3r& se3, const Body*);
		FUNCTOR1D(Superquadrics);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Bo1_Superquadrics_Aabb);
};
REGISTER_SERIALIZABLE_BASE(Bo1_Superquadrics_Aabb, BoundFunctor);

//***************************************************************************
/*! Elastic material */
class SuperquadricsMat: public Material{
	public:
		 SuperquadricsMat(double N, double S, double F){Kn=N; Ks=S; frictionAngle=F;};
		 double GetStrength(){return strength;};
	virtual ~SuperquadricsMat(){};

		Real Kn;
		Real Ks;
		Real frictionAngle;
		Real betan;
		Real betas;
		bool IsSplitable;
		double strength;

		SuperquadricsMat() : Kn(1e8), Ks(1e5), frictionAngle(0.5), betan(0.0), betas(0.0), IsSplitable(false), strength(100) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Material, Kn, Ks, frictionAngle, betan, betas, IsSplitable, strength);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsMat);
		REGISTER_CLASS_INDEX_H(SuperquadricsMat,Material)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsMat, FrictMat);
//material for Hertz-Mindlin model
class SuperquadricsMat2: public SuperquadricsMat{
	public:
		 SuperquadricsMat2(double G_in, double nu_in, double F){G=G_in; nu=nu_in; frictionAngle=F;};
		 //double GetStrength(){return strength;};
	virtual ~SuperquadricsMat2(){};

		Real G;
		Real nu;

		SuperquadricsMat2() : G(29e9), nu(0.15) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(SuperquadricsMat, G, nu);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsMat2);
		REGISTER_CLASS_INDEX_H(SuperquadricsMat2,SuperquadricsMat)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsMat2, SuperquadricsMat);


//***************************************************************************

//class SuperquadricsPhys: public IPhys{
//	public:
//	virtual ~SuperquadricsPhys(){};
//	SUDODEM_CLASS_BASE_DOC_ATTRS_CTOR(SuperquadricsPhys,IPhys,"Simple elastic material with friction for volumetric constitutive laws",
//		((Real,kn,0,,"Normal stiffness"))
//		((Vector3r,normalForce,Vector3r::Zero(),,"Normal force after previous step (in global coordinates)."))
//		((Real,ks,0,,"Shear stiffness"))
//		((Vector3r,shearForce,Vector3r::Zero(),,"Shear force after previous step (in global coordinates)."))
//		// Contact damping ratio as for linear elastic contact law
//		((Vector3r,normalViscous,Vector3r::Zero(),,"Normal viscous component"))
//		((Vector3r,shearViscous,Vector3r::Zero(),,"Shear viscous component"))
//		((Real,betan,0.0,,"Normal Damping Ratio. Fraction of the viscous damping coefficient (normal direction) equal to $\\frac{c_{n}}{C_{n,crit}}$."))
//		((Real,betas,0.0,,"Shear Damping Ratio. Fraction of the viscous damping coefficient (shear direction) equal to $\\frac{c_{s}}{C_{s,crit}}$."))
//		((Real,tangensOfFrictionAngle,0.,,"tangens of angle of internal friction")),
//		/*ctor*/ createIndex();
//	);
//	REGISTER_CLASS_INDEX(SuperquadricsPhys,IPhys);
//};
//REGISTER_SERIALIZABLE(SuperquadricsPhys);
//use FrictPhys as the basic class
class SuperquadricsPhys: public FrictPhys{
	public:
	virtual ~SuperquadricsPhys(){};

	Vector3r normalViscous;
	Vector3r shearViscous;
	Real betan;
	Real betas;

	SuperquadricsPhys() : normalViscous(Vector3r::Zero()), shearViscous(Vector3r::Zero()), betan(0.0), betas(0.0) {
		createIndex();
	}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(FrictPhys, normalViscous, shearViscous, betan, betas);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsPhys);
		REGISTER_CLASS_INDEX_H(SuperquadricsPhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsPhys, FrictPhys);

//////////////////////Herzian contact law
class SuperquadricsHertzMindlinPhys: public SuperquadricsPhys{
	public:
	virtual ~SuperquadricsHertzMindlinPhys(){};

	Real nuab;
	Real Gab;
	Real mu1;
	Real mu2;

	SuperquadricsHertzMindlinPhys() : nuab(0), Gab(0), mu1(0.), mu2(0.) {
		createIndex();
	}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(SuperquadricsPhys, nuab, Gab, mu1, mu2);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		REGISTER_CLASS_NAME_DERIVED(SuperquadricsHertzMindlinPhys);
		REGISTER_CLASS_INDEX_H(SuperquadricsHertzMindlinPhys,SuperquadricsPhys)
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsHertzMindlinPhys, FrictPhys);

//***************************************************************************
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
	#include<sudodem/lib/opengl/GLUtils.hpp>
	#ifdef __APPLE__
		#include<OpenGL/glu.h>
	#else
		#include<GL/glu.h>
	#endif
	#include<sudodem/pkg/dem/Shop.hpp>

	/*! Draw Superquadrics using OpenGL */
	class Gl1_Superquadrics: public GlShapeFunctor{
		private:
			//static vector<Vector3r> vertices;
            static int glSolidList;
		    static int glWireList;
            void initSolidGlList(Superquadrics*);
            void initWireGlList(Superquadrics*);
            static int pre_slices;
			void drawSlice(Vector3r &p1,Vector3r &p2,Vector3r &p3,Vector3r &p4);

		public:
							virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
							inline static bool wire = true;
							inline static int slices = 5;

							Gl1_Superquadrics() {} 			RENDERS(Superquadrics);
							SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_Superquadrics, GlShapeFunctor);

	struct Gl1_SuperquadricsGeom: public GlIGeomFunctor{
		RENDERS(SuperquadricsGeom);
		void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool);
		void draw(const shared_ptr<IGeom>&);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_SuperquadricsGeom, GlIGeomFunctor);
#endif


//***************************************************************************
class Ip2_SuperquadricsMat_SuperquadricsMat_SuperquadricsPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
	FUNCTOR2D(SuperquadricsMat,SuperquadricsMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Ip2_SuperquadricsMat_SuperquadricsMat_SuperquadricsPhys);
};
REGISTER_SERIALIZABLE_BASE(Ip2_SuperquadricsMat_SuperquadricsMat_SuperquadricsPhys, IPhysFunctor);
//***************************************************************************
//Hertz-Mindlin model
class Ip2_SuperquadricsMat2_SuperquadricsMat2_HertzMindlinPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
	FUNCTOR2D(SuperquadricsMat2,SuperquadricsMat2);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(Ip2_SuperquadricsMat2_SuperquadricsMat2_HertzMindlinPhys);
};
REGISTER_SERIALIZABLE_BASE(Ip2_SuperquadricsMat2_SuperquadricsMat2_HertzMindlinPhys, IPhysFunctor);

//***************************************************************************
/*! Calculate physical response based on penetration configuration given by TTetraGeom. */

class SuperquadricsLaw: public LawFunctor{
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
	SuperquadricsLaw() : shearForce(Vector3r::Zero()), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(LawFunctor, shearForce, traceEnergy, plastDissipIx, elastPotentialIx);

	FUNCTOR2D(SuperquadricsGeom,SuperquadricsPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(SuperquadricsLaw);
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsLaw, LawFunctor);

class SuperquadricsLaw2: public LawFunctor{

	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);

	Vector3r shearForce;

public:
	SuperquadricsLaw2() : shearForce(Vector3r::Zero()) {}

	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(LawFunctor, shearForce);

	FUNCTOR2D(SuperquadricsGeom2,SuperquadricsHertzMindlinPhys);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	REGISTER_CLASS_NAME_DERIVED(SuperquadricsLaw2);
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsLaw2, LawFunctor);
/*
class SuperquadricsLaw2: public SuperquadricsLaw{

	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);

	SUDODEM_CLASS_BASE_DOC_ATTRS_CTOR_PY(SuperquadricsLaw2,LawFunctor,"Calculate physical response of 2 :yref:`vector<Superquadrics>` in interaction, based on penetration configuration given by :yref:`SuperquadricsGeom`.",
	,,

	);
	FUNCTOR2D(SuperquadricsGeom2,SuperquadricsHertzMindlinPhys);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(SuperquadricsLaw2, LawFunctor);
*/
//********************************************************************************
shared_ptr<Body> NewSuperquadrics(double x, double y, double z, double ep1, double ep2, shared_ptr<Material> mat,bool rotate);
//shared_ptr<Body> NewSuperquadrics(Vector3r rxyz, Vector2r eps, shared_ptr<Material> mat,bool rotate);
