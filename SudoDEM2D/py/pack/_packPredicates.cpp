// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/lib/base/Logging.hpp>
//#include<sudodem/lib/base/Math.hpp>
#include<sudodem/lib/pyutil/doc_opts.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

/*
This file contains various predicates that say whether a given point is within the solid,
or, not closer than "pad" to its boundary, if pad is nonzero
Besides the (point,pad) operator, each predicate defines aabb() method that returns
(min,max) tuple defining minimum and maximum point of axis-aligned bounding box
for the predicate.

These classes are primarily used for sudodem.pack.* functions creating packings.
See examples/regular-disk-pack/regular-disk-pack.py for an example.

*/

// aux functions
void ttuple2vvec(const py::tuple& t, Vector3r& v1, Vector3r& v2){ v1=t[0].cast<Vector3r>()); v2=py::extract<Vector3r>(t[1])(); }
// do not use make_tuple directly on vector ops, since their type can be something like Eigen::CwiseBinaryOp<...>
py::tuple vvec2tuple(const Vector3r& a, const Vector3r& b){ return pybind11::make_tuple(a,b); }

struct Predicate{
	public:
		virtual bool operator() (const Vector3r& pt,Real pad=0.) const = 0;
		virtual py::tuple aabb() const = 0;
		Vector3r dim() const { Vector3r mn,mx; ttuple2vvec(aabb(),mn,mx); return (mx-mn).eval(); }
		Vector3r center() const { Vector3r mn,mx; ttuple2vvec(aabb(),mn,mx); return .5*(mn+mx); }
};
// make the pad parameter optional

/* Since we want to make Predicate::operator() and Predicate::aabb() callable from c++ on py::object
with the right virtual method resolution, we have to wrap the class in the following way.

This makes it possible to derive a python class from Predicate, override its aabb() method, for instance,
and use it in PredicateUnion, which will call the python implementation of aabb() as it should. This
approach is used in the inGtsSurface class defined in pack.py.

See scripts/test/gts-operators.py for an example.

NOTE: you still have to call base class ctor in your class' ctor derived in python, e.g.
super(inGtsSurface,self).__init__() so that virtual methods work as expected.
*/
struct PredicateWrap: Predicate, py::wrapper<Predicate>{
	bool operator()(const Vector3r& pt, Real pad=0.) const { return this->get_override("__call__")(pt,pad);}
	py::tuple aabb() const { return this->get_override("aabb")(); }
};
// make the pad parameter optional

/*********************************************************************************
****************** Boolean operations on predicates ******************************
*********************************************************************************/

const Predicate& obj2pred(py::object obj){ return py::extract<const Predicate&>(obj)();}

class PredicateBoolean: public Predicate{
	protected:
		const py::object A,B;
	public:
		PredicateBoolean(const py::object _A, const py::object _B): A(_A), B(_B){}
		const py::object getA(){ return A;}
		const py::object getB(){ return B;}
};

// http://www.linuxtopia.org/online_books/programming_books/python_programming/python_ch16s03.html
class PredicateUnion: public PredicateBoolean{
	public:
		PredicateUnion(const py::object _A, const py::object _B): PredicateBoolean(_A,_B){}
		virtual bool operator()(const Vector3r& pt,Real pad) const {return obj2pred(A)(pt,pad)||obj2pred(B)(pt,pad);}
		virtual py::tuple aabb() const {
			Vector3r minA,maxA,minB,maxB;
			ttuple2vvec(obj2pred(A).aabb(),minA,maxA);
			ttuple2vvec(obj2pred(B).aabb(),minB,maxB);
			return vvec2tuple(minA.cwiseMin(minB),maxA.cwiseMax(maxB));
		}
};
PredicateUnion makeUnion(const py::object& A, const py::object& B){ return PredicateUnion(A,B);}

