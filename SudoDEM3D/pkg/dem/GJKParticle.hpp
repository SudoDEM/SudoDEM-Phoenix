#ifndef GJKPARTICLE_H
#define GJKPARTICLE_H
/*
 * =====================================================================================
 *
 *       Filename:  GJKParticle.h
 *
 *    Description:  GJKParticle have been defined.
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

#ifdef SUDODEM_OPENGL
	#include<sudodem/lib/opengl/OpenGLWrapper.hpp>
#endif

#include <sudodem/pkg/dem/GJKParticle_shapes.h>
//**********************************************************************************
class GJKParticle: public Shape{
	public:
		int m_shapeType;
		std::vector<Vector3r> m_vertices;
		std::vector<Vector3r> m_vertices2;
		SD_Scalar m_radius;
		SD_Scalar m_height;
		SD_Scalar m_margin;
		bool isSphere;
		int m_GL_slices;
		int m_glSolidListNum;
		int m_glWireListNum;
		mutable int testflag;
		std::vector<int> facetTri;
		std::vector<Vector3r> m_facetVertices;
		
		GJKParticle(std::vector<Vector3r> vertices,int shapeType, SD_Scalar radius,SD_Scalar height,SD_Scalar margin=0.0)
			: m_vertices(vertices), m_shapeType(shapeType), m_radius(radius), m_height(height), m_margin(margin), 
			  isSphere(false), m_GL_slices(10), m_glSolidListNum(-1), m_glWireListNum(-1), testflag(0)
		{
			createIndex();
			m_init = false;
			Initial();
		}
		
		GJKParticle(SD_Scalar x, SD_Scalar y, SD_Scalar z, SD_Scalar ep1, SD_Scalar ep2)
			: m_shapeType(0), m_radius(0.0), m_height(0.0), m_margin(0.0), 
			  isSphere(false), m_GL_slices(10), m_glSolidListNum(-1), m_glWireListNum(-1), testflag(0)
		{
			createIndex();
			m_init = false;
			Initial();
		}
		
		GJKParticle(): m_shapeType(0), m_radius(0.0), m_height(0.0), m_margin(0.0), isSphere(false), m_GL_slices(10), m_glSolidListNum(-1), m_glWireListNum(-1), testflag(0)
		{
			createIndex();
			m_init = false;
			Initial();
		}
		
		virtual~GJKParticle();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Shape, m_shapeType, m_vertices, m_vertices2, m_radius, m_height, m_margin, isSphere, m_GL_slices, m_glSolidListNum, m_glWireListNum, testflag, facetTri, m_facetVertices);
		
		//friend bool intersect(const GJKParticle*, const GJKParticle*, Vector3r& v);
		//friend bool common_point(const GJKParticle*, const GJKParticle*, Vector3r&,  Vector3r&, Vector3r&);
		//friend bool penetration_depth(const GJKParticle*, const GJKParticle*,  Vector3r&, Vector3r&, Vector3r&);
		//friend SD_Scalar closest_points(const GJKParticle*, const GJKParticle*, Vector3r&, Vector3r&);
		//friend bool penetration_depth(const GJKParticle *,const Matrix3r&,  Vector3r&,const GJKParticle*,const Matrix3r&, Vector3r&, Vector3r&, Vector3r&, Vector3r&);
		//for GJK end

		void Initial();
		bool IsInitialized(){return m_init;}
		int getGLslices(){return m_GL_slices;}
		/////////get
		SD_Scalar getMargin(){return m_margin;}
		Vector3r getPosition(){return m_position;}
		Vector3r getCentroid(){return m_centroid;}
		int getShapeType() const {return m_shapeType;}
		T_MultiIndexBuf getFacetIndexBuf(){return m_facetIndexBuf;}
		double getVolume(){Initial();return m_volume;}
		double getRadius(){Initial();return m_radius;}
		Vector3r getInertia(){Initial();return m_inertia;}
		Quaternionr getOrientation(){return m_orientation;}

		void ConvexHull(std::vector<Vector3r>);
		void Polyhedron_volume_centroid();
		Matrix3r TetraInertiaTensor(Vector3r av,Vector3r bv,Vector3r cv,Vector3r dv);
		void Polyhedron_inertia();

		////////////////////////
		//rotation matrices
		Matrix3r rot_mat2local;
		Matrix3r rot_mat2global;

	private:
		T_MultiIndexBuf m_facetIndexBuf;
		Vector3r position;
		shared_ptr<DT_Convex> m_shape;
		shared_ptr<DT_Convex> m_shape2;
		MT_Transform m_xform;
		mutable GLuint m_displayList;
	protected:
		bool m_init;
		Vector3r m_position;
		double m_volume;
		Vector3r m_centroid;
		Vector3r m_inertia;
		Quaternionr m_orientation;

	public:
		SD_Scalar supportH(const Vector3r& v) const;
		Vector3r support(const Vector3r& v) const;

		void setMargin(SD_Scalar margin){m_margin = margin;}
		void setScaling(const Vector3r& scaling){m_xform.scale(scaling);}
		void setPosition(const Vector3r& pos){ m_position = pos;m_xform.setOrigin(pos);}
		void setOrientation(Quaternionr Ori){
			Ori.normalize();m_orientation = Ori;
			rot_mat2local = (m_orientation).conjugate().toRotationMatrix();
			rot_mat2global = (m_orientation).toRotationMatrix();
		}
		void setSe3(const Se3r& se3){m_xform.setOrigin(se3.position);m_xform.setRotation(se3.orientation);}
		void setMatrix(const float *m){m_xform.setValue(m);assert(m_xform.getBasis().determinant() != SD_Scalar(0.0));}
		void setMatrix(const double *m){m_xform.setValue(m);assert(m_xform.getBasis().determinant() != SD_Scalar(0.0));}
		void getMatrix(float *m) const{m_xform.getValue(m);}
		void getMatrix(double *m) const{m_xform.getValue(m);}

		DECLARE_LOGGER;
		
	REGISTER_CLASS_INDEX_H(GJKParticle,Shape)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GJKParticle, Shape);


//***************************************************************************
class GJKParticleGeom: public IGeom{
	public:
		Real PenetrationDepth;
		Vector2r contactAngle;
		Vector3r contactSA;
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
		
		virtual ~GJKParticleGeom();
		
		// Explicit constructor with initial values
		GJKParticleGeom(): PenetrationDepth(0.), contactAngle(Vector2r::Zero()), contactSA(Vector3r::Zero()), contactPoint(Vector3r::Zero()), shearInc(Vector3r::Zero()), relativeVn(Vector3r::Zero()), relativeVs(Vector3r::Zero()), normal(Vector3r::Zero()), twist_axis(Vector3r::Zero()), point1(Vector3r::Zero()), point2(Vector3r::Zero()), point11(Vector3r::Zero()), point22(Vector3r::Zero()), isShearNew(true), orthonormal_axis(Vector3r::Zero()) {
			createIndex();
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(IGeom, PenetrationDepth, contactAngle, contactSA, contactPoint, shearInc, relativeVn, relativeVs, normal, twist_axis, point1, point2, point11, point22, isShearNew, orthonormal_axis);
		
		//precompute data for shear evaluation
		void precompute(const State& rbp1, const State& rbp2, const Scene* scene, const shared_ptr<Interaction>& c, const Vector3r& currentNormal, bool isNew, const Vector3r& shift2);
		Vector3r& rotate(Vector3r& shearForce) const;
		
		REGISTER_CLASS_INDEX_H(GJKParticleGeom,IGeom)

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GJKParticleGeom, IGeom);

//***************************************************************************
class Bo1_GJKParticle_Aabb: public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& ig, shared_ptr<Bound>& bv, const Se3r& se3, const Body*);
		
		FUNCTOR1D(GJKParticle);
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Bo1_GJKParticle_Aabb, BoundFunctor);

//***************************************************************************
class GJKParticleMat: public Material{
	public:
		Real Kn;
		Real Ks;
		Real frictionAngle;
		Real betan;
		Real betas;
		bool IsSplitable;
		double strength;
		
		GJKParticleMat(): Kn(1e8), Ks(1e5), frictionAngle(.5), betan(0.0), betas(0.0), IsSplitable(false), strength(100.) {
			createIndex();
		}
		
		double GetStrength(){return strength;}
		
		virtual ~GJKParticleMat(){};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(Material, Kn, Ks, frictionAngle, betan, betas, IsSplitable, strength);
		
		REGISTER_CLASS_INDEX_H(GJKParticleMat,Material)
		
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(GJKParticleMat, Material);

//***************************************************************************
class GJKParticlePhys: public FrictPhys{
	public:
		Vector3r normalViscous;
		Vector3r shearViscous;
		Real betan;
		Real betas;
		
		virtual ~GJKParticlePhys(){};
		
		// Explicit constructor with initial values
		GJKParticlePhys(): normalViscous(Vector3r::Zero()), shearViscous(Vector3r::Zero()), betan(0.0), betas(0.0) {
			createIndex();
		}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(FrictPhys, normalViscous, shearViscous, betan, betas);
		
		REGISTER_CLASS_INDEX_H(GJKParticlePhys,FrictPhys)
};
REGISTER_SERIALIZABLE_BASE(GJKParticlePhys, FrictPhys);

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

	/*! Draw GJKParticle using OpenGL */
	class Gl1_GJKParticle: public GlShapeFunctor{
		private:
			void initSolidGlList(GJKParticle*);
			void initWireGlList(GJKParticle*);
		public:
			bool wire;
			int slices;
			
			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
			
			// Explicit constructor with initial values
			Gl1_GJKParticle(): wire(true), slices(10) {}
			
			// Use REGISTER_ATTRIBUTES for serialization
			REGISTER_ATTRIBUTES(GlShapeFunctor, wire, slices);
			
			RENDERS(GJKParticle);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_GJKParticle, GlShapeFunctor);

	struct Gl1_GJKParticleGeom: public GlIGeomFunctor{
		RENDERS(GJKParticleGeom);
		void go(const shared_ptr<IGeom>&, const shared_ptr<Interaction>&, const shared_ptr<Body>&, const shared_ptr<Body>&, bool);
		void draw(const shared_ptr<IGeom>&);
	};
	REGISTER_SERIALIZABLE_BASE(Gl1_GJKParticleGeom, GlIGeomFunctor);
#endif


//***************************************************************************
class Ip2_GJKParticleMat_GJKParticleMat_GJKParticlePhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1,
			const shared_ptr<Material>& b2,
			const shared_ptr<Interaction>& interaction);
		
		FUNCTOR2D(GJKParticleMat,GJKParticleMat);
};
REGISTER_SERIALIZABLE_BASE(Ip2_GJKParticleMat_GJKParticleMat_GJKParticlePhys, IPhysFunctor);

//***************************************************************************
class GJKParticleLaw: public LawFunctor{
	public:
		bool traceEnergy;
		int plastDissipIx;
		int elastPotentialIx;
		
		OpenMPAccumulator<Real> plasticDissipation;
		
		virtual ~GJKParticleLaw();
		
		// Explicit constructor with initial values
		GJKParticleLaw(): traceEnergy(false), plastDissipIx(-1), elastPotentialIx(-1) {}
		
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		Real elasticEnergy ();
		Real getPlasticDissipation();
		void initPlasticDissipation(Real initVal=0);
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(LawFunctor, traceEnergy, plastDissipIx, elastPotentialIx);
		
		FUNCTOR2D(GJKParticleGeom,GJKParticlePhys);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(GJKParticleLaw, LawFunctor);
//********************************************************************************

#endif