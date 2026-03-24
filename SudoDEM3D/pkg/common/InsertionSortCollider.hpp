// 2009 © Václav Šmilauer <eudoxos@arcig.cz>
// 2013 © Bruno Chareyre <bruno.chareyre@hmg.inpg.fr>
//2019 @ Shiwei Zhao <ceswzhao@ust.hk>
#pragma once
#include<sudodem/pkg/common/Collider.hpp>
#include<sudodem/core/Scene.hpp>
class InteractionContainer;


/*! Periodic collider notes.

Architecture
============
Values from bounding boxes are added information about period in which they are.
Their container (VecBounds) holds position of where the space wraps.
The sorting algorithm is changed in such way that periods are changed when body crosses cell boundary.

Interaction::cellDist holds information about relative cell coordinates of the 2nd body
relative to the 1st one. Dispatchers (IGeomDispatcher and InteractionLoop)
use this information to pass modified position of the 2nd body to IGeomFunctors.
Since properly behaving IGeomFunctor's and LawFunctor's do not take positions
directly from Body::state, the interaction is computed with the periodic positions.

Positions of bodies (in the sense of Body::state) and their natural bboxes are not wrapped
to the periodic cell, they can be anywhere (but not "too far" in the sense of int overflow).

Since Interaction::cellDists holds cell coordinates, it is possible to change the cell boundaries
at runtime. This should make it easy to implement some stress control on the top.

Clumps do not interfere with periodicity in any way.

Rendering
---------
OpenGLRenderer renders Shape at all periodic positions that touch the
periodic cell (i.e. Bounds crosses its boundary).

It seems to affect body selection somehow, but that is perhaps not related at all.

Periodicity control
===================
c++:
	Scene::isPeriodic, Scene::cellSize
python:
	O.periodicCell=((0,0,0),(10,10,10)  # activates periodic boundary
	O.periodicCell=() # deactivates periodic boundary

Requirements
============
* By default, no body can have Aabb larger than about .499*cellSize. Exception is thrown if that is false.
	Large bodies are accepted if allowBiggerThanPeriod (experimental)
* Constitutive law must not get body positions from Body::state directly.
	If it does, it uses Interaction::cellDist to compute periodic position.
* No body can get further away than MAXINT periods. It will do horrible things if there is overflow. Not checked at the moment.
* Body cannot move over distance > cellSize in one step. Since body size is limited as well, that would mean simulation explosion.
	Exception is thrown if the movement is upwards. If downwards, it is not handled at all.

Possible performance improvements & bugs
========================================

* PeriodicInsertionSortCollider::{cellWrap,cellWrapRel} OpenGLRenderer::{wrapCell,wrapCellPt} Shop::PeriodicWrap
	are all very similar functions. They should be put into separate header and included from all those places.

*/


// #define this macro to enable timing within this engine
// #define ISC_TIMING

// #define to turn on some tracing information for the periodic part

#ifdef ISC_TIMING
	#define ISC_CHECKPOINT(cpt) timingDeltas->checkpoint(cpt)
#else
	#define ISC_CHECKPOINT(cpt)
#endif

class NewtonIntegrator;

class Integrator;

class GeneralIntegratorInsertionSortCollider;// Forward decleration of child to decleare it as friend

class InsertionSortCollider: public Collider{

	friend class GeneralIntegratorInsertionSortCollider;

	//! struct for storing bounds of bodies
	struct Bounds{
		//! coordinate along the given sortAxis
		Real coord;
		//! id of the body this bound belongs to
		Body::id_t id;
		//! periodic cell coordinate
		int period;
		//! is it the minimum (true) or maximum (false) bound?
		struct{ unsigned hasBB:1; unsigned isMin:1; } flags;
		Bounds(Real coord_, Body::id_t id_, bool isMin): coord(coord_), id(id_), period(0){ flags.isMin=isMin; }
		bool operator<(const Bounds& b) const {
			/* handle special case of zero-width bodies, which could otherwise get min/max swapped in the unstable std::sort */
			if(id==b.id && coord==b.coord) return flags.isMin;
			return coord<b.coord;
		}
		bool operator>(const Bounds& b) const {
			if(id==b.id && coord==b.coord) return !flags.isMin;
			return coord>b.coord;
		}
	};

