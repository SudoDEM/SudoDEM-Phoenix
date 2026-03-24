/*************************************************************************
*  Copyright (C) 2017 by Sway Zhao                                       *
*  zhswee@gmail.com                                                      *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once

#include<sudodem/pkg/common/GLDrawFunctors.hpp>
#include <sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/core/Scene.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/pkg/fem/Node.hpp>
#include<sudodem/pkg/fem/FEContainer.hpp>

#include<sudodem/pkg/common/Sphere.hpp>

// define this to have topology information about facets enabled;
// it is necessary for TriElementTopologyAnalyzer
// #define FACET_TOPO
typedef Eigen::Matrix<Real,9,1> Vector9r;


// mimick expectation macros that linux has (see e.g. http://kerneltrap.org/node/4705)

#ifdef _WIN32
    #ifndef likely
        #define likely(x)   (x)
    #endif
    #ifndef unlikely
        #define unlikely(x) (x)
    #endif
#else
	#ifndef likely
		#define likely(x) __builtin_expect(!!(x),1)
	#endif
	#ifndef unlikely
		#define unlikely(x) __builtin_expect(!!(x),0)
	#endif//should be moved to Math.hpp
#endif

#define MEMBRANE_CONDENSE_DKT

class TriElement : public Shape{
    public:

	virtual ~TriElement();
    //Three nodes
    //Node::id_t nids[3];//node ids in the node container
	int numNodes() const { return nodes.size()==3; }

	/// TriElement's normal
	//Vector3r nf;
	/// Normals of edges
	Vector3r ne[3];
	/// Inscribing cirle radius
	Real icr;
	/// Length of the vertice vectors
	Real vl[3];
	/// Unit vertice vectors
	Vector3r vu[3];
    //gemetry-related func
    Matrix3r triangleInertia(const Vector3r& v0, const Vector3r& v1, const Vector3r& v2);
    Real triangleArea(const Vector3r& v0, const Vector3r& v1, const Vector3r& v2);
    Real TetraVolume(const Vector3r& v);//volume of a tetrhedron constructed with a side point v.
    void lumpMassInertia(Real density);
    //
    Vector3r getCentroid() const;
    Vector3r getNormal() const;
    // closest point on the facet
	Vector3r getNearestPt(const Vector3r& pt);
	vector<Vector3r> outerEdgeNormals() const;
	Real getArea() const;
	Real getPerimeterSq() const;
    // return velocity which is linearly interpolated between velocities of facet nodes, and also angular velocity at that point
	// takes fakeVel in account, with the NaN special case as documented
	std::tuple<Vector3r,Vector3r> interpolatePtLinAngVel(const Vector3r& x) const;
	std::tuple<Vector3r,Vector3r,Vector3r> getOuterVectors() const;
	bool isPointInTriangle(const Vector3r& pt);
	// generic routine: return nearest point on triangle closes to *pt*, given triangle vertices and its normal
	static Vector3r getNearestTrianglePt(const Vector3r& pt, const Vector3r& A, const Vector3r& B, const Vector3r& C, const Vector3r& normal);
    //
    Vector3r closestSegmentPt(const Vector3r& P, const Vector3r& A, const Vector3r& B, Real* normPos);
    Vector3r closestSegmentPt(const Vector3r& P, const Vector3r& A, const Vector3r& B);
    Vector3r glob2loc(const Vector3r& p){ return ori.conjugate()*(p-pos); }
	//
	Vector3r loc2glob(const Vector3r& p){ return ori*p+pos; }
    //
	bool hasRefConf() const { return refRot.size()==3; }
	bool hasBending(){ return KKdkt.size()>0; }
	void pyReset(){ refRot.clear(); }
	void setRefConf(); // use the current configuration as the referential one
	void ensureStiffnessMatrices(const Real& young, const Real& nu, const Real& thickness, bool bending, const Real& bendThickness);
	void updateNode(); // update local coordinate system
	void computeNodalDisplacements(Real dt, bool rotIncr);
	// called by functors to initialize (if necessary) and update
	void stepUpdate(Real dt, bool rotIncr);
	// called from DynDt for updating internal stiffness for given node
	void addIntraStiffnesses(const shared_ptr<Node>&, Vector3r& ktrans, Vector3r& krot) const;

    void applyNodalForces(const Scene* scene,Real dt);
    //
	void postLoad(TriElement&);
    
    // Member variables
    shared_ptr<Node> node1, node2, node3;
    vector<shared_ptr<Node>> nodes;
    Quaternionr ori;
    vector<Vector3r> vertices;
    Vector3r normal;
    Real area;
    Real halfThick;
    Vector3r pos;
    vector<Quaternionr> refRot;
    Vector6r refPos;
    Vector6r uXy;
    Real surfLoad;
    Vector6r phiXy;
    MatrixXr KKcst;
    MatrixXr KKdkt;

    #ifdef FACET_TOPO
    vector<Body::id_t> edgeAdjIds;
    vector<Real> edgeAdjHalfAngle;
    #endif

    #ifdef MEMBRANE_DEBUG_ROT
    Vector3r drill;
    vector<Quaternionr> currRot;
    VectorXr uDkt;
    #endif

	//! Default constructor
	TriElement() : ori(Quaternionr::Identity()), vertices(3, Vector3r(NaN, NaN, NaN)), normal(Vector3r(NaN, NaN, NaN)), area(NaN), halfThick(0), pos(Vector3r::Zero()), surfLoad(0) {
        nodes.resize(3);
        createIndex();
    }

	//! Python constructor
	static std::shared_ptr<TriElement> create() { return std::make_shared<TriElement>(); }

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX_H(TriElement,Shape)

	#if defined(FACET_TOPO) && defined(MEMBRANE_DEBUG_ROT)
		REGISTER_ATTRIBUTES(Shape,
			nodes, ori, vertices, normal, area, halfThick, pos, refRot, refPos,
			uXy, surfLoad, phiXy, KKcst, KKdkt,
			edgeAdjIds, edgeAdjHalfAngle,
			drill, currRot, uDkt
		)
	#elif defined(FACET_TOPO)
		REGISTER_ATTRIBUTES(Shape,
			nodes, ori, vertices, normal, area, halfThick, pos, refRot, refPos,
			uXy, surfLoad, phiXy, KKcst, KKdkt,
			edgeAdjIds, edgeAdjHalfAngle
		)
	#elif defined(MEMBRANE_DEBUG_ROT)
		REGISTER_ATTRIBUTES(Shape,
			nodes, ori, vertices, normal, area, halfThick, pos, refRot, refPos,
			uXy, surfLoad, phiXy, KKcst, KKdkt,
			drill, currRot, uDkt
		)
	#else
		REGISTER_ATTRIBUTES(Shape,
			nodes, ori, vertices, normal, area, halfThick, pos, refRot, refPos,
			uXy, surfLoad, phiXy, KKcst, KKdkt
		)
	#endif
};
////////
class Bo1_TriElement_Aabb : public BoundFunctor{
	public:
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body*);
	FUNCTOR1D(TriElement);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Bo1_TriElement_Aabb);
////////
class Gl1_TriElement : public GlShapeFunctor
{
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
	RENDERS(TriElement);

	// Member variables
	bool wire;
	bool normals;

	//! Default constructor
	Gl1_TriElement() : wire(true), normals(false) {}

	//! Python constructor
	static std::shared_ptr<Gl1_TriElement> create() { return std::make_shared<Gl1_TriElement>(); }

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;

	DECLARE_LOGGER;
	REGISTER_ATTRIBUTES(GlShapeFunctor, wire, normals);
};

REGISTER_SERIALIZABLE(Gl1_TriElement);


