#pragma once
/*
 * =====================================================================================
 *
 *       Filename:  superellipse.h
 *
 *    Description:  superellipse have been defined.
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
#include<sudodem/core/State.hpp>
#include<sudodem/pkg/common/Aabb.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/Wall.hpp>
//#include<sudodem/pkg/common/Facet.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

//#include <math.h>
//#include <cmath>
//#include <stdlib.h>
//#include <iostream>

//#include <Eigen/Dense>

typedef Eigen::Quaternion<Real> Quaternionr;
typedef Eigen::Matrix<Real,3,3> Matrix3r;
typedef Eigen::Matrix<Real, 3, 2> Mat32r;
typedef Eigen::Matrix<Real, 2, 3> Mat23r;
//typedef Eigen::Matrix<Real, 1,2> Vector2r;
typedef Eigen::Matrix<Real, 2,2> Matrix2d;
//using namespace Eigen;
//**********************************************************************************
class Superellipse: public Shape{
	public:
		//constructor
		Superellipse(double x, double y, double ep);//{/*createIndex();*/ rxy = Vector2r(x,y); eps = Vector2r(ep1,ep2); Initial();};
		/*Superellipse(Vector2r xyz, Vector2r _eps):rxyz(xyz),eps(_eps)
		{
		createIndex();

	        //initialize to calculate some basic geometric parameters.
	        Initial();
	        };*/
		virtual ~Superellipse();
		void Initial();				//calculate some basic geometric parameters.
		bool IsInitialized(){return init;}
		/////////get
		//double getrx(){return rx;}
		//double getry(){return ry;}
		//double getrz(){return rz;}
		Vector2r getrxy(){return rxy;}
		//double geteps1(){return eps1;}
		//double geteps2(){return eps2;}
		Real geteps(){return eps;}
		double getr_max(){return r_max;}
		Vector2r getPosition(){return Position;}
		double getArea(){Initial();return Area;}
		Real getInertia(){Initial();return Inertia;}
		Rotationr getOrientation(){return Orientation;}
    bool isInside(Vector2r p);

		//void setrx(double x){rx = x;}
		//void setry(double y){ry = y;}
		//void setrz(double z){rz = z;}
		//void seteps1(double ep1){eps1 = ep1;}
		//void seteps2(double ep2){eps2 = ep2;}
		void setPosition(Vector2r p){Position = p;}
		void setOrientation(Rotationr Ori){
		Orientation = Ori;
		//rotation matrices
    rot_mat2local = (Orientation).inverse().toRotationMatrix();//to particle's system
	  rot_mat2global = (Orientation).toRotationMatrix();//to global system
		}

		////////////////////////////////////////////
		Vector2r getSurface(Real phi) const;
		Vector2r getNormal(Real);
		Real Normal2Phi(Vector2r);


    //rotation matrices
    Matrix2r rot_mat2local; //(Orientation).conjugate().toRotationMatrix();//to particle's system
    Matrix2r rot_mat2global; // (Orientation).toRotationMatrix();//to global system


	protected:
		//double rx, ry, rz, eps1, eps2;//the main parameters of a superquadric
		//r_max:the maximum r in the three axes
    double r_max;
    //sign of performed initialization
		bool init;
		Vector2r Position;			//the center position of a superquadric, i.e (x, y, z)
		double Area;
		Real Inertia;			//the pricipal moments of inetia
		Rotationr Orientation;	//orientation
		Vector2r Surface;			//surface equations represented by the sureface parameters Phi1 and Phi2, where Phi1 belongs [-pi,pi], Phi2 in [-pi/2., pi/2.].
		Vector2r CurrentP;			//the local current point (x,y,z) that is used to calculate some parameters.
		Vector2r CurrentPhi;		//the local current phi1,phi2
		Vector2r CurrentNorm;		//the local current n1,n2,n3
		Vector2r ContactNorm;		//the vector of contact direction
public:
        int m_GL_slices;//
        int m_glSolidListNum;//glList number
        int m_glWireListNum;//glList number
        Vector2r rxy;
        Vector2r ref_rxy;
        double rx;
        double ry;
        double eps;
        bool isSphere;

        Superellipse() : rxy(Vector2r::Ones()), ref_rxy(Vector2r::Ones()), rx(1), ry(1), eps(1.0), isSphere(false) {
            createIndex();
            init = false;
            m_glSolidListNum = -1;
            m_glWireListNum = -1;
            m_GL_slices = 10;
        }

        // Use REGISTER_ATTRIBUTES for serialization
        REGISTER_ATTRIBUTES(Shape, rxy, ref_rxy, rx, ry, eps, isSphere);

        SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

		REGISTER_CLASS_NAME_DERIVED(Superellipse);
		REGISTER_CLASS_INDEX_H(Superellipse,Shape)
};
REGISTER_SERIALIZABLE_BASE(Superellipse, Shape);


//***************************************************************************
/*! Collision configuration for Superellipse and something.
 * This is expressed as penetration volume properties: centroid, volume, depth ...
 *
 * Self-contained. */
class SuperellipseGeom: public IGeom{
	public:
		//Real rotAngle;//rotation angle
		Vector2r preNormal;//normal at the previous step
		Real PenetrationDepth;
		Real contactAngle;
		Vector2r contactPoint;
		Vector2r shearInc;
		Vector2r relativeVn;
		Vector2r relativeVs;
		Vector2r normal;
		Vector2r shift2;
		Vector2r point1;
		Vector2r point2;
		Vector2r point11;
		Vector2r point22;
		bool isShearNew;
		
		virtual ~SuperellipseGeom();
		
		SuperellipseGeom() : preNormal(Vector2r::Zero()), PenetrationDepth(0.), contactAngle(0.0), contactPoint(Vector2r::Zero()), shearInc(Vector2r::Zero()), relativeVn(Vector2r::Zero()), relativeVs(Vector2r::Zero()), normal(Vector2r::Zero()), shift2(Vector2r::Zero()), point1(Vector2r::Zero()), point2(Vector2r::Zero()), point11(Vector2r::Zero()), point22(Vector2r::Zero()), isShearNew(true) {
			createIndex();
		}

		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeom, preNormal, PenetrationDepth, contactAngle, contactPoint, shearInc, relativeVn, relativeVs, normal, shift2, point1, point2, point11, point22, isShearNew);

		//precompute data for shear evaluation
		void precompute(const State& rbp1, const State& rbp2, const Scene* scene, const shared_ptr<Interaction>& c, const Vector2r& currentNormal, bool isNew, const Vector2r& shift2);
		Vector2r& rotate(Vector2r& shearForce) const;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<SuperellipseGeom, IGeom, std::shared_ptr<SuperellipseGeom>> _classObj(_module, "SuperellipseGeom", "Geometry of interaction between 2 :yref:`vector<Superellipse>`, including volumetric characteristics");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("PenetrationDepth", &SuperellipseGeom::PenetrationDepth, "PenetrationDepth");
			_classObj.def_readwrite("contactAngle", &SuperellipseGeom::contactAngle, "Normal direction (in local coordinates)");
			_classObj.def_readwrite("contactPoint", &SuperellipseGeom::contactPoint, "Contact point (global coords), center of the PenetrationDepth");
			_classObj.def_readwrite("shearInc", &SuperellipseGeom::shearInc, "Shear displacement increment in the last step");
			_classObj.def_readwrite("relativeVn", &SuperellipseGeom::relativeVn, "relative velocity in the normal at the contact");
			_classObj.def_readwrite("relativeVs", &SuperellipseGeom::relativeVs, "relative velocity in the tangential at the contact");
			_classObj.def_readwrite("normal", &SuperellipseGeom::normal, "Contact normal");
			_classObj.def_readwrite("shift2", &SuperellipseGeom::shift2, "");
			_classObj.def_readwrite("point1", &SuperellipseGeom::point1, "point 1 of the closest two points.");
			_classObj.def_readwrite("point2", &SuperellipseGeom::point2, "point 2 of the closest two points.");
			_classObj.def_readwrite("point11", &SuperellipseGeom::point11, "direction from point 1.FOR TESTING");
			_classObj.def_readwrite("point22", &SuperellipseGeom::point22, "direction from point 2.FOR TESTING");
						_classObj.def_readwrite("isShearNew", &SuperellipseGeom::isShearNew, "");
									}
					
									REGISTER_CLASS_INDEX_H(SuperellipseGeom,IGeom)
							};
					REGISTER_SERIALIZABLE_BASE(SuperellipseGeom, IGeom);
//***************************************************************************
/*! Creates Aabb from Superellipse.
 *
 * Self-contained. */
class Bo1_Superellipse_Aabb: public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& ig, shared_ptr<Bound>& bv, const Se2r& se2, const Body*) override;
		FUNCTOR1D(Superellipse);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Bo1_Superellipse_Aabb, BoundFunctor, std::shared_ptr<Bo1_Superellipse_Aabb>> _classObj(_module, "Bo1_Superellipse_Aabb", "Create/update :yref:`Aabb` of a :yref:`Superellipse`");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Bo1_Superellipse_Aabb, BoundFunctor);

//***************************************************************************
/*! Elastic material */
class SuperellipseMat: public Material{
	public:
		Real Kn;
		Real Ks;
		Real frictionAngle;
		Real betan;
		Real betas;
		bool IsSplitable;
		double strength;
		