class PredicateIntersection: public PredicateBoolean{
	public:
		PredicateIntersection(const py::object _A, const py::object _B): PredicateBoolean(_A,_B){}
		virtual bool operator()(const Vector3r& pt,Real pad) const {return obj2pred(A)(pt,pad) && obj2pred(B)(pt,pad);}
		virtual py::tuple aabb() const {
			Vector3r minA,maxA,minB,maxB;
			ttuple2vvec(obj2pred(A).aabb(),minA,maxA);
			ttuple2vvec(obj2pred(B).aabb(),minB,maxB);
			return vvec2tuple(minA.cwiseMax(minB),maxA.cwiseMin(maxB));
		}
};
PredicateIntersection makeIntersection(const py::object& A, const py::object& B){ return PredicateIntersection(A,B);}

class PredicateDifference: public PredicateBoolean{
	public:
		PredicateDifference(const py::object _A, const py::object _B): PredicateBoolean(_A,_B){}
		virtual bool operator()(const Vector3r& pt,Real pad) const {return obj2pred(A)(pt,pad) && !obj2pred(B)(pt,-pad);}
		virtual py::tuple aabb() const { return obj2pred(A).aabb(); }
};
PredicateDifference makeDifference(const py::object& A, const py::object& B){ return PredicateDifference(A,B);}

class PredicateSymmetricDifference: public PredicateBoolean{
	public:
		PredicateSymmetricDifference(const py::object _A, const py::object _B): PredicateBoolean(_A,_B){}
		virtual bool operator()(const Vector3r& pt,Real pad) const {bool inA=obj2pred(A)(pt,pad), inB=obj2pred(B)(pt,pad); return (inA && !inB) || (!inA && inB);}
		virtual py::tuple aabb() const {
			Vector3r minA,maxA,minB,maxB;
			ttuple2vvec(obj2pred(A).aabb(),minA,maxA);
			ttuple2vvec(obj2pred(B).aabb(),minB,maxB);
			return vvec2tuple(minA.cwiseMin(minB),maxA.cwiseMax(maxB));
		}
};
PredicateSymmetricDifference makeSymmetricDifference(const py::object& A, const py::object& B){ return PredicateSymmetricDifference(A,B);}

/*********************************************************************************
****************************** Primitive predicates ******************************
*********************************************************************************/


/*! Disk predicate */
class inDisk: public Predicate {
	Vector3r center; Real radius;
public:
	inDisk(const Vector3r& _center, Real _radius){center=_center; radius=_radius;}
	virtual bool operator()(const Vector3r& pt, Real pad=0.) const { return ((pt-center).norm()-pad<=radius-pad); }
	virtual py::tuple aabb() const {return vvec2tuple(Vector3r(center[0]-radius,center[1]-radius,center[2]-radius),Vector3r(center[0]+radius,center[1]+radius,center[2]+radius));}
};

/*! Axis-aligned box predicate */
class inAlignedBox: public Predicate{
	Vector3r mn, mx;
public:
	inAlignedBox(const Vector3r& _mn, const Vector3r& _mx): mn(_mn), mx(_mx) {}
	virtual bool operator()(const Vector3r& pt, Real pad=0.) const {
		return
			mn[0]+pad<=pt[0] && mx[0]-pad>=pt[0] &&
			mn[1]+pad<=pt[1] && mx[1]-pad>=pt[1] &&
			mn[2]+pad<=pt[2] && mx[2]-pad>=pt[2];
	}
	virtual py::tuple aabb() const { return vvec2tuple(mn,mx); }
};

class inParallelepiped: public Predicate{
	Vector3r n[6]; // outer normals, for -x, +x, -y, +y, -z, +z
	Vector3r pts[6]; // points on planes
	Vector3r mn,mx;
public:
	inParallelepiped(const Vector3r& o, const Vector3r& a, const Vector3r& b, const Vector3r& c){
		Vector3r A(o), B(a), C(a+(b-o)), D(b), E(c), F(c+(a-o)), G(c+(a-o)+(b-o)), H(c+(b-o));
		Vector3r x(B-A), y(D-A), z(E-A);
		n[0]=-y.cross(z).normalized(); n[1]=-n[0]; pts[0]=A; pts[1]=B;
		n[2]=-z.cross(x).normalized(); n[3]=-n[2]; pts[2]=A; pts[3]=D;
		n[4]=-x.cross(y).normalized(); n[5]=-n[4]; pts[4]=A; pts[5]=E;
		// bounding box
		Vector3r vertices[8]={A,B,C,D,E,F,G,H};
		mn=mx=vertices[0];
		for(int i=1; i<8; i++){
			mn=mn.cwiseMin(vertices[i]);
			mx=mx.cwiseMax(vertices[i]);
		}
	}
	virtual bool operator()(const Vector3r& pt, Real pad=0.) const {
		for(int i=0; i<6; i++) if((pt-pts[i]).dot(n[i])>-pad) return false;
		return true;
	}
	virtual py::tuple aabb() const { return vvec2tuple(mn,mx); }
};