		// we need this to find out about current maxVelocitySq
		shared_ptr<NewtonIntegrator> newton;
		// if False, no type of striding is used
		// if True, then either verletDist XOR nBins is set
		bool strideActive;
	struct VecBounds{
		// axis set in the ctor
		int axis;
		std::vector<Bounds> vec;
		Real cellDim;
		// cache vector size(), update at every step in action()
		long size;
		// index of the lowest coordinate element, before which the container wraps
		long loIdx;
		Bounds& operator[](long idx){ assert(idx<size && idx>=0); return vec[idx]; }
		const Bounds& operator[](long idx) const { assert(idx<size && idx>=0); return vec[idx]; }
		// update number of bodies, periodic properties and size from Scene
		void updatePeriodicity(Scene* scene){
			assert(scene->isPeriodic);
			assert(axis>=0 && axis <=1);
			cellDim=scene->cell->getSize()[axis];
		}
		// normalize given index to the right range (wraps around)
		long norm(long i) const { if(i<0) i+=size; long ret=i%size; assert(ret>=0 && ret<size); return ret;}
		VecBounds(): axis(-1), size(0), loIdx(0){}
		void dump(ostream& os){ string ret; for(size_t i=0; i<vec.size(); i++) os<<((long)i==loIdx?"@@ ":"")<<vec[i].coord<<"(id="<<vec[i].id<<","<<(vec[i].flags.isMin?"min":"max")<<",p"<<vec[i].period<<") "; os<<endl;}
	};
	private:
	//! storage for bounds
	VecBounds BB[2];
	//! storage for bb maxima and minima
	std::vector<Real> maxima, minima;
	//! Store inverse sizes to avoid repeated divisions within loops
  Vector3r invSizes;
	//! Whether the Scene was periodic (to detect the change, which shouldn't happen, but shouldn't crash us either)
	bool periodic;

	// return python representation of the BB struct, as ([...],[...],[...]).
  pybind11::tuple dumpBounds();

	/*! sorting routine; insertion sort is very fast for strongly pre-sorted lists, which is our case
  	    http://en.wikipedia.org/wiki/Insertion_sort has the algorithm and other details
	*/
	void insertionSort(VecBounds& v,InteractionContainer*,Scene*,bool doCollide=true);
	void insertionSortParallel(VecBounds& v,InteractionContainer*,Scene*,bool doCollide=true);
	void handleBoundInversion(Body::id_t,Body::id_t,InteractionContainer*,Scene*);
// 	bool spatialOverlap(Body::id_t,Body::id_t) const;

	// periodic variants
	void insertionSortPeri(VecBounds& v,InteractionContainer*,Scene*,bool doCollide=true);
	void handleBoundInversionPeri(Body::id_t,Body::id_t,InteractionContainer*,Scene*);
	void handleBoundSplit(Body::id_t,Body::id_t,InteractionContainer*,Scene*);

	bool spatialOverlapPeri(Body::id_t,Body::id_t,Scene*,Vector3i&) const;
	inline bool spatialOverlap(const Body::id_t& id1, const Body::id_t& id2) const {
	assert(!periodic);
	return	(minima[2*id1+0]<=maxima[2*id2+0]) && (maxima[2*id1+0]>=minima[2*id2+0]) &&
		(minima[2*id1+1]<=maxima[2*id2+1]) && (maxima[2*id1+1]>=minima[2*id2+1]);
	}

	static Real cellWrap(const Real, const Real, const Real, int&);
	static Real cellWrapRel(const Real, const Real, const Real);


	public:
	// Attributes
	int sortAxis;
	bool allowBiggerThanPeriod;
	bool sortThenCollide;
	int targetInterv;
	Real updatingDispFactor;
	Real verletDist;
	Real minSweepDistFactor;
	Real fastestBodyMaxDist;
	int numReinit;
	Real useless;
	bool doSort;

	// Constructor
	InsertionSortCollider();
	
	//! Predicate called from loop within InteractionContainer::erasePending
	bool shouldBeErased(Body::id_t id1, Body::id_t id2, Scene* rb) const {
		if(!periodic) return !spatialOverlap(id1,id2);
		else { Vector3i periods; return !spatialOverlapPeri(id1,id2,rb,periods); }
	}
	virtual bool isActivated() override;

	// force reinitialization at next run
	virtual void invalidatePersistentData() override{ for(int i=0; i<3; i++){ BB[i].vec.clear(); BB[i].size=0; }}

	vector<Body::id_t> probeBoundingVolume(const Bound&) override;

	virtual void action() override;
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Collider, sortAxis, allowBiggerThanPeriod, sortThenCollide, targetInterv, updatingDispFactor, verletDist, minSweepDistFactor, fastestBodyMaxDist, numReinit, doSort);
	
	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(InsertionSortCollider, Collider);