		SuperellipseMat(double N, double S, double F) : Kn(N), Ks(S), frictionAngle(F) { createIndex(); }
		SuperellipseMat() : Kn(1e8), Ks(1e5), frictionAngle(0.5), betan(0.0), betas(0.0), IsSplitable(false), strength(100) { createIndex(); }
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Material, Kn, Ks, frictionAngle, betan, betas, IsSplitable, strength);
		
		double GetStrength(){return strength;};
		virtual ~SuperellipseMat(){};
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<SuperellipseMat, Material, std::shared_ptr<SuperellipseMat>> _classObj(_module, "SuperellipseMat", "Elastic material with Coulomb friction.");
			_classObj.def(pybind11::init<>());
			// Constructor with keyword arguments for common parameters
			_classObj.def(pybind11::init([](const std::string& label, Real Kn, Real Ks, Real frictionAngle, Real density) {
				auto m = std::make_shared<SuperellipseMat>();
				m->label = label;
				m->Kn = Kn;
				m->Ks = Ks;
				m->frictionAngle = frictionAngle;
				m->density = density;
				return m;
			}), pybind11::arg("label") = "", pybind11::arg("Kn") = 1e8, pybind11::arg("Ks") = 1e5, pybind11::arg("frictionAngle") = 0.5, pybind11::arg("density") = 1000);
			_classObj.def_readwrite("Kn", &SuperellipseMat::Kn, "Normal stiffness (N/m).");
					_classObj.def_readwrite("Ks", &SuperellipseMat::Ks, "Shear stiffness (N/m).");
					_classObj.def_readwrite("frictionAngle", &SuperellipseMat::frictionAngle, "Contact friction angle (in radians).");
					_classObj.def_readwrite("betan", &SuperellipseMat::betan, "Normal Damping Ratio. Fraction of the viscous damping coefficient (normal direction) equal to $\\frac{c_{n}}{C_{n,crit}}$.");
					_classObj.def_readwrite("betas", &SuperellipseMat::betas, "Shear Damping Ratio. Fraction of the viscous damping coefficient (shear direction) equal to $\\frac{c_{s}}{C_{s,crit}}$.");
					_classObj.def_readwrite("IsSplitable", &SuperellipseMat::IsSplitable, "To be splitted ... or not");
					_classObj.def_readwrite("strength", &SuperellipseMat::strength, "Stress at whis polyhedra of volume 4/3*pi [mm] breaks.");					        }
			
							REGISTER_CLASS_INDEX_H(SuperellipseMat,Material)
					};
			REGISTER_SERIALIZABLE_BASE(SuperellipseMat, Material);//material for Hertz-Mindlin model
class SuperellipseMat2: public SuperellipseMat{
	public:
		Real G;
		Real nu;
		
		SuperellipseMat2(double G_in, double nu_in, double F) : G(G_in), nu(nu_in) { frictionAngle = F; createIndex(); }
		SuperellipseMat2() : G(29e9), nu(0.15) { createIndex(); }
		virtual ~SuperellipseMat2(){};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(SuperellipseMat, G, nu);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<SuperellipseMat2, SuperellipseMat, std::shared_ptr<SuperellipseMat2>> _classObj(_module, "SuperellipseMat2", "Elastic material with Coulomb friction.");
			_classObj.def(pybind11::init<>());
						_classObj.def_readwrite("G", &SuperellipseMat2::G, "shear modulus (Pa).");
						_classObj.def_readwrite("nu", &SuperellipseMat2::nu, "Poisson's ratio.");
					        }
			
							REGISTER_CLASS_INDEX_H(SuperellipseMat2,SuperellipseMat)
					};
			REGISTER_SERIALIZABLE_BASE(SuperellipseMat2, SuperellipseMat);
//use FrictPhys as the basic class
class SuperellipsePhys: public FrictPhys{
	public:
		Vector2r normalViscous;
		Vector2r shearViscous;
		Real betan;
		Real betas;
		