/*! Arbitrarily oriented cylinder predicate */
class inCylinder: public Predicate{
	Vector3r c1,c2,c12; Real radius,ht;
public:
	inCylinder(const Vector3r& _c1, const Vector3r& _c2, Real _radius){c1=_c1; c2=_c2; c12=c2-c1; radius=_radius; ht=c12.norm(); }
	bool operator()(const Vector3r& pt, Real pad=0.) const {
		Real u=(pt.dot(c12)-c1.dot(c12))/(ht*ht); // normalized coordinate along the c1--c2 axis
		if((u*ht<0+pad) || (u*ht>ht-pad)) return false; // out of cylinder along the axis
		Real axisDist=((pt-c1).cross(pt-c2)).norm()/ht;
		if(axisDist>radius-pad) return false;
		return true;
	}
	py::tuple aabb() const {
		// see http://www.gamedev.net/community/forums/topic.asp?topic_id=338522&forum_id=20&gforum_id=0 for the algorithm
		const Vector3r& A(c1); const Vector3r& B(c2);
		Vector3r k(
			sqrt((pow(A[1]-B[1],2)+pow(A[2]-B[2],2)))/ht,
			sqrt((pow(A[0]-B[0],2)+pow(A[2]-B[2],2)))/ht,
			sqrt((pow(A[0]-B[0],2)+pow(A[1]-B[1],2)))/ht);
		Vector3r mn=A.cwiseMin(B), mx=A.cwiseMax(B);
		return vvec2tuple((mn-radius*k).eval(),(mx+radius*k).eval());
	}
};

/*! Oriented hyperboloid predicate (cylinder as special case).

See http://mathworld.wolfram.com/Hyperboloid.html for the parametrization and meaning of symbols
*/
class inHyperboloid: public Predicate{
	Vector3r c1,c2,c12; Real R,a,ht,c;
public:
	inHyperboloid(const Vector3r& _c1, const Vector3r& _c2, Real _R, Real _r){
		c1=_c1; c2=_c2; R=_R; a=_r;
		c12=c2-c1; ht=c12.norm();
		Real uMax=sqrt(pow(R/a,2)-1); c=ht/(2*uMax);
	}
	// WARN: this is not accurate, since padding is taken as perpendicular to the axis, not the the surface
	bool operator()(const Vector3r& pt, Real pad=0.) const {
		Real v=(pt.dot(c12)-c1.dot(c12))/(ht*ht); // normalized coordinate along the c1--c2 axis
		if((v*ht<0+pad) || (v*ht>ht-pad)) return false; // out of cylinder along the axis
		Real u=(v-.5)*ht/c; // u from the wolfram parametrization; u is 0 in the center
		Real rHere=a*sqrt(1+u*u); // pad is taken perpendicular to the axis, not to the surface (inaccurate)
		Real axisDist=((pt-c1).cross(pt-c2)).norm()/ht;
		if(axisDist>rHere-pad) return false;
		return true;
	}
	py::tuple aabb() const {
		// the lazy way
		return inCylinder(c1,c2,R).aabb();
	}
};

/*! Axis-aligned ellipsoid predicate */
class inEllipsoid: public Predicate{
	Vector3r c, abc;
public:
	inEllipsoid(const Vector3r& _c, const Vector3r& _abc) {c=_c; abc=_abc;}
	bool operator()(const Vector3r& pt, Real pad=0.) const {
		//Define the ellipsoid X-coordinate of given Y and Z
		Real x = sqrt((1-pow((pt[1]-c[1]),2)/((abc[1]-pad)*(abc[1]-pad))-pow((pt[2]-c[2]),2)/((abc[2]-pad)*(abc[2]-pad)))*((abc[0]-pad)*(abc[0]-pad)))+c[0];
		Vector3r edgeEllipsoid(x,pt[1],pt[2]); // create a vector of these 3 coordinates
		//check whether given coordinates lie inside ellipsoid or not
		if ((pt-c).norm()<=(edgeEllipsoid-c).norm()) return true;
		else return false;
	}
	py::tuple aabb() const {
		const Vector3r& center(c); const Vector3r& ABC(abc);
		return vvec2tuple(Vector3r(center[0]-ABC[0],center[1]-ABC[1],center[2]-ABC[2]),Vector3r(center[0]+ABC[0],center[1]+ABC[1],center[2]+ABC[2]));
	}
};

/*! Negative notch predicate.

Use intersection (& operator) of another predicate with notInNotch to create notched solid.



		geometry explanation:

			c: the center
			normalHalfHt (in constructor): A-C
			inside: perpendicular to notch edge, points inside the notch (unit vector)
			normal: perpendicular to inside, perpendicular to both notch planes
			edge: unit vector in the direction of the edge

		          ↑ distUp        A
		-------------------------
		                        | C
		         inside(unit) ← * → distInPlane
		                        |
		-------------------------
		          ↓ distDown      B

*/
class notInNotch: public Predicate{
	Vector3r c, edge, normal, inside; Real aperture;
public:
	notInNotch(const Vector3r& _c, const Vector3r& _edge, const Vector3r& _normal, Real _aperture){
		c=_c;
		edge=_edge; edge.normalize();
		normal=_normal; normal-=edge*edge.dot(normal); normal.normalize();
		inside=edge.cross(normal);
		aperture=_aperture;
		// LOG_DEBUG("edge="<<edge<<", normal="<<normal<<", inside="<<inside<<", aperture="<<aperture);
	}
	bool operator()(const Vector3r& pt, Real pad=0.) const {
		Real distUp=normal.dot(pt-c)-aperture/2, distDown=-normal.dot(pt-c)-aperture/2, distInPlane=-inside.dot(pt-c);
		// LOG_DEBUG("pt="<<pt<<", distUp="<<distUp<<", distDown="<<distDown<<", distInPlane="<<distInPlane);
		if(distInPlane>=pad) return true;
		if(distUp     >=pad) return true;
		if(distDown   >=pad) return true;
		if(distInPlane<0) return false;
		if(distUp  >0) return sqrt(pow(distInPlane,2)+pow(distUp,2))>=pad;
		if(distDown>0) return sqrt(pow(distInPlane,2)+pow(distUp,2))>=pad;
		// between both notch planes, closer to the edge than pad (distInPlane<pad)
		return false;
	}
	// This predicate is not bounded, return infinities
	py::tuple aabb() const {
		Real inf=std::numeric_limits<Real>::infinity();
		return vvec2tuple(Vector3r(-inf,-inf,-inf),Vector3r(inf,inf,inf));
	}
};

#ifdef SUDODEM_GTS
extern "C" {
// HACK
#include"../3rd-party/pygts-0.3.1/pygts.h"

}
/* Helper function for inGtsSurface::aabb() */
static void vertex_aabb(GtsVertex *vertex, std::pair<Vector3r,Vector3r> *bb)
{
	GtsPoint *_p=GTS_POINT(vertex);
	Vector3r p(_p->x,_p->y,_p->z);
	bb->first=bb->first.cwiseMin(p);
	bb->second=bb->second.cwiseMax(p);
}