		SuperellipsePhys() : normalViscous(Vector2r::Zero()), shearViscous(Vector2r::Zero()), betan(0.0), betas(0.0) { createIndex(); }
		virtual ~SuperellipsePhys(){};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, normalViscous, shearViscous, betan, betas);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<SuperellipsePhys, FrictPhys, std::shared_ptr<SuperellipsePhys>> _classObj(_module, "SuperellipsePhys", "Simple elastic material with friction for volumetric constitutive laws");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("normalViscous", &SuperellipsePhys::normalViscous, "Normal viscous component");
			_classObj.def_readwrite("shearViscous", &SuperellipsePhys::shearViscous, "Shear viscous component");
			_classObj.def_readwrite("betan", &SuperellipsePhys::betan, "Normal Damping Ratio. Fraction of the viscous damping coefficient (normal direction) equal to $\\frac{c_{n}}{C_{n,crit}}$.");
			_classObj.def_readwrite("betas", &SuperellipsePhys::betas, "Shear Damping Ratio. Fraction of the viscous damping coefficient (shear direction) equal to $\\frac{c_{s}}{C_{s,crit}}$.");
		}

		REGISTER_CLASS_INDEX_H(SuperellipsePhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(SuperellipsePhys, FrictPhys);

//***************************************************************************
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
	#include<sudodem/lib/opengl/GLUtils.hpp>
	#ifdef __APPLE__
		#include <OpenGL/glu.h>
	#else
		#include <GL/glu.h>
	#endif
	#include<sudodem/pkg/dem/Shop.hpp>
	class Gl1_Superellipse : public GlShapeFunctor{
		private:
			// for stripes
			//static int glStripedDiskList;
			//static int glGlutDiskList;
			//Generate GlList for GLUT disk
			void initWireGlList(Superellipse* t);
			//Generate GlList for sliced disks
			void initSolidGlList(Superellipse* t);
			//for regenerating glutDisk list if needed
			SUDODEM_STATIC_MEMBER_API static int preSlices;
		public:
			SUDODEM_STATIC_MEMBER_API static bool wire;
			SUDODEM_STATIC_MEMBER_API static int Slices;

			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&) override;

			SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
				pybind11::class_<Gl1_Superellipse, Functor, std::shared_ptr<Gl1_Superellipse>> _classObj(_module, "Gl1_Superellipse", "Renders :yref:`Superellipse` object");
				_classObj.def(pybind11::init<>());
				_classObj.def_readwrite_static("wire", &Gl1_Superellipse::wire, "Only show wireframe (controlled by ``glutSlices`` and ``glutStacks``.");
				_classObj.def_readwrite_static("Slices", &Gl1_Superellipse::Slices, "Base number of Superellipse slices");
			}
		RENDERS(Superellipse);
	};

	REGISTER_SERIALIZABLE_BASE(Gl1_Superellipse, GlShapeFunctor);
	struct Gl1_SuperellipseGeom: public GlIGeomFunctor{
		void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool) override;
		void draw(const shared_ptr<IGeom>&);

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Gl1_SuperellipseGeom, Functor, std::shared_ptr<Gl1_SuperellipseGeom>> _classObj(_module, "Gl1_SuperellipseGeom", "Render :yref:`SuperellipseGeom` geometry.");
			_classObj.def(pybind11::init<>());
		}
		RENDERS(SuperellipseGeom);
	};

	REGISTER_SERIALIZABLE_BASE(Gl1_SuperellipseGeom, GlIGeomFunctor);
#endif


//***************************************************************************
class Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction) override;
		FUNCTOR2D(SuperellipseMat,SuperellipseMat);
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys, IPhysFunctor, std::shared_ptr<Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys>> _classObj(_module, "Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys", "");
			_classObj.def(pybind11::init<>());
		}
};
REGISTER_SERIALIZABLE_BASE(Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys, IPhysFunctor);

//***************************************************************************
/*! Calculate physical response based on penetration configuration given by TTetraGeom. */

class SuperellipseLaw: public LawFunctor{
	OpenMPAccumulator<Real> plasticDissipation;
public:
	Vector2r shearForce;
	bool traceEnergy;
	int plastDissipIx;
	int elastPotentialIx;

	virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*) override;
	Real elasticEnergy ();
	Real getPlasticDissipation();
	void initPlasticDissipation(Real initVal=0);
	
	SuperellipseLaw() : shearForce(Vector2r::Zero()), traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
	FUNCTOR2D(SuperellipseGeom,FrictPhys);
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
		pybind11::class_<SuperellipseLaw, LawFunctor, std::shared_ptr<SuperellipseLaw>> _classObj(_module, "SuperellipseLaw", "Calculate physical response of 2 :yref:`vector<Superellipse>` in interaction, based on penetration configuration given by :yref:`SuperellipseGeom`.");
		_classObj.def(pybind11::init<>());
		_classObj.def_readwrite("shearForce", &SuperellipseLaw::shearForce, "Shear force from last step");
		_classObj.def_readwrite("traceEnergy", &SuperellipseLaw::traceEnergy, "Define the total energy dissipated in plastic slips at all contacts. This will trace only plastic energy in this law, see O.trackEnergy for a more complete energies tracing");
		_classObj.def("elasticEnergy",&SuperellipseLaw::elasticEnergy,"Compute and return the total elastic energy in all \"FrictPhys\" contacts");
		_classObj.def("plasticDissipation",&SuperellipseLaw::getPlasticDissipation,"Total energy dissipated in plastic slips at all FrictPhys contacts. Computed only if :yref:`Law2_ScGeom_FrictPhys_CundallStrack::traceEnergy` is true.");
		_classObj.def("initPlasticDissipation",&SuperellipseLaw::initPlasticDissipation,"Initialize cummulated plastic dissipation to a value (0 by default).");
		}

	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(SuperellipseLaw, LawFunctor);