/*
This class plays tricks getting around pyGTS to get GTS objects and cache bb tree to speed
up point inclusion tests. For this reason, we have to link with _gts.so (see corresponding
SConscript file), which is at the same time the python module.
*/
class inGtsSurface: public Predicate{
	py::object pySurf; // to hold the reference so that surf is valid
	GtsSurface *surf;
	bool is_open, noPad, noPadWarned;
	GNode* tree;
public:
	inGtsSurface(py::object _surf, bool _noPad=false): pySurf(_surf), noPad(_noPad), noPadWarned(false) {
		if(!pygts_surface_check(_surf.ptr())) throw std::invalid_argument("Ctor must receive a gts.Surface() instance.");
		surf=PYGTS_SURFACE_AS_GTS_SURFACE(PYGTS_SURFACE(_surf.ptr()));
	 	if(!gts_surface_is_closed(surf)) throw std::invalid_argument("Surface is not closed.");
		is_open=gts_surface_volume(surf)<0.;
		if((tree=gts_bb_tree_surface(surf))==NULL) throw std::runtime_error("Could not create GTree.");
	}
	~inGtsSurface(){g_node_destroy(tree);}
	py::tuple aabb() const {
		Real inf=std::numeric_limits<Real>::infinity();
		std::pair<Vector3r,Vector3r> bb; bb.first=Vector3r(inf,inf,inf); bb.second=Vector3r(-inf,-inf,-inf);
		gts_surface_foreach_vertex(surf,(GtsFunc)vertex_aabb,&bb);
		return vvec2tuple(bb.first,bb.second);
	}
	bool ptCheck(const Vector3r& pt) const{
		GtsPoint gp; gp.x=pt[0]; gp.y=pt[1]; gp.z=pt[2];
		return (bool)gts_point_is_inside_surface(&gp,tree,is_open);
	}
	bool operator()(const Vector3r& pt, Real pad=0.) const {
		if(noPad){
			if(pad!=0. && noPadWarned) LOG_WARN("inGtsSurface constructed with noPad; requested non-zero pad set to zero.");
			return ptCheck(pt);
		}
		return ptCheck(pt) && ptCheck(pt-Vector3r(pad,0,0)) && ptCheck(pt+Vector3r(pad,0,0)) && ptCheck(pt-Vector3r(0,pad,0))&& ptCheck(pt+Vector3r(0,pad,0)) && ptCheck(pt-Vector3r(0,0,pad))&& ptCheck(pt+Vector3r(0,0,pad));
	}
	py::object surface() const {return pySurf;}
};

#endif

void pybind_init__packPredicates(pybind11::module& m){
	m.doc()="Spatial predicates for volumes (defined analytically or by triangulation).";
	SUDODEM_SET_DOCSTRING_OPTS;
	// base predicate class
	pybind11::class_<PredicateWrap, /* necessary, as methods are pure virtual*/ std::shared_ptr<PredicateWrap>>("Predicate")
		.def("__call__",pybind11::pure_virtual(&Predicate::operator()))
		.def("aabb",pybind11::pure_virtual(&Predicate::aabb))
		.def("dim",&Predicate::dim)
		.def("center",&Predicate::center)
		.def("__or__",makeUnion).def("__and__",makeIntersection).def("__sub__",makeDifference).def("__xor__",makeSymmetricDifference);
	// boolean operations
	pybind11::class_<PredicateBoolean,Predicate,std::shared_ptr<PredicateBoolean>>("PredicateBoolean","Boolean operation on 2 predicates (abstract class)")
		.def_property("A",&PredicateBoolean::getA).def_property("B",&PredicateBoolean::getB);
	pybind11::class_<PredicateUnion,PredicateBoolean,std::shared_ptr<PredicateUnion>>("PredicateUnion","Union (non-exclusive disjunction) of 2 predicates. A point has to be inside any of the two predicates to be inside. Can be constructed using the ``|`` operator on predicates: ``pred1 | pred2``.",pybind11::init<py::object,py::object>());
	pybind11::class_<PredicateIntersection,PredicateBoolean,std::shared_ptr<PredicateIntersection>>("PredicateIntersection","Intersection (conjunction) of 2 predicates. A point has to be inside both predicates. Can be constructed using the ``&`` operator on predicates: ``pred1 & pred2``.",pybind11::init<py::object,py::object>());
	pybind11::class_<PredicateDifference,PredicateBoolean,std::shared_ptr<PredicateDifference>>("PredicateDifference","Difference (conjunction with negative predicate) of 2 predicates. A point has to be inside the first and outside the second predicate. Can be constructed using the ``-`` operator on predicates: ``pred1 - pred2``.",pybind11::init<py::object,py::object>());
	pybind11::class_<PredicateSymmetricDifference,PredicateBoolean,std::shared_ptr<PredicateSymmetricDifference>>("PredicateSymmetricDifference","SymmetricDifference (exclusive disjunction) of 2 predicates. A point has to be in exactly one predicate of the two. Can be constructed using the ``^`` operator on predicates: ``pred1 ^ pred2``.",pybind11::init<py::object,py::object>());
	// primitive predicates
	pybind11::class_<inDisk,Predicate,std::shared_ptr<inDisk>>("inDisk","Disk predicate.",pybind11::init<const Vector3r&,Real>(pybind11::arg("center"),pybind11::arg("radius"),"Ctor taking center (as a 3-tuple) and radius"));
	pybind11::class_<inAlignedBox,Predicate,std::shared_ptr<inAlignedBox>>("inAlignedBox","Axis-aligned box predicate",pybind11::init<const Vector3r&,const Vector3r&>(pybind11::arg("minAABB"),pybind11::arg("maxAABB"),"Ctor taking minumum and maximum points of the box (as 3-tuples)."));
	pybind11::class_<inParallelepiped,Predicate,std::shared_ptr<inParallelepiped>>("inParallelepiped","Parallelepiped predicate",pybind11::init<const Vector3r&,const Vector3r&, const Vector3r&, const Vector3r&>(pybind11::arg("o"),pybind11::arg("a"),pybind11::arg("b"),pybind11::arg("c"),"Ctor taking four points: ``o`` (for origin) and then ``a``, ``b``, ``c`` which define endpoints of 3 respective edges from ``o``."));
	pybind11::class_<inCylinder,Predicate,std::shared_ptr<inCylinder>>("inCylinder","Cylinder predicate",pybind11::init<const Vector3r&,const Vector3r&,Real>(pybind11::arg("centerBottom"),pybind11::arg("centerTop"),pybind11::arg("radius"),"Ctor taking centers of the lateral walls (as 3-tuples) and radius."));
	pybind11::class_<inHyperboloid,Predicate,std::shared_ptr<inHyperboloid>>("inHyperboloid","Hyperboloid predicate",pybind11::init<const Vector3r&,const Vector3r&,Real,Real>(pybind11::arg("centerBottom"),pybind11::arg("centerTop"),pybind11::arg("radius"),pybind11::arg("skirt"),"Ctor taking centers of the lateral walls (as 3-tuples), radius at bases and skirt (middle radius)."));
	pybind11::class_<inEllipsoid,Predicate,std::shared_ptr<inEllipsoid>>("inEllipsoid","Ellipsoid predicate",pybind11::init<const Vector3r&,const Vector3r&>(pybind11::arg("centerPoint"),pybind11::arg("abc"),"Ctor taking center of the ellipsoid (3-tuple) and its 3 radii (3-tuple)."));
	pybind11::class_<notInNotch,Predicate,std::shared_ptr<notInNotch>>("notInNotch","Outside of infinite, rectangle-shaped notch predicate",pybind11::init<const Vector3r&,const Vector3r&,const Vector3r&,Real>(pybind11::arg("centerPoint"),pybind11::arg("edge"),pybind11::arg("normal"),pybind11::arg("aperture"),"Ctor taking point in the symmetry plane, vector pointing along the edge, plane normal and aperture size.\nThe side inside the notch is edge×normal.\nNormal is made perpendicular to the edge.\nAll vectors are normalized at construction time."));
	#ifdef SUDODEM_GTS
		pybind11::class_<inGtsSurface,Predicate,std::shared_ptr<inGtsSurface>>("inGtsSurface","GTS surface predicate",pybind11::init<py::object,pybind11::optional<bool>>(pybind11::arg("surface"),pybind11::arg("noPad"),"Ctor taking a gts.Surface() instance, which must not be modified during instance lifetime.\nThe optional noPad can disable padding (if set to True), which speeds up calls several times.\nNote: padding checks inclusion of 6 points along +- cardinal directions in the pad distance from given point, which is not exact."))
			.def_property("surf",&inGtsSurface::surface,"The associated gts.Surface object.");
	#endif
}
