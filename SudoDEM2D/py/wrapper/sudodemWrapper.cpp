// 2007,2008 © Václav Šmilauer <eudoxos@arcig.cz>

#include <sudodem/lib/base/Math.hpp>
#include <list>
#include <signal.h>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <functional>
#include <thread>
#include <chrono>
#include <algorithm>
#include <limits>

#include<sudodem/lib/base/Logging.hpp>
#include<sudodem/lib/pyutil/gil.hpp>
#include<sudodem/lib/pyutil/doc_opts.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/ThreadRunner.hpp>
#include<sudodem/core/FileGenerator.hpp>
#include<sudodem/core/EnergyTracker.hpp>

// Include headers for IGeom and IPhys classes that need readable names for ClassFactory
#include<sudodem/pkg/dem/ScGeom.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>

// Explicit template instantiation of Singleton<ClassRegistry>::self
// This must be in each shared library that uses ClassRegistry
#include<sudodem/lib/factory/ClassRegistry.hpp>
template class Singleton<ClassRegistry>;

// Forward declaration for registering Engine base classes
extern "C" void register_engine_base_classes(pybind11::module_ m);

// Forward declaration for core class registration
extern "C" void initCoreClassRegistry();

// Forward declaration for Superellipse class registration
extern "C" void register_superellipse_classes();

// Forward declaration for Engine classes registration
extern "C" void register_engine_classes();

// Forward declaration for Shape and Material classes registration
extern "C" void register_shape_material_classes();

// Forward declaration for common functors registration
extern "C" void register_common_functors();

// Forward declaration for IGeom and IPhys classes registration
extern "C" void register_geom_phys_classes();

// Forward declaration for core extra classes registration
extern "C" void register_core_extra_classes();

// Forward declarations for submodule initialization functions
void pybind_init_WeightedAverage2d(pybind11::module& m);
void pybind_init__utils(pybind11::module& m);
void pybind_init__customConverters(pybind11::module& m);
void pybind_init__superellipse_utils(pybind11::module& m);

// Core class headers for pybind11 registration
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/core/Engine.hpp>
#include<sudodem/core/Functor.hpp>
#include<sudodem/core/Dispatcher.hpp>
#include<sudodem/core/Bound.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/pkg/common/Wall.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/Material.hpp>

// Engine class headers for manual registration
#include<sudodem/pkg/dem/NewtonIntegrator.hpp>
#include<sudodem/pkg/common/ForceResetter.hpp>
#include<sudodem/pkg/common/InsertionSortCollider.hpp>
#include<sudodem/pkg/common/InteractionLoop.hpp>
#include<sudodem/core/Body.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/dem/FrictPhys.hpp>
#include<sudodem/pkg/common/NormShearPhys.hpp>
#include<sudodem/core/IPhys.hpp>
#include<sudodem/pkg/dem/Ig2_Basic_ScGeom.hpp>
#include<sudodem/pkg/dem/ElasticContactLaw.hpp>

//#include<sudodem/pkg/dem/STLImporter.hpp>

#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/pkg/common/ParallelEngine.hpp>
#include<sudodem/pkg/common/Collider.hpp>

#include<sudodem/pkg/common/InteractionLoop.hpp>



// Include Superellipse classes for manual registration

#include<sudodem/pkg/dem/Superellipse.hpp>
#include<sudodem/pkg/dem/Superellipse_Ig2.hpp>

#include <sudodem/core/Clump.hpp>

#include <sudodem/pkg/common/Disk.hpp>

#include <locale>
#include <random>
#include <functional>
#include <functional>
#include <ctime>
#include <random>
#include <functional>
#include <functional>
#include <ctime>

#include <sudodem/core/Timing.hpp>
#include <sudodem/lib/serialization/ObjectIO.hpp>

namespace py = pybind11;

/*
Python normally iterates over object it is has __getitem__ and __len__, which BodyContainer does.
However, it will not skip removed bodies automatically, hence this iterator which does just that.
*/
class pyBodyIterator{
	BodyContainer::iterator I, Iend;
	public:
	pyBodyIterator(const shared_ptr<BodyContainer>& bc){ I=bc->begin(); Iend=bc->end(); }
	pyBodyIterator pyIter(){return *this;}
	shared_ptr<Body> pyNext(){
		BodyContainer::iterator ret;
		while(I!=Iend){ ret=I; ++I; if(*ret) return *ret; }
		PyErr_SetNone(PyExc_StopIteration); throw pybind11::error_already_set();; /* never reached, but makes the compiler happier */ throw;
	}
};

class pyBodyContainer{
	private:
		void checkClump(shared_ptr<Body> b){
			if (!(b->isClump())){
				PyErr_SetString(PyExc_TypeError,("Error: Body"+std::to_string(b->getId())+" is not a clump.").c_str());
				throw pybind11::error_already_set();;
			}
		}
		typedef std::map<Body::id_t,Se2r> MemberMap;
	public:
	const shared_ptr<BodyContainer> proxee;
	pyBodyIterator pyIter(){return pyBodyIterator(proxee);}
	pyBodyContainer(const shared_ptr<BodyContainer>& _proxee): proxee(_proxee){}
	shared_ptr<Body> pyGetitem(Body::id_t _id){
		int id=(_id>=0 ? _id : proxee->size()+_id);
		if(id<0 || (size_t)id>=proxee->size()){ PyErr_SetString(PyExc_IndexError, "Body id out of range."); throw pybind11::error_already_set();; /* make compiler happy; never reached */ return shared_ptr<Body>(); }
		else return (*proxee)[id];
	}
	Body::id_t append(shared_ptr<Body> b){
		// shoud be >=0, but Body is by default created with id 0... :-|
		if(b->getId()>=0){ PyErr_SetString(PyExc_IndexError,("Body already has id "+std::to_string(b->getId())+" set; appending such body (for the second time) is not allowed.").c_str()); throw pybind11::error_already_set();; }
		return proxee->insert(b);
	}
	vector<Body::id_t> appendList(vector<shared_ptr<Body> > bb){
		// prevent crash when adding lots of bodies - use lock for thread safety
		std::lock_guard<std::timed_mutex> lock(Omega::instance().renderMutex);
		vector<Body::id_t> ret; for (shared_ptr<Body>& b : bb){ret.push_back(append(b));} return ret;
	}
	Body::id_t clump(vector<Body::id_t> ids, unsigned int discretization){
		// create and add clump itself
		Scene* scene(Omega::instance().getScene().get());
		shared_ptr<Body> clumpBody=shared_ptr<Body>(new Body());
		shared_ptr<Clump> clump=shared_ptr<Clump>(new Clump());
		clumpBody->shape=clump;
		clumpBody->setBounded(false);
		proxee->insert(clumpBody);
		// add clump members to the clump
		for (Body::id_t id : ids) {
			if (Body::byId(id,scene)->isClumpMember()){	//Check, whether the body is clumpMember
				Clump::del(Body::byId(Body::byId(id,scene)->clumpId,scene),Body::byId(id,scene)); //If so, remove it from there
			}
		};

		for (Body::id_t id : ids) Clump::add(clumpBody,Body::byId(id,scene));
		Clump::updateProperties(clumpBody, discretization);
		return clumpBody->getId();
	}
	pybind11::tuple appendClump(vector<shared_ptr<Body> > bb, unsigned int discretization){
		// append constituent particles
		vector<Body::id_t> ids(appendList(bb));
		// clump them together (the clump fcn) and return
		return pybind11::make_tuple(clump(ids, discretization),ids);
	}
	void updateClumpProperties(pybind11::list excludeList,unsigned int discretization){
		//convert excludeList to a c++ list
		vector<Body::id_t> excludeListC;
		for (int ii = 0; ii < pybind11::len(excludeList); ii++) excludeListC.push_back(pybind11::cast<Body::id_t>(excludeList[ii]));
		for (const shared_ptr<Body>& b : *proxee){
			if ( !(std::find(excludeListC.begin(), excludeListC.end(), b->getId()) != excludeListC.end()) ) {
				if (b->isClump()) Clump::updateProperties(b, discretization);
			}
		}
	}
	void addToClump(vector<Body::id_t> bids, Body::id_t cid, unsigned int discretization){
		Scene* scene(Omega::instance().getScene().get());	// get scene
		shared_ptr<Body> clp = Body::byId(cid,scene);		// get clump pointer
		checkClump(clp);
		vector<Body::id_t> eraseList;
		for (Body::id_t bid : bids) {
			shared_ptr<Body> bp = Body::byId(bid,scene);		// get body pointer
			if (bp->isClump()){
				if (bp == clp) {PyErr_Warn(PyExc_UserWarning,("Warning: Body "+std::to_string(bid)+" and clump "+std::to_string(cid)+" are the same bodies. Body was not added.").c_str()); return;}
				Clump::add(clp,bp);//add clump bid to clump cid
				eraseList.push_back(bid);
			}
			else if (bp->isClumpMember()){
				Body::id_t bpClumpId = bp->clumpId;
				shared_ptr<Body> bpClumpPointer = Body::byId(bpClumpId,scene);
				if (bpClumpPointer == clp) {PyErr_Warn(PyExc_UserWarning,("Warning: Body "+std::to_string(bid)+" is already a clump member of clump "+std::to_string(cid)+". Body was not added.").c_str()); return;}
				Clump::add(clp,bpClumpPointer);//add clump bpClumpId to clump cid
				eraseList.push_back(bpClumpId);
			}
			else Clump::add(clp,bp);// bp must be a standalone!
		}
		Clump::updateProperties(clp, discretization);
		for (Body::id_t bid : eraseList) proxee->erase(bid,false);//erase old clumps
	}
	void releaseFromClump(Body::id_t bid, Body::id_t cid, unsigned int discretization){
		Scene* scene(Omega::instance().getScene().get());	// get scene
		shared_ptr<Body> bp = Body::byId(bid,scene);		// get body pointer
		shared_ptr<Body> clp = Body::byId(cid,scene);		// get clump pointer
		checkClump(clp);
		if (bp->isClumpMember()){
			Body::id_t bpClumpId = bp->clumpId;
			if (cid == bpClumpId){
				const shared_ptr<Clump>& clump=SUDODEM_PTR_CAST<Clump>(clp->shape);
				std::map<Body::id_t,Se2r>& members = clump->members;
				if (members.size() == 2) {PyErr_Warn(PyExc_UserWarning,("Warning: Body "+std::to_string(bid)+" not released from clump "+std::to_string(cid)+", because number of clump members would get < 2!").c_str()); return;}
				Clump::del(clp,bp);//release bid from cid
				Clump::updateProperties(clp, discretization);
			} else { PyErr_Warn(PyExc_UserWarning,("Warning: Body "+std::to_string(bid)+" must be a clump member of clump "+std::to_string(cid)+". Body was not released.").c_str()); return;}
		} else { PyErr_Warn(PyExc_UserWarning,("Warning: Body "+std::to_string(bid)+" is not a clump member. Body was not released.").c_str()); return;}
	}
	pybind11::list replaceByClumps(pybind11::list ctList, vector<Real> amounts, unsigned int discretization){
		pybind11::list ret;
		Real checkSum = 0.0;
		for (Real amount : amounts) {
			if (amount < 0.0) {
				PyErr_SetString(PyExc_ValueError,("Error: One or more of given amounts are negative!"));
				throw pybind11::error_already_set();;
			}
			else checkSum += amount;
		}
		if (checkSum > 1.0){
			PyErr_SetString(PyExc_ValueError,("Error: Sum of amounts "+std::to_string(checkSum)+" should not be bigger than 1.0!").c_str());
			throw pybind11::error_already_set();;
		}
		if (pybind11::len(ctList) != (unsigned) amounts.size()) {//avoid unsigned comparison warning
			PyErr_SetString(PyExc_ValueError,("Error: Length of amounts list ("+std::to_string(amounts.size())+") differs from length of template list ("+std::to_string(pybind11::len(ctList))+").").c_str());
			throw pybind11::error_already_set();;
		}
		//set a random generator (code copied from pkg/dem/DiskPack.cpp):
		static std::mt19937 randGen(static_cast<unsigned int>(std::time(nullptr)));
		static std::uniform_real_distribution<> randDist(-1, 1);

		//get number of spherical particles and a list of all disks:
		vector<shared_ptr<Body> > diskList;
		shared_ptr<Disk> sph (new Disk);
		int Sph_Index = sph->getClassIndexStatic();
		for (const shared_ptr<Body>& b : *proxee) if ( (b->shape->getClassIndex() == Sph_Index) && (b->isStandalone()) ) diskList.push_back(b);
		int num = diskList.size();

		//loop over templates:
		int numDiskList = num, numTemplates = amounts.size();
		for (int ii = 0; ii < numTemplates; ii++) {
			//ctList: [<ct1>,<ct2>, ...] = [<int,[double,double, ... ],[Vector3r,Vector3r, ...]>,<int,[double,double, ... ],[Vector3r,Vector3r, ...]>, ...]
			//ct: <len(relRadList),relRadList,relPosList> = <int,[double,double, ... ],[Vector3r,Vector3r, ...]> (python objects)
			//relRadList: [relRad1,relRad2, ...] (list of doubles)
			//relPosList: [relPos1,relPos2, ...] (list of vectors)

			//extract attributes from python objects:
			pybind11::object ctTmp = ctList[ii];
			int numCM = pybind11::cast<int>(ctTmp.attr("numCM")); // number of clump members
			pybind11::list relRadListTmp = pybind11::cast<pybind11::list>(ctTmp.attr("relRadii"));
			pybind11::list relPosListTmp = pybind11::cast<pybind11::list>(ctTmp.attr("relPositions"));

			//get relative radii and positions; calculate volumes; get balance point: get axis aligned bounding box; get minimum radius;
			vector<Real> relRadTmp(numCM), relVolTmp(numCM);
			vector<Vector2r> relPosTmp(numCM);
			Vector2r relPosTmpMean = Vector2r::Zero();
			Real rMin=std::numeric_limits<Real>::infinity(); AlignedBox2r aabb;
			for (int jj = 0; jj < numCM; jj++) {
				relRadTmp[jj] = pybind11::cast<Real>(relRadListTmp[jj]);
				relVolTmp[jj] = (4./3.)*Mathr::PI*pow(relRadTmp[jj],3.);
				relPosTmp[jj] = pybind11::cast<Vector2r>(relPosListTmp[jj]);
				relPosTmpMean += relPosTmp[jj];
				aabb.extend(relPosTmp[jj] + Vector2r::Constant(relRadTmp[jj]));
				aabb.extend(relPosTmp[jj] - Vector2r::Constant(relRadTmp[jj]));
				rMin=min(rMin,relRadTmp[jj]);
			}
			relPosTmpMean /= numCM;//balance point

			//get volume of the clump template using regular cubic cell array inside axis aligned bounding box of the clump:
			//(some parts are duplicated from intergration algorithm in Clump::updateProperties)
			Real dx = rMin/5.; 	//edge length of cell
			Real aabbMax = max(aabb.max().x()-aabb.min().x(),aabb.max().y()-aabb.min().y());
			if (aabbMax/dx > 150) dx = aabbMax/150;//limit dx
			Real dv = pow(dx,3);		//volume of a single cell
			Vector2r x;			//position vector (center) of cell
			Real relVolSumTmp = 0.0;	//volume of clump template
			for(x.x()=aabb.min().x()+dx/2.; x.x()<aabb.max().x(); x.x()+=dx){
				for(x.y()=aabb.min().y()+dx/2.; x.y()<aabb.max().y(); x.y()+=dx){
						for (int jj = 0; jj < numCM; jj++) {
							if((x-relPosTmp[jj]).squaredNorm() < pow(relRadTmp[jj],2)){ relVolSumTmp += dv; break; }
						}

				}
			}
			/**
			### old method, not working for multiple overlaps:
			//check for overlaps and correct volumes (-= volume of spherical caps):
			Real distCMTmp, overlapTmp, hCapjj, hCapkk;
			for (int jj = 0; jj < numCM; jj++) {
				for (int kk = jj; kk < numCM; kk++) {
					if (jj != kk) {
						distCMTmp = (relPosTmp[jj] - relPosTmp[kk]).norm(); //distance between two disks
						overlapTmp = (relRadTmp[jj] + relRadTmp[kk]) - distCMTmp;//positive if overlapping ...
						if (overlapTmp > 0.0) {//calculation of overlapping disks, see http://mathworld.wolfram.com/Disk-DiskIntersection.html
							hCapjj = relRadTmp[jj] - (distCMTmp*distCMTmp - relRadTmp[kk]*relRadTmp[kk] + relRadTmp[jj]*relRadTmp[jj])/(2*distCMTmp);
							hCapkk = relRadTmp[kk] - (distCMTmp*distCMTmp - relRadTmp[jj]*relRadTmp[jj] + relRadTmp[kk]*relRadTmp[kk])/(2*distCMTmp);
							//calculation of spherical cap, see http://en.wikipedia.org/wiki/Spherical_cap
							relVolTmp[jj] -= (1./3.)*Mathr::PI*hCapjj*hCapjj*(3.*relRadTmp[jj] - hCapjj);// correct relative volumes
							relVolTmp[kk] -= (1./3.)*Mathr::PI*hCapkk*hCapkk*(3.*relRadTmp[kk] - hCapkk);
						}
					}
				}
			}
			//get relative volume of the clump:
			for (int jj = 0; jj < numCM; jj++) relVolSumTmp += relVolTmp[jj];
			**/

			//get pointer lists of disks, that should be replaced:
			int numReplaceTmp = round(num*amounts[ii]);
			vector<shared_ptr<Body> > bpListTmp(numReplaceTmp);
			int a = 0, c = 0;//counters
			vector<int> posTmp;
			for (const shared_ptr<Body>& b : diskList) {
				if (c == a*numDiskList/numReplaceTmp) {
					bpListTmp[a] = b; a++;
					posTmp.push_back(c);//remember position in diskList
				} c++;
			}
			for (int jj = 0; jj < a; jj++) {
				diskList.erase(diskList.begin()+posTmp[jj]-jj);//remove bodies from diskList, that were already found
				numDiskList--;
			}

			//adapt position- and radii-informations and replace disks from bpListTmp by clumps:
			#ifdef SUDODEM_OPENMP
			omp_lock_t locker;
			omp_init_lock(&locker);//since bodies are created and deleted in following sections, it is neccessary to lock critical parts of the code (avoid seg fault)
			#pragma omp parallel for schedule(dynamic) shared(locker)
			for(int i=0; i<numReplaceTmp; i++) {

				while (! omp_test_lock(&locker)) 
				#ifdef _WIN32
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				#else
					usleep(1);
				#endif

				const shared_ptr<Body>& b = bpListTmp[i];
				LOG_DEBUG("replaceByClumps: Started processing body "<<bpListTmp[i]->id<<" in parallel ...");
			#else
			for (const shared_ptr<Body>& b : bpListTmp) {
			#endif
				//get disk, that should be replaced:
				const Disk* disk = SUDODEM_CAST<Disk*> (b->shape.get());
				shared_ptr<Material> matTmp = b->material;

				//get a random rotation quaternion:
				//Quaternionr randAxisTmp = (Quaternionr) AngleAxisr(2*Mathr::PI*randGen(),Vector2r(randGen(),randGen());
				//randAxisTmp.normalize();

				//convert geometries in global coordinates (scaling):
				Real scalingFactorVolume = ((4./3.)*Mathr::PI*pow(disk->radius,3.))/relVolSumTmp;
				Real scalingFactor1D = pow(scalingFactorVolume,1./3.);//=((vol. disk)/(relative clump volume))^(1/3)
				vector<Vector2r> newPosTmp(numCM);
				vector<Real> newRadTmp(numCM);
				vector<Body::id_t> idsTmp(numCM);
				for (int jj = 0; jj < numCM; jj++) {
					newPosTmp[jj] = relPosTmp[jj] - relPosTmpMean;	//shift position, to get balance point at (0,0,0)
					newPosTmp[jj] =/* randAxisTmp*/newPosTmp[jj];	//rotate around balance point
					newRadTmp[jj] = relRadTmp[jj] * scalingFactor1D;//scale radii
					newPosTmp[jj] = newPosTmp[jj] * scalingFactor1D;//scale position
					newPosTmp[jj] += b->state->pos;			//translate new position to disks center

					//create disks:
					shared_ptr<Body> newDisk = shared_ptr<Body>(new Body());
					newDisk->state->blockedDOFs = State::DOF_NONE;
					newDisk->state->mass = scalingFactorVolume*relVolTmp[jj]*matTmp->density;//vol. corrected mass for clump members
					Real inertiaTmp = 2.0/5.0*newDisk->state->mass*newRadTmp[jj]*newRadTmp[jj];
					newDisk->state->inertia	= inertiaTmp;
					newDisk->state->pos = newPosTmp[jj];
					newDisk->material = matTmp;

					shared_ptr<Disk> diskTmp = shared_ptr<Disk>(new Disk());
					diskTmp->radius = newRadTmp[jj];
					diskTmp->color = Vector3r(Mathr::UnitRandom(),Mathr::UnitRandom(),Mathr::UnitRandom());
					diskTmp->color.normalize();
					newDisk->shape = diskTmp;

					shared_ptr<Aabb> aabbTmp = shared_ptr<Aabb>(new Aabb());
					aabbTmp->color = Vector3r(0,1,0);
					newDisk->bound = aabbTmp;
					proxee->insert(newDisk);
					LOG_DEBUG("New body (disk) "<<newDisk->id<<" added.");
					idsTmp[jj] = newDisk->id;
				}
				//cout << "thread " << omp_get_thread_num() << " unsets locker" << endl;
				#ifdef SUDODEM_OPENMP
				omp_unset_lock(&locker);//end of critical section
				#endif
				Body::id_t newClumpId = clump(idsTmp, discretization);
				ret.append(pybind11::make_tuple(newClumpId,idsTmp));
				erase(b->id,false);
			}
		}
		return ret;
	}
	Real getRoundness(pybind11::list excludeList){
		Scene* scene(Omega::instance().getScene().get());	// get scene
		shared_ptr<Disk> sph (new Disk);
		int Sph_Index = sph->getClassIndexStatic();		// get disk index for checking if bodies are disks
		//convert excludeList to a c++ list
		vector<Body::id_t> excludeListC;
		for (int ii = 0; ii < pybind11::len(excludeList); ii++) excludeListC.push_back(pybind11::cast<Body::id_t>(excludeList[ii]));
		Real RC_sum = 0.0;	//sum of local roundnesses
		Real R1, R2, vol, dens;
		int c = 0;		//counter
		for (const shared_ptr<Body>& b : *proxee){
			if ( !(std::find(excludeListC.begin(), excludeListC.end(), b->getId()) != excludeListC.end()) ) {
				if ((b->shape->getClassIndex() ==  Sph_Index) && (b->isStandalone())) { RC_sum += 1.0; c += 1; }
				if (b->isClump()){
					R2 = 0.0; dens = 0.0; vol = 0.0;
					const shared_ptr<Clump>& clump=SUDODEM_PTR_CAST<Clump>(b->shape);
					std::map<Body::id_t,Se2r>& members = clump->members;
					for (MemberMap::value_type& mm : members){
						const Body::id_t& memberId=mm.first;
						const shared_ptr<Body>& member=Body::byId(memberId,scene);
						assert(member->isClumpMember());
						if (member->shape->getClassIndex() ==  Sph_Index){//clump member should be a disk
							const Disk* disk = SUDODEM_CAST<Disk*> (member->shape.get());
							R2 = max((member->state->pos - b->state->pos).norm() + disk->radius, R2);	//get minimum radius of a disk, that imbeds clump
							dens = member->material->density;
						}
					}
					if (dens > 0.) vol = b->state->mass/dens;
					R1 = pow((3.*vol)/(4.*Mathr::PI),1./3.);	//get theoretical radius of a disk, with same volume as clump
					if (R2 < R1) {PyErr_Warn(PyExc_UserWarning,("Something went wrong in getRoundness method (R2 < R1 detected).")); return 0;}
					RC_sum += R1/R2; c += 1;
				}
			}
		}
		if (c == 0) c = 1;	//in case no disks and no clumps are present in the scene: RC = 0
		return RC_sum/c;	//return roundness coefficient RC
	}
	vector<Body::id_t> replace(vector<shared_ptr<Body> > bb){proxee->clear(); return appendList(bb);}
	long length(){return proxee->size();}
	void clear(){proxee->clear();}
	bool erase(Body::id_t id, bool eraseClumpMembers){ return proxee->erase(id,eraseClumpMembers); }
};


/////////////////////////////////////
class pyTags{
	public:
		pyTags(const shared_ptr<Scene> _mb): mb(_mb){}
		const shared_ptr<Scene> mb;
		bool hasKey(const string& key){ for (string val : mb->tags){ if(val.rfind(key+"=") == 0){ return true;} } return false; }
		string getItem(const string& key){
			for (string& val : mb->tags){
				if(val.rfind(key+"=") == 0){ string val1(val); val1 = val1.substr(key.size()+1); return val1;}
			}
			PyErr_SetString(PyExc_KeyError,("Invalid key: "+key+".").c_str());
			throw pybind11::error_already_set();; /* make compiler happy; never reached */ return string();
		}
		void setItem(const string& key,const string& item){
			if(key.find("=")!=string::npos) {
				PyErr_SetString(PyExc_KeyError, "Key must not contain the '=' character (implementation limitation; sorry).");
				throw pybind11::error_already_set();;
			}
			for (string& val : mb->tags){if(val.rfind(key+"=") == 0){ val=key+"="+item; return; } }
			mb->tags.push_back(key+"="+item);
			}
		pybind11::list keys(){
			pybind11::list ret;
			for (string val : mb->tags){
				size_t i=val.find("=");
				if(i==string::npos) throw runtime_error("Tags must be in the key=value format (internal error?)");
				val = val.substr(0, val.size() - val.size()-i); ret.append(val);
			}
			return ret;
		}
};


class pyInteractionIterator{
	InteractionContainer::iterator I, Iend;
	public:
	pyInteractionIterator(const shared_ptr<InteractionContainer>& ic){ I=ic->begin(); Iend=ic->end(); }
	pyInteractionIterator pyIter(){return *this;}
	shared_ptr<Interaction> pyNext(){
		InteractionContainer::iterator ret;
		while(I!=Iend){ ret=I; ++I; if((*ret)->isReal()) return *ret; }
		PyErr_SetNone(PyExc_StopIteration); throw pybind11::error_already_set();;
		throw; // to avoid compiler warning; never reached
		//InteractionContainer::iterator ret=I; ++I; return *ret;
	}
};

class pyInteractionContainer{
	public:
		const shared_ptr<InteractionContainer> proxee;
		pyInteractionContainer(const shared_ptr<InteractionContainer>& _proxee): proxee(_proxee){}
		pyInteractionIterator pyIter(){return pyInteractionIterator(proxee);}
		shared_ptr<Interaction> pyGetitem(vector<Body::id_t> id12){
			//if(!PySequence_Check(id12.ptr())) throw invalid_argument("Key must be a tuple");
			//if(pybind11::len(id12)!=2) throw invalid_argument("Key must be a 2-tuple: id1,id2.");
			if(id12.size()==2){
				//if(max(id12[0],id12[1])>
				shared_ptr<Interaction> i=proxee->find(id12[0],id12[1]);
				if(i) return i; else { PyErr_SetString(PyExc_IndexError,"No such interaction"); throw pybind11::error_already_set();; /* make compiler happy; never reached */ return shared_ptr<Interaction>(); }
			}
			else if(id12.size()==1){ return (*proxee)[id12[0]];}
			else throw invalid_argument("2 integers (id1,id2) or 1 integer (nth) required.");
		}
		/* return nth _real_ iteration from the container (0-based index); this is to facilitate picking random interaction */
		shared_ptr<Interaction> pyNth(long n){
			long i=0; for (shared_ptr<Interaction> I : *proxee){ if(!I->isReal()) continue; if(i++==n) return I; }
			PyErr_SetString(PyExc_IndexError,(string("Interaction number out of range (")+std::to_string(n)+">="+std::to_string(i)+").").c_str());
			throw pybind11::error_already_set();; /* make compiler happy; never reached */ return shared_ptr<Interaction>();
		}
		long len(){return proxee->size();}
		void clear(){proxee->clear();}
		pybind11::list withBody(long id){ pybind11::list ret; for (const shared_ptr<Interaction>& I : *proxee){ if(I->isReal() && (I->getId1()==id || I->getId2()==id)) ret.append(I);} return ret;}
		pybind11::list withBodyAll(long id){ pybind11::list ret; for (const shared_ptr<Interaction>& I : *proxee){ if(I->getId1()==id || I->getId2()==id) ret.append(I);} return ret; }
		long countReal(){ long ret=0; for (const shared_ptr<Interaction>& I : *proxee){ if(I->isReal()) ret++; } return ret; }
		bool serializeSorted_get(){return proxee->serializeSorted;}
		void serializeSorted_set(bool ss){proxee->serializeSorted=ss;}
		void eraseNonReal(){ proxee->eraseNonReal(); }
		void erase(Body::id_t id1, Body::id_t id2){ proxee->requestErase(id1,id2); }
};

class pyForceContainer{
		shared_ptr<Scene> scene;
	public:
		pyForceContainer(shared_ptr<Scene> _scene): scene(_scene) { }
		void checkId(long id){ if(id<0 || (size_t)id>=scene->bodies->size()){ PyErr_SetString(PyExc_IndexError, "Body id out of range."); throw pybind11::error_already_set();; /* never reached */ throw; } }
		Vector2r force_get(long id, bool sync){  checkId(id); if (!sync) return scene->forces.getForceSingle(id); scene->forces.sync(); return scene->forces.getForce(id);}
		Real torque_get(long id, bool sync){ checkId(id); if (!sync) return scene->forces.getTorqueSingle(id); scene->forces.sync(); return scene->forces.getTorque(id);}
		Vector2r move_get(long id){ checkId(id); return scene->forces.getMoveSingle(id); }
		Real rot_get(long id){ checkId(id); return scene->forces.getRotSingle(id); }
		void force_add(long id, const Vector2r& f, bool permanent){  checkId(id); if (!permanent) scene->forces.addForce (id,f); else scene->forces.addPermForce (id,f); }
		void torque_add(long id, const Real& t, bool permanent){ checkId(id); if (!permanent) scene->forces.addTorque(id,t); else scene->forces.addPermTorque(id,t);}
		void move_add(long id, const Vector2r& t){   checkId(id); scene->forces.addMove(id,t);}
		void rot_add(long id, const Real& t){    checkId(id); scene->forces.addRot(id,t);}
		Vector2r permForce_get(long id){  checkId(id); return scene->forces.getPermForce(id);}
		Real permTorque_get(long id){  checkId(id); return scene->forces.getPermTorque(id);}
		void reset(bool resetAll) {scene->forces.reset(scene->iter,resetAll);}
		long syncCount_get(){ return scene->forces.syncCount;}
		void syncCount_set(long count){ scene->forces.syncCount=count;}
		bool getPermForceUsed() {return scene->forces.getPermForceUsed();}
};


class pyMaterialContainer{
		shared_ptr<Scene> scene;
	public:
		pyMaterialContainer(shared_ptr<Scene> _scene): scene(_scene) { }
		shared_ptr<Material> getitem_id(int _id){ int id=(_id>=0 ? _id : scene->materials.size()+_id); if(id<0 || (size_t)id>=scene->materials.size()){ PyErr_SetString(PyExc_IndexError, "Material id out of range."); throw pybind11::error_already_set();; /* never reached */ throw; } return Material::byId(id,scene); }
		shared_ptr<Material> getitem_label(string label){
			// translate runtime_error to KeyError (instead of RuntimeError) if the material doesn't exist
			try { return Material::byLabel(label,scene);	}
			catch (std::runtime_error& e){ PyErr_SetString(PyExc_KeyError,e.what()); throw pybind11::error_already_set();; /* never reached; avoids warning */ throw; }
		}
		int append(shared_ptr<Material> m){ scene->materials.push_back(m); m->id=scene->materials.size()-1; return m->id; }
		vector<int> appendList(vector<shared_ptr<Material> > mm){ vector<int> ret; for (shared_ptr<Material>& m : mm) ret.push_back(append(m)); return ret; }
		int len(){ return (int)scene->materials.size(); }
		int index(const std::string& label){ return Material::byLabelIndex(label,scene.get()); }
};

void termHandlerNormal(int sig){cerr<<"SudoDEM: normal exit."<<endl; raise(SIGTERM);}
void termHandlerError(int sig){cerr<<"SudoDEM: error exit."<<endl; raise(SIGTERM);}

class pyOmega{
	private:
		// can be safely removed now, since pyOmega makes an empty scene in the constructor, if there is none
		void assertScene(){if(!OMEGA.getScene()) throw std::runtime_error("No Scene instance?!"); }
		Omega& OMEGA;
	public:
	pyOmega(): OMEGA(Omega::instance()){
		shared_ptr<Scene> rb=OMEGA.getScene();
		if(!rb){
			OMEGA.init();
			rb=OMEGA.getScene();
		}
		assert(rb);
		if(!OMEGA.hasSimulationLoop()){
			OMEGA.createSimulationLoop();
		}
	};
	/* Create variables in python's __builtin__ namespace that correspond to labeled objects. At this moment, only engines and functors can be labeled (not bodies etc). */
	void mapLabeledEntitiesToVariables(){
		// not sure if we should map materials to variables by default...
		// a call to this functions would have to be added to pyMaterialContainer::append
		#if 0
			for (const shared_ptr<Material>& m : OMEGA.getScene()->materials){
				if(!m->label.empty()) { PyGILState_STATE gstate; gstate = PyGILState_Ensure(); PyRun_SimpleString(("__builtins__."+m->label+"=Omega().materials["+std::to_string(m->id)+"]").c_str()); PyGILState_Release(gstate); }
			}
		#endif
		for (const shared_ptr<Engine>& e : OMEGA.getScene()->engines){
			if(!e->label.empty()){
				pyRunString("__builtins__."+e->label+"=Omega().labeledEngine('"+e->label+"')");
			}
			#define _DO_FUNCTORS(functors,FunctorT){ for (const shared_ptr<FunctorT>& f : functors){ if(!f->label.empty()){ pyRunString("__builtins__."+f->label+"=Omega().labeledEngine('"+f->label+"')");}} }
			#define _TRY_DISPATCHER(DispatcherT) { DispatcherT* d=dynamic_cast<DispatcherT*>(e.get()); if(d){ _DO_FUNCTORS(d->functors,DispatcherT::FunctorType); } }
			_TRY_DISPATCHER(BoundDispatcher); _TRY_DISPATCHER(IGeomDispatcher); _TRY_DISPATCHER(IPhysDispatcher); _TRY_DISPATCHER(LawDispatcher);
			InteractionLoop* id=dynamic_cast<InteractionLoop*>(e.get());
			if(id){
				_DO_FUNCTORS(id->geomDispatcher->functors,IGeomFunctor);
				_DO_FUNCTORS(id->physDispatcher->functors,IPhysFunctor);
				_DO_FUNCTORS(id->lawDispatcher->functors,LawFunctor);
			}
			Collider* coll=dynamic_cast<Collider*>(e.get());
			if(coll){ _DO_FUNCTORS(coll->boundDispatcher->functors,BoundFunctor); }
			#undef _DO_FUNCTORS
			#undef _TRY_DISPATCHER
		}
	}
	pybind11::object labeled_engine_get(string label){
		for (const shared_ptr<Engine>& e : OMEGA.getScene()->engines){
			#define _DO_FUNCTORS(functors,FunctorT){ for (const shared_ptr<FunctorT>& f : functors){ if(f->label==label) return pybind11::cast(f); }}
			#define _TRY_DISPATCHER(DispatcherT) { DispatcherT* d=dynamic_cast<DispatcherT*>(e.get()); if(d){ _DO_FUNCTORS(d->functors,DispatcherT::FunctorType); } }
			if(e->label==label){ return pybind11::cast(e); }
			_TRY_DISPATCHER(BoundDispatcher); _TRY_DISPATCHER(IGeomDispatcher); _TRY_DISPATCHER(IPhysDispatcher); _TRY_DISPATCHER(LawDispatcher);
			InteractionLoop* id=dynamic_cast<InteractionLoop*>(e.get());
			if(id){
				_DO_FUNCTORS(id->geomDispatcher->functors,IGeomFunctor);
				_DO_FUNCTORS(id->physDispatcher->functors,IPhysFunctor);
				_DO_FUNCTORS(id->lawDispatcher->functors,LawFunctor);
			}
			Collider* coll=dynamic_cast<Collider*>(e.get());
			if(coll){ _DO_FUNCTORS(coll->boundDispatcher->functors,BoundFunctor); }
			#undef _DO_FUNCTORS
			#undef _TRY_DISPATCHER
		}
		throw std::invalid_argument(string("No engine labeled `")+label+"'");
	}

	long iter(){ return OMEGA.getScene()->iter;}
	int subStep(){ return OMEGA.getScene()->subStep; }
	bool subStepping_get(){ return OMEGA.getScene()->subStepping; }
	void subStepping_set(bool val){ OMEGA.getScene()->subStepping=val; }

	double time(){return OMEGA.getScene()->time;}
	double realTime(){ return OMEGA.getRealTime(); }
	double speed(){ return OMEGA.getScene()->speed; }
	double dt_get(){return OMEGA.getScene()->dt;}
	void dt_set(double dt){
		Scene* scene=OMEGA.getScene().get();
		// activate timestepper, if possible (throw exception if there is none)
		if(dt<0){ if(!scene->timeStepperActivate(true)) /* not activated*/ throw runtime_error("No TimeStepper found in O.engines."); }
		else { scene->dt=dt; }
	}
	bool dynDt_get(){return OMEGA.getScene()->timeStepperActive();}
	bool dynDt_set(bool activate){if(!OMEGA.getScene()->timeStepperActivate(activate)) /* not activated*/ throw runtime_error("No TimeStepper found in O.engines."); return true;}
	bool dynDtAvailable_get(){ return OMEGA.getScene()->timeStepperPresent(); }
	long stopAtIter_get(){return OMEGA.getScene()->stopAtIter; }
	void stopAtIter_set(long s){OMEGA.getScene()->stopAtIter=s; }
	Real stopAtTime_get(){return OMEGA.getScene()->stopAtTime; }
	void stopAtTime_set(long s){OMEGA.getScene()->stopAtTime=s; }


	bool timingEnabled_get(){return TimingInfo::enabled;}
	void timingEnabled_set(bool enabled){TimingInfo::enabled=enabled;}
	// deprecated:
		unsigned long forceSyncCount_get(){ return OMEGA.getScene()->forces.syncCount;}
		void forceSyncCount_set(unsigned long count){ OMEGA.getScene()->forces.syncCount=count;}

	void run(long int numIter=-1,bool doWait=false){
		Scene* scene=OMEGA.getScene().get();
		if(numIter>0) scene->stopAtIter=scene->iter+numIter;
		OMEGA.run();
		// timespec t1,t2; t1.tv_sec=0; t1.tv_nsec=40000000; /* 40 ms */
		// while(!OMEGA.isRunning()) nanosleep(&t1,&t2); // wait till we start, so that calling wait() immediately afterwards doesn't return immediately
		LOG_DEBUG("RUN"<<((scene->stopAtIter-scene->iter)>0?string(" ("+std::to_string(scene->stopAtIter-scene->iter)+" to go)"):string(""))<<"!");
		if(doWait) wait();
	}
	void pause(){Py_BEGIN_ALLOW_THREADS; OMEGA.pause(); Py_END_ALLOW_THREADS; LOG_DEBUG("PAUSE!");}
	void step() { if(OMEGA.isRunning()) throw runtime_error("Called O.step() while simulation is running."); OMEGA.getScene()->moveToNextTimeStep(); /* LOG_DEBUG("STEP!"); run(1); wait(); */ }
	void wait(){
		if(OMEGA.isRunning()){LOG_DEBUG("WAIT!");} else return;
		timespec t1,t2; t1.tv_sec=0; t1.tv_nsec=40000000; /* 40 ms */ Py_BEGIN_ALLOW_THREADS; 
		while(OMEGA.isRunning())
		{
			#ifdef _WIN32
				std::this_thread::sleep_for(std::chrono::milliseconds(40));
			#else
				nanosleep(&t1,&t2); 
			#endif
		} 
		
		Py_END_ALLOW_THREADS;
		if(!OMEGA.simulationLoop->workerThrew) return;
		LOG_ERROR("Simulation error encountered."); OMEGA.simulationLoop->workerThrew=false; throw OMEGA.simulationLoop->workerException;
	}
	bool isRunning(){ return OMEGA.isRunning(); }
	pybind11::object get_filename(){ string f=OMEGA.sceneFile; if(f.size()>0) return pybind11::cast(f); return pybind11::object();}
	void load(std::string fileName,bool quiet=false) {
		Py_BEGIN_ALLOW_THREADS; OMEGA.stop(); Py_END_ALLOW_THREADS;
		OMEGA.loadSimulation(fileName,quiet);
		OMEGA.createSimulationLoop();
		mapLabeledEntitiesToVariables();
	}
	void reload(bool quiet=false){	load(OMEGA.sceneFile,quiet);}
	void saveTmp(string mark="", bool quiet=false){ save(":memory:"+mark,quiet);}
	void loadTmp(string mark="", bool quiet=false){ load(":memory:"+mark,quiet);}
	pybind11::list lsTmp(){ pybind11::list ret; typedef pair<std::string,string> strstr; for (const auto& sim : OMEGA.memSavedSimulations){ string mark = sim.first; std::size_t pos = mark.find(":memory:"); if (pos != std::string::npos) mark.replace(pos, 8, "");  ret.append(mark); } return ret; }
	void tmpToFile(string mark, string filename){
		if(OMEGA.memSavedSimulations.count(":memory:"+mark)==0) throw runtime_error("No memory-saved simulation named "+mark);
		LOG_INFO("Saving :memory:"<<mark<<" to "<<filename);
		sudodem::CompressionUtils::writeToFile(filename, OMEGA.memSavedSimulations[":memory:"+mark]);
	}
	string tmpToString(string mark){
		if(OMEGA.memSavedSimulations.count(":memory:"+mark)==0) throw runtime_error("No memory-saved simulation named "+mark);
		return OMEGA.memSavedSimulations[":memory:"+mark];
	}

	void reset(){OMEGA.stop(); OMEGA.reset(); }
	void resetThisScene(){Py_BEGIN_ALLOW_THREADS; OMEGA.stop(); Py_END_ALLOW_THREADS; OMEGA.resetCurrentScene(); OMEGA.createSimulationLoop();}
	void resetCurrentScene(){Py_BEGIN_ALLOW_THREADS; OMEGA.stop(); Py_END_ALLOW_THREADS; OMEGA.resetCurrentScene(); OMEGA.createSimulationLoop();}
	void resetTime(){ OMEGA.getScene()->iter=0; OMEGA.getScene()->time=0; OMEGA.timeInit(); }
	void switchScene(){ std::swap(OMEGA.scenes[OMEGA.currentSceneNb],OMEGA.sceneAnother); }
	void resetAllScenes(){Py_BEGIN_ALLOW_THREADS; OMEGA.stop(); Py_END_ALLOW_THREADS; OMEGA.resetAllScenes(); OMEGA.createSimulationLoop();}
	shared_ptr<Scene> scene_get(){ return OMEGA.getScene(); }
	int addScene(){return OMEGA.addScene();}
	void switchToScene(int i){OMEGA.switchToScene(i);}
	string sceneToString(){
		ostringstream oss;
		sudodem::ObjectIO::save<decltype(OMEGA.getScene()),cereal::BinaryOutputArchive>(oss,"scene",OMEGA.getScene());
		oss.flush();
		return oss.str();
	}
	void stringToScene(const string &sstring, string mark=""){
		Py_BEGIN_ALLOW_THREADS; OMEGA.stop(); Py_END_ALLOW_THREADS;
		assertScene();
		OMEGA.memSavedSimulations[":memory:"+mark]=sstring;
		OMEGA.sceneFile=":memory:"+mark;
		load(OMEGA.sceneFile,true);
	}

	void save(std::string fileName,bool quiet=false){
		assertScene();
		OMEGA.saveSimulation(fileName,quiet);
		// OMEGA.sceneFile=fileName; // done in Omega::saveSimulation;
	}

	pybind11::list miscParams_get(){
		pybind11::list ret;
		for (const shared_ptr<Serializable>& s : OMEGA.getScene()->miscParams){
			ret.append(s);
		}
		return ret;
	}

	void miscParams_set(vector<shared_ptr<Serializable> > ss){
		vector<shared_ptr<Serializable> >& miscParams=OMEGA.getScene()->miscParams;
		miscParams.clear();
		for (shared_ptr<Serializable> s : ss){
			miscParams.push_back(s);
		}
	}


	vector<shared_ptr<Engine> > engines_get(void){assertScene(); Scene* scene=OMEGA.getScene().get(); return scene->_nextEngines.empty()?scene->engines:scene->_nextEngines;}
	void engines_set(const vector<shared_ptr<Engine> >& egs){
		assertScene(); Scene* scene=OMEGA.getScene().get();
		if(scene->subStep<0) scene->engines=egs; // not inside the engine loop right now, ok to update directly
		else scene->_nextEngines=egs; // inside the engine loop, update _nextEngines; O.engines picks that up automatically, and Scene::moveToNextTimestep will put them in place of engines at the start of the next loop
		mapLabeledEntitiesToVariables();
	}
	// raw access to engines/_nextEngines, for debugging
	vector<shared_ptr<Engine> > currEngines_get(){ return OMEGA.getScene()->engines; }
	vector<shared_ptr<Engine> > nextEngines_get(){ return OMEGA.getScene()->_nextEngines; }

	pyBodyContainer bodies_get(void){assertScene(); return pyBodyContainer(OMEGA.getScene()->bodies); }

	pyInteractionContainer interactions_get(void){assertScene(); return pyInteractionContainer(OMEGA.getScene()->interactions); }

	pyForceContainer forces_get(void){return pyForceContainer(OMEGA.getScene());}

	pyMaterialContainer materials_get(void){return pyMaterialContainer(OMEGA.getScene());}


	pybind11::list listChildClassesNonrecursive(const string& base){
		pybind11::list ret;
		for(map<string,DynlibDescriptor>::const_iterator di=Omega::instance().getDynlibsDescriptor().begin();di!=Omega::instance().getDynlibsDescriptor().end();++di) if (Omega::instance().isInheritingFrom((*di).first,base)) ret.append(di->first);
		return ret;
	}

	bool isChildClassOf(const string& child, const string& base){
		return (Omega::instance().isInheritingFrom_recursive(child,base));
	}

	pybind11::list plugins_get(){
		const map<string,DynlibDescriptor>& plugins=Omega::instance().getDynlibsDescriptor();
		std::pair<string,DynlibDescriptor> p; pybind11::list ret;
		for (const auto& p : plugins) ret.append(p.first);
		return ret;
	}

	pyTags tags_get(void){assertScene(); return pyTags(OMEGA.getScene());}

	void interactionContainer_set(string clss){
		Scene* rb=OMEGA.getScene().get();
		if(rb->interactions->size()>0) throw std::runtime_error("Interaction container not empty, will not change its class.");
		shared_ptr<InteractionContainer> ic=SUDODEM_PTR_DYN_CAST<InteractionContainer>(ClassRegistry::instance().create(clss));
		rb->interactions=ic;
	}
	string interactionContainer_get(string clss){ return OMEGA.getScene()->interactions->getClassName(); }

	void bodyContainer_set(string clss){
		Scene* rb=OMEGA.getScene().get();
		if(rb->bodies->size()>0) throw std::runtime_error("Body container not empty, will not change its class.");
		shared_ptr<BodyContainer> bc=SUDODEM_PTR_DYN_CAST<BodyContainer>(ClassRegistry::instance().create(clss));
		rb->bodies=bc;
	}
	string bodyContainer_get(string clss){ return OMEGA.getScene()->bodies->getClassName(); }
	#ifdef SUDODEM_OPENMP
		int numThreads_get(){ return omp_get_max_threads();}
		void numThreads_set(int n){ int bcn=OMEGA.getScene()->forces.getNumAllocatedThreads(); if(bcn<n) LOG_WARN("ForceContainer has only "<<bcn<<" threads allocated. Changing thread number to on "<<bcn<<" instead of "<<n<<" requested."); omp_set_num_threads(min(n,bcn)); LOG_WARN("BUG: Omega().numThreads=n doesn't work as expected (number of threads is not changed globally). Set env var OMP_NUM_THREADS instead."); }
	#else
		int numThreads_get(){return 1;}
		void numThreads_set(int n){ LOG_WARN("SudoDEM was compiled without openMP support, changing number of threads will have no effect."); }
	#endif

	shared_ptr<Cell> cell_get(){ if(OMEGA.getScene()->isPeriodic) return OMEGA.getScene()->cell; return shared_ptr<Cell>(); }
	bool periodic_get(void){ return OMEGA.getScene()->isPeriodic; }
	void periodic_set(bool v){ OMEGA.getScene()->isPeriodic=v; }

	shared_ptr<EnergyTracker> energy_get(){ return OMEGA.getScene()->energy; }
	bool trackEnergy_get(void){ return OMEGA.getScene()->trackEnergy; }
	void trackEnergy_set(bool e){ OMEGA.getScene()->trackEnergy=e; }

	void disableGdb(){
		signal(SIGSEGV,SIG_DFL);
		signal(SIGABRT,SIG_DFL);
	}
	void exitNoBacktrace(int status=0){
		if(status==0) signal(SIGSEGV,termHandlerNormal); /* unset the handler that runs gdb and prints backtrace */
		else signal(SIGSEGV,termHandlerError);
		// try to clean our mess
		Omega::instance().cleanupTemps();
		// flush all streams (so that in case we crash at exit, unflushed buffers are not lost)
		fflush(NULL);
		// attempt exit
		exit(status);
	}
	void runEngine(const shared_ptr<Engine>& e){ LOG_WARN("Omega().runEngine(): deprecated, use __call__ method of the engine instance directly instead; will be removed in the future."); e->scene=OMEGA.getScene().get(); e->action(); }
	std::string tmpFilename(){ return OMEGA.tmpFilename(); }
};

PYBIND11_MODULE(wrapper, m)
{
	// std::cerr << "DEBUG: PYBIND11_MODULE starting" << std::endl;
	m.doc() = "Wrapper for c++ internals of sudodem.";

	SUDODEM_SET_DOCSTRING_OPTS;
	// std::cerr << "DEBUG: SUDODEM_SET_DOCSTRING_OPTS done" << std::endl;

	pybind11::enum_<sudodem::Attr::flags>(m, "AttrFlags")
		.value("noSave",sudodem::Attr::noSave)
		.value("readonly",sudodem::Attr::readonly)
		.value("triggerPostLoad",sudodem::Attr::triggerPostLoad)
		.value("noResize",sudodem::Attr::noResize)
    ;

	pybind11::class_<pyOmega>(m, "Omega")
		.def(pybind11::init<>())
		.def_property_readonly("iter",&pyOmega::iter,"Get current step number")
		.def_property_readonly("subStep",&pyOmega::subStep,"Get the current subStep number (only meaningful if O.subStepping==True); -1 when outside the loop, otherwise either 0 (O.subStepping==False) or number of engine to be run (O.subStepping==True)")
		.def_property("subStepping",&pyOmega::subStepping_get,&pyOmega::subStepping_set,"Get/set whether subStepping is active.")
		.def_property("stopAtIter",&pyOmega::stopAtIter_get,&pyOmega::stopAtIter_set,"Get/set number of iteration after which the simulation will stop.")
		.def_property("stopAtTime",&pyOmega::stopAtTime_get,&pyOmega::stopAtTime_set,"Get/set time after which the simulation will stop.")
		.def_property_readonly("time",&pyOmega::time,"Return virtual (model world) time of the simulation.")
		.def_property_readonly("realtime",&pyOmega::realTime,"Return clock (human world) time the simulation has been running.")
		.def_property_readonly("speed",&pyOmega::speed,"Return current calculation speed [iter/sec].")
		.def_property("dt",&pyOmega::dt_get,&pyOmega::dt_set,"Current timestep (Δt) value.")
		.def_property("dynDt",&pyOmega::dynDt_get,&pyOmega::dynDt_set,"Whether a :yref:`TimeStepper` is used for dynamic Δt control. See :yref:`dt<Omega.dt>` on how to enable/disable :yref:`TimeStepper`.")
		.def_property_readonly("dynDtAvailable",&pyOmega::dynDtAvailable_get,"Whether a :yref:`TimeStepper` is amongst :yref:`O.engines<Omega.engines>`, activated or not.")
		.def("load",&pyOmega::load,pybind11::arg("file"),pybind11::arg("quiet")=false,"Load simulation from file. The file should be :yref:`saved<Omega.save>` in the same version of SudoDEM, otherwise compatibility is not guaranteed.")
		.def("reload",&pyOmega::reload,pybind11::arg("quiet")=false,"Reload current simulation")
		.def("save",&pyOmega::save,pybind11::arg("file"),pybind11::arg("quiet")=false,"Save current simulation to file (should be .xml or .xml.bz2). The file should be :yref:`loaded<Omega.load>` in the same version of SudoDEM, otherwise compatibility is not guaranteed.")
		.def("loadTmp",&pyOmega::loadTmp,pybind11::arg("mark")="",pybind11::arg("quiet")=false,"Load simulation previously stored in memory by saveTmp. *mark* optionally distinguishes multiple saved simulations")
		.def("saveTmp",&pyOmega::saveTmp,pybind11::arg("mark")="",pybind11::arg("quiet")=false,"Save simulation to memory (disappears at shutdown), can be loaded later with loadTmp. *mark* optionally distinguishes different memory-saved simulations.")
		.def("lsTmp",&pyOmega::lsTmp,"Return list of all memory-saved simulations.")
		.def("tmpToFile",&pyOmega::tmpToFile,pybind11::arg("mark"),pybind11::arg("fileName"),"Save XML of :yref:`saveTmp<Omega.saveTmp>`'d simulation into *fileName*.")
		.def("tmpToString",&pyOmega::tmpToString,pybind11::arg("mark")="","Return XML of :yref:`saveTmp<Omega.saveTmp>`'d simulation as string.")
		.def("run",&pyOmega::run,pybind11::arg("nSteps")=-1,pybind11::arg("wait")=false,"Run the simulation. *nSteps* how many steps to run, then stop (if positive); *wait* will cause not returning to python until simulation will have stopped.")
		.def("pause",&pyOmega::pause,"Stop simulation execution. (May be called from within the loop, and it will stop after the current step).")
		.def("step",&pyOmega::step,"Advance the simulation by one step. Returns after the step will have finished.")
		.def("wait",&pyOmega::wait,"Don't return until the simulation will have been paused. (Returns immediately if not running).")
		.def_property_readonly("running",&pyOmega::isRunning,"Whether background thread is currently running a simulation.")
		.def_property_readonly("filename",&pyOmega::get_filename,"Filename under which the current simulation was saved (None if never saved).")
		.def("reset",&pyOmega::reset,"Reset simulations completely (including another scenes!).")
		.def("resetThisScene",&pyOmega::resetThisScene,"Reset current scene.")
		.def("resetCurrentScene",&pyOmega::resetCurrentScene,"Reset current scene.")
		.def("resetAllScenes",&pyOmega::resetAllScenes,"Reset all scenes.")
		.def("addScene",&pyOmega::addScene,"Add new scene to Omega, returns its number")
		.def("switchToScene",&pyOmega::switchToScene,"Switch to defined scene. Default scene has number 0, other scenes have to be created by addScene method.")
		.def("switchScene",&pyOmega::switchScene,"Switch to alternative simulation (while keeping the old one). Calling the function again switches back to the first one. Note that most variables from the first simulation will still refer to the first simulation even after the switch\n(e.g. b=O.bodies[4]; O.switchScene(); [b still refers to the body in the first simulation here])")
		.def("sceneToString",&pyOmega::sceneToString,"Return the entire scene as a string. Equivalent to using O.save(...) except that the scene goes to a string instead of a file. (see also stringToScene())")
		.def("stringToScene",&pyOmega::stringToScene,pybind11::arg("sstring"),pybind11::arg("mark")="","Load simulation from a string passed as argument (see also sceneToString).")
		.def("labeledEngine",&pyOmega::labeled_engine_get,"Return instance of engine/functor with the given label. This function shouldn't be called by the user directly; every ehange in O.engines will assign respective global python variables according to labels.\n\nFor example:\n\n\t *O.engines=[InsertionSortCollider(label='collider')]*\n\n\t *collider.nBins=5 # collider has become a variable after assignment to O.engines automatically*")
		.def("resetTime",&pyOmega::resetTime,"Reset simulation time: step number, virtual and real time. (Doesn't touch anything else, including timings).")
		.def("plugins",&pyOmega::plugins_get,"Return list of all plugins registered in the class factory.")
		.def("_sceneObj",&pyOmega::scene_get,"Return the :yref:`scene <Scene>` object. Debugging only, all (or most) :yref:`Scene` functionality is proxies through :yref:`Omega`.")
		.def_property("engines",&pyOmega::engines_get,&pyOmega::engines_set,"List of engines in the simulation (Scene::engines).")
		.def_property_readonly("_currEngines",&pyOmega::currEngines_get,"Currently running engines; debugging only!")
		.def_property_readonly("_nextEngines",&pyOmega::nextEngines_get,"Engines for the next step, if different from the current ones, otherwise empty; debugging only!")
		.def_property("miscParams",&pyOmega::miscParams_get,&pyOmega::miscParams_set,"MiscParams in the simulation (Scene::mistParams), usually used to save serializables that don't fit anywhere else, like GL functors")
		.def_property_readonly("bodies",&pyOmega::bodies_get,"Bodies in the current simulation (container supporting index access by id and iteration)")
		.def_property_readonly("interactions",&pyOmega::interactions_get,"Interactions in the current simulation (container supporting index acces by either (id1,id2) or interactionNumber and iteration)")
		.def_property_readonly("materials",&pyOmega::materials_get,"Shared materials; they can be accessed by id or by label")
		.def_property_readonly("forces",&pyOmega::forces_get,":yref:`ForceContainer` (forces, torques, displacements) in the current simulation.")
		.def_property_readonly("energy",&pyOmega::energy_get,":yref:`EnergyTracker` of the current simulation. (meaningful only with :yref:`O.trackEnergy<Omega.trackEnergy>`)")
		.def_property("trackEnergy",&pyOmega::trackEnergy_get,&pyOmega::trackEnergy_set,"When energy tracking is enabled or disabled in this simulation.")
		.def_property_readonly("tags",&pyOmega::tags_get,"Tags (string=string dictionary) of the current simulation (container supporting string-index access/assignment)")
		.def("childClassesNonrecursive",&pyOmega::listChildClassesNonrecursive,"Return list of all classes deriving from given class, as registered in the class factory")
		.def("isChildClassOf",&pyOmega::isChildClassOf,"Tells whether the first class derives from the second one (both given as strings).")
		.def_property("timingEnabled",&pyOmega::timingEnabled_get,&pyOmega::timingEnabled_set,"Globally enable/disable timing services (see documentation of the :yref:`timing module<sudodem.timing>`).")
		.def_property("forceSyncCount",&pyOmega::forceSyncCount_get,&pyOmega::forceSyncCount_set,"Counter for number of syncs in ForceContainer, for profiling purposes.")
		.def_property_readonly("numThreads",&pyOmega::numThreads_get /* ,&pyOmega::numThreads_set*/ ,"Get maximum number of threads openMP can use.")
		.def_property_readonly("cell",&pyOmega::cell_get,"Periodic cell of the current scene (None if the scene is aperiodic).")
		.def_property("periodic",&pyOmega::periodic_get,&pyOmega::periodic_set,"Get/set whether the scene is periodic or not (True/False).")
		.def("exitNoBacktrace",&pyOmega::exitNoBacktrace,pybind11::arg("status")=0,"Disable SEGV handler and exit, optionally with given status number.")
		.def("disableGdb",&pyOmega::disableGdb,"Revert SEGV and ABRT handlers to system defaults.")
		.def("runEngine",&pyOmega::runEngine,"Run given engine exactly once; simulation time, step number etc. will not be incremented (use only if you know what you do).")
		.def("tmpFilename",&pyOmega::tmpFilename,"Return unique name of file in temporary directory which will be deleted when sudodem exits.")
		;
	// std::cerr << "DEBUG: pyOmega registration done" << std::endl;
	pybind11::class_<pyTags>(m, "TagsWrapper","Container emulating dictionary semantics for accessing tags associated with simulation. Tags are accesed by strings.")
		.def("__getitem__",&pyTags::getItem)
		.def("__setitem__",&pyTags::setItem)
		.def("keys",&pyTags::keys)
		.def("has_key",&pyTags::hasKey);
	// std::cerr << "DEBUG: pyTags registration done" << std::endl;
	pybind11::class_<pyBodyContainer>(m, "BodyContainer")
		.def("__getitem__",&pyBodyContainer::pyGetitem)
		.def("__len__",&pyBodyContainer::length)
		.def("__iter__",&pyBodyContainer::pyIter)
		.def("append",&pyBodyContainer::append,"Append one Body instance, return its id.")
		.def("append",&pyBodyContainer::appendList,"Append list of Body instance, return list of ids")
		.def("appendClumped",&pyBodyContainer::appendClump,pybind11::arg("bb"),pybind11::arg("discretization")=0,"Append given list of bodies as a clump (rigid aggregate); returns a tuple of ``(clumpId,[memberId1,memberId2,...])``. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).")
		.def("clump",&pyBodyContainer::clump,pybind11::arg("ids"),pybind11::arg("discretization")=0,"Clump given bodies together (creating a rigid aggregate); returns ``clumpId``. Clump masses and inertia are adapted automatically when discretization>0. If clump members are overlapping this is done by integration/summation over mass points using a regular grid of cells (number of grid cells in one direction is defined as $R_{min}/discretization$, where $R_{min}$ is minimum clump member radius). For non-overlapping members inertia of the clump is the sum of inertias from members. If discretization<=0 sum of inertias from members is used (faster, but inaccurate).")
		.def("updateClumpProperties",&pyBodyContainer::updateClumpProperties,pybind11::arg("excludeList")=pybind11::list(),pybind11::arg("discretization")=5,"Update clump properties mass, volume and inertia (for details of 'discretization' value see :yref:`clump()<BodyContainer.clump>`). Clumps can be excluded from the calculation by giving a list of ids: *O.bodies.updateProperties([ids])*.")
		.def("addToClump",&pyBodyContainer::addToClump,pybind11::arg("bids"),pybind11::arg("cid"),pybind11::arg("discretization")=0,"Add body b (or a list of bodies) to an existing clump c. c must be clump and b may not be a clump member of c. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).\n\nSee :ysrc:`examples/clumps/addToClump-example.py` for an example script.\n\n.. note:: If b is a clump itself, then all members will be added to c and b will be deleted. If b is a clump member of clump d, then all members from d will be added to c and d will be deleted. If you need to add just clump member b, :yref:`release<BodyContainer.releaseFromClump>` this member from d first.")
		.def("releaseFromClump",&pyBodyContainer::releaseFromClump,pybind11::arg("bid"),pybind11::arg("cid"),pybind11::arg("discretization")=0,"Release body b from clump c. b must be a clump member of c. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).\n\nSee :ysrc:`examples/clumps/releaseFromClump-example.py` for an example script.\n\n.. note:: If c contains only 2 members b will not be released and a warning will appear. In this case clump c should be :yref:`erased<BodyContainer.erase>`.")
		.def("replaceByClumps",&pyBodyContainer::replaceByClumps,pybind11::arg("ctList"),pybind11::arg("amounts"),pybind11::arg("discretization")=0,"Replace disks by clumps using a list of clump templates and a list of amounts; returns a list of tuples: ``[(clumpId1,[memberId1,memberId2,...]),(clumpId2,[memberId1,memberId2,...]),...]``. A new clump will have the same volume as the disk, that was replaced. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`). \n\n\t *O.bodies.replaceByClumps( [utils.clumpTemplate([1,1],[.5,.5])] , [.9] ) #will replace 90 % of all standalone disks by 'dyads'*\n\nSee :ysrc:`examples/clumps/replaceByClumps-example.py` for an example script.")
		.def("getRoundness",&pyBodyContainer::getRoundness,pybind11::arg("excludeList")=pybind11::list(),"Returns roundness coefficient RC = R2/R1. R1 is the theoretical radius of a disk, with same volume as clump. R2 is the minimum radius of a disk, that imbeds clump. If just disks are present RC = 1. If clumps are present 0 < RC < 1. Bodies can be excluded from the calculation by giving a list of ids: *O.bodies.getRoundness([ids])*.\n\nSee :ysrc:`examples/clumps/replaceByClumps-example.py` for an example script.")
		.def("clear", &pyBodyContainer::clear,"Remove all bodies (interactions not checked)")
		.def("erase", &pyBodyContainer::erase,pybind11::arg("id"),pybind11::arg("eraseClumpMembers")=0,"Erase body with the given id; all interaction will be deleted by InteractionLoop in the next step. If a clump is erased use *O.bodies.erase(clumpId,True)* to erase the clump AND its members.")
		.def("replace",&pyBodyContainer::replace);
	// std::cerr << "DEBUG: pyBodyContainer registration done" << std::endl;
	pybind11::class_<pyBodyIterator>(m, "BodyIterator").def(pybind11::init<const pyBodyIterator&>())
		.def("__iter__",&pyBodyIterator::pyIter)
		.def("__next__",&pyBodyIterator::pyNext);

	pybind11::class_<pyInteractionContainer>(m, "InteractionContainer", "Access to :yref:`interactions<Interaction>` of simulation, by using \n\n#. id's of both :yref:`Bodies<Body>` of the interactions, e.g. ``O.interactions[23,65]``\n#. iteraction over the whole container::\n\n\tfor i in O.interactions: print i.id1,i.id2\n\n.. note::\n\tIteration silently skips interactions that are not :yref:`real<Interaction.isReal>`.").def(pybind11::init<const pyInteractionContainer&>())
		.def("__iter__",&pyInteractionContainer::pyIter)
		.def("__getitem__",&pyInteractionContainer::pyGetitem)
		.def("__len__",&pyInteractionContainer::len)
		.def("countReal",&pyInteractionContainer::countReal,"Return number of interactions that are \"real\", i.e. they have phys and geom.")
		.def("nth",&pyInteractionContainer::pyNth,"Return n-th interaction from the container (usable for picking random interaction).")
		.def("withBody",&pyInteractionContainer::withBody,"Return list of real interactions of given body.")
		.def("withBodyAll",&pyInteractionContainer::withBodyAll,"Return list of all (real as well as non-real) interactions of given body.")
		.def("eraseNonReal",&pyInteractionContainer::eraseNonReal,"Erase all interactions that are not :yref:`real <InteractionContainer.isReal>`.")
		.def("erase",&pyInteractionContainer::erase,"Erase one interaction, given by id1, id2 (internally, ``requestErase`` is called -- the interaction might still exist as potential, if the :yref:`Collider` decides so).")
		.def_property("serializeSorted",&pyInteractionContainer::serializeSorted_get,&pyInteractionContainer::serializeSorted_set)
		.def("clear",&pyInteractionContainer::clear,"Remove all interactions, and invalidate persistent collider data (if the collider supports it).");
	pybind11::class_<pyInteractionIterator>(m, "InteractionIterator").def(pybind11::init<const pyInteractionIterator&>())
		.def("__iter__",&pyInteractionIterator::pyIter)
		.def("__next__",&pyInteractionIterator::pyNext);

	pybind11::class_<pyForceContainer>(m, "ForceContainer").def(pybind11::init<const pyForceContainer&>())
		.def("f",&pyForceContainer::force_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Force applied on body. For clumps in openMP, synchronize the force container with sync=True, else the value will be wrong.")
		.def("t",&pyForceContainer::torque_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Torque applied on body. For clumps in openMP, synchronize the force container with sync=True, else the value will be wrong.")
		.def("m",&pyForceContainer::torque_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Deprecated alias for t (torque).")
		.def("move",&pyForceContainer::move_get,(pybind11::arg("id")),"Displacement applied on body.")
		.def("rot",&pyForceContainer::rot_get,(pybind11::arg("id")),"Rotation applied on body.")
		.def("addF",&pyForceContainer::force_add,pybind11::arg("id"),pybind11::arg("f"),pybind11::arg("permanent")=false,"Apply force on body (accumulates).\n\n # If permanent=false (default), the force applies for one iteration, then it is reset by ForceResetter. \n # If permanent=true, it persists over iterations, until it is overwritten by another call to addF(id,f,True) or removed by reset(resetAll=True). The permanent force on a body can be checked with permF(id).")
		.def("addT",&pyForceContainer::torque_add,pybind11::arg("id"),pybind11::arg("t"),pybind11::arg("permanent")=false,"Apply torque on body (accumulates). \n\n # If permanent=false (default), the torque applies for one iteration, then it is reset by ForceResetter. \n # If permanent=true, it persists over iterations, until it is overwritten by another call to addT(id,f,True) or removed by reset(resetAll=True). The permanent torque on a body can be checked with permT(id).")
		.def("permF",&pyForceContainer::permForce_get,(pybind11::arg("id")),"read the value of permanent force on body (set with setPermF()).")
		.def("permT",&pyForceContainer::permTorque_get,(pybind11::arg("id")),"read the value of permanent torque on body (set with setPermT()).")
		.def("addMove",&pyForceContainer::move_add,pybind11::arg("id"),pybind11::arg("m"),"Apply displacement on body (accumulates).")
		.def("addRot",&pyForceContainer::rot_add,pybind11::arg("id"),pybind11::arg("r"),"Apply rotation on body (accumulates).")
		.def("reset",&pyForceContainer::reset,(pybind11::arg("resetAll")=true),"Reset the force container, including user defined permanent forces/torques. resetAll=False will keep permanent forces/torques unchanged.")
		.def("getPermForceUsed",&pyForceContainer::getPermForceUsed,"Check wether permanent forces are present.")
		.def_property("syncCount",&pyForceContainer::syncCount_get,&pyForceContainer::syncCount_set,"Number of synchronizations  of ForceContainer (cummulative); if significantly higher than number of steps, there might be unnecessary syncs hurting performance.")
		;
	pybind11::class_<pyMaterialContainer>(m, "MaterialContainer", "Container for :yref:`Materials<Material>`. A material can be accessed using \n\n #. numerical index in range(0,len(cont)), like cont[2]; \n #. textual label that was given to the material, like cont['steel']. This etails traversing all materials and should not be used frequently.").def(pybind11::init<const pyMaterialContainer&>())
		.def("append",&pyMaterialContainer::append,"Add new shared :yref:`Material`; changes its id and return it.")
		.def("append",&pyMaterialContainer::appendList,"Append list of :yref:`Material` instances, return list of ids.")
		.def("index",&pyMaterialContainer::index,"Return id of material, given its label.")
		.def("__getitem__",&pyMaterialContainer::getitem_id)
		.def("__getitem__",&pyMaterialContainer::getitem_label)
		.def("__len__",&pyMaterialContainer::len);

//	py::class_<STLImporter>("STLImporter").def("ymport",&STLImporter::import);

//////////////////////////////////////////////////////////////
///////////// proxyless wrappers
	// Register core base classes that are not in ClassRegistry
	// These classes use REGISTER_CLASS_NAME which doesn't add them to ClassRegistry
	// Register core classes in ClassRegistry
	// They will be registered with pybind11 when ClassRegistry::registerAll() is called
	// Core classes are now registered manually with pybind11 below
	// We don't add them to ClassRegistry to avoid double-registration

	// std::cerr << "DEBUG: Starting core classes registration" << std::endl;
	// ClassFactory has been removed - using ClassRegistry for all class management

	// std::cerr << "DEBUG: Registering TimingDeltas..." << std::endl;
	pybind11::class_<TimingDeltas, shared_ptr<TimingDeltas>>(m, "TimingDeltas", pybind11::dynamic_attr()).def_property_readonly("data",&TimingDeltas::pyData,"Get timing data as list of tuples (label, execTime[nsec], execCount) (one tuple per checkpoint)").def("reset",&TimingDeltas::reset,"Reset timing information");

	// Register core classes with pybind11 first (base classes must be registered before derived)
	// std::cerr << "DEBUG: Registering Serializable with pybind11..." << std::endl;
	pybind11::class_<Serializable, std::shared_ptr<Serializable>>(m, "Serializable", "Base class for all serializable objects");
	// std::cerr << "DEBUG: Registering Engine with pybind11..." << std::endl;
	// Call Engine::pyRegisterClass to properly expose all attributes (dead, ompThreads, label, etc.)
	std::make_shared<Engine>()->pyRegisterClass(m);
	// std::cerr << "DEBUG: Registering Functor with pybind11..." << std::endl;
	pybind11::class_<Functor, Serializable, std::shared_ptr<Functor>>(m, "Functor", "Base class for functors");
	// std::cerr << "DEBUG: Registering Bound with pybind11..." << std::endl;
	pybind11::class_<Bound, Serializable, std::shared_ptr<Bound>>(m, "Bound", "Bounding volume");
	// std::cerr << "DEBUG: Registering Shape with pybind11..." << std::endl;
	pybind11::class_<Shape, Serializable, std::shared_ptr<Shape>>(m, "Shape", "Shape of a body");
	// std::cerr << "DEBUG: Registering State with pybind11..." << std::endl;
	pybind11::class_<State, Serializable, std::shared_ptr<State>>(m, "State", "State of a body")
		.def(pybind11::init<>())
		.def_property("pos", &State::pos_get, &State::pos_set, "Current position.")
		.def_readwrite("vel", &State::vel, "Current linear velocity.")
		.def_readwrite("mass", &State::mass, "Mass of this body")
		.def_readwrite("angVel", &State::angVel, "Current angular velocity")
		.def_readwrite("angMom", &State::angMom, "Current angular momentum")
		.def_readwrite("inertia", &State::inertia, "Inertia of associated body, in local coordinate system.")
		.def_readwrite("refPos", &State::refPos, "Reference position")
		.def_readwrite("refOri", &State::refOri, "Reference orientation")
		.def_readwrite("blockedDOFs_vec", &State::blockedDOFs, "[Will be overridden]")
		.def_property("blockedDOFs", &State::blockedDOFs_vec_get, &State::blockedDOFs_vec_set, "Degress of freedom where linear/angular velocity will be always constant (equal to zero, or to an user-defined value), regardless of applied force/torque. String that may contain 'xyzXYZ' (translations and rotations).")
		.def_readwrite("isDamped", &State::isDamped, "Damping can be deactivated for individual particles")
		.def_readwrite("densityScaling", &State::densityScaling, "Density scaling for time stepping")
		.def_property_readonly("dispIndex", [](std::shared_ptr<State> s){ return Indexable_getClassIndex(s); }, "Return class index of this instance.")
		.def("dispHierarchy", [](std::shared_ptr<State> s, bool names=true){ return Indexable_getClassIndices(s, names); }, pybind11::arg("names")=true, "Return list of dispatch classes")
		.def("displ", &State::displ, "Displacement from reference position")
		.def("rot", &State::rot, "Rotation from reference orientation");
	// std::cerr << "DEBUG: Registering Material with pybind11..." << std::endl;
	// Material, Body, IGeom, IPhys, Shape, and most derived classes are now registered by ClassRegistry
	// No need for manual registration - they will be registered with proper constructors via pyRegisterClass
	
	// However, functor template base classes need manual registration because they use complex templates
	// that cannot be easily auto-registered via ClassRegistry
	// std::cerr << "DEBUG: Registering functor base classes with pybind11..." << std::endl;
	pybind11::class_<BoundFunctor, Functor, std::shared_ptr<BoundFunctor>>(m, "BoundFunctor", "Bound functor")
		.def(pybind11::init<>());
	pybind11::class_<IGeomFunctor, Functor, std::shared_ptr<IGeomFunctor>>(m, "IGeomFunctor", "IGeom functor")
		.def(pybind11::init<>());
	pybind11::class_<IPhysFunctor, Functor, std::shared_ptr<IPhysFunctor>>(m, "IPhysFunctor", "IPhys functor")
		.def(pybind11::init<>());
	pybind11::class_<LawFunctor, Functor, std::shared_ptr<LawFunctor>>(m, "LawFunctor", "Law functor")
		.def(pybind11::init<>());
	
	// Register GL functor base classes for GUI rendering
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	// std::cerr << "DEBUG: Registering GL functor base classes with pybind11..." << std::endl;
	pybind11::class_<GlBoundFunctor, Functor, std::shared_ptr<GlBoundFunctor>>(m, "GlBoundFunctor", "GL bound functor")
		.def(pybind11::init<>());
	pybind11::class_<GlShapeFunctor, Functor, std::shared_ptr<GlShapeFunctor>>(m, "GlShapeFunctor", "GL shape functor")
		.def(pybind11::init<>());
	pybind11::class_<GlIGeomFunctor, Functor, std::shared_ptr<GlIGeomFunctor>>(m, "GlIGeomFunctor", "GL geometry functor")
		.def(pybind11::init<>());
	pybind11::class_<GlIPhysFunctor, Functor, std::shared_ptr<GlIPhysFunctor>>(m, "GlIPhysFunctor", "GL physics functor")
		.def(pybind11::init<>());
	pybind11::class_<GlStateFunctor, Functor, std::shared_ptr<GlStateFunctor>>(m, "GlStateFunctor", "GL state functor")
		.def(pybind11::init<>());
	
	// Also register GL functor base classes with ClassRegistry so OpenGLRenderer can find them
	// std::cerr << "DEBUG: Registering GL functor base classes with ClassRegistry..." << std::endl;
	ClassRegistry::instance().registerClassWithBase<GlBoundFunctor, Functor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<GlShapeFunctor, Functor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<GlIGeomFunctor, Functor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<GlIPhysFunctor, Functor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<GlStateFunctor, Functor>(__FILE__);
	
	// Register GL shape functors for rendering
	// std::cerr << "DEBUG: Registering GL shape functors with ClassRegistry..." << std::endl;
	ClassRegistry::instance().registerClassWithBase<Gl1_Wall, GlShapeFunctor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<Gl1_Fwall, GlShapeFunctor>(__FILE__);
	ClassRegistry::instance().registerClassWithBase<Gl1_Disk, GlShapeFunctor>(__FILE__);

#endif
	
	// std::cerr << "DEBUG: Core pybind11 registration done" << std::endl;

	// Auto-register all classes using the new ClassRegistry system
	#include<sudodem/lib/factory/ClassRegistry.hpp>
	// std::cerr << "DEBUG: ClassRegistry header included" << std::endl;
	
	// Explicitly register Superellipse classes
	
		// Explicitly register Wall class (used by utils.wall)
	std::cerr << "DEBUG: Registering Wall class..." << std::endl;
	ClassRegistry::instance().registerClassWithBase<Wall, Shape>(__FILE__);

	// First, register core base classes BEFORE anything else
	// This ensures all base classes are available when derived classes are registered
	std::cerr << "DEBUG: About to call initCoreClassRegistry..." << std::endl;
	::initCoreClassRegistry();
	std::cerr << "DEBUG: initCoreClassRegistry done, ClassRegistry size: " << ClassRegistry::instance().size() << std::endl;

	// Register all other classes through register_core_extra_classes
	std::cerr << "DEBUG: About to call register_core_extra_classes..." << std::endl;
	::register_core_extra_classes();
	std::cerr << "DEBUG: register_core_extra_classes done, ClassRegistry size: " << ClassRegistry::instance().size() << std::endl;

	// Register Engine base classes first (must be before registerAll)
	// std::cerr << "DEBUG: About to call register_engine_base_classes..." << std::endl;
	::register_engine_base_classes(m);
	// std::cerr << "DEBUG: register_engine_base_classes done" << std::endl;
	
	// std::cerr << "DEBUG: About to call ClassRegistry::registerAll..." << std::endl;
	try {
		ClassRegistry::instance().registerAll(m);
		std::cerr << "DEBUG: registerAll completed successfully" << std::endl;
		LOG_INFO("Auto-registered " << ClassRegistry::instance().size() << " classes with pybind11");
	} catch (const std::exception& e) {
		std::cerr << "ERROR in registerAll: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "ERROR in registerAll: unknown exception" << std::endl;
	}
	
	pybind11::module_(m).attr("O")=pyOmega();
	// 设置人类可读的类名，以便 ClassFactory 可以通过可读名称查找类
	// 这是为了支持 BoundDispatcher.add() 等方法使用 getClassName() 返回的可读名称
	// std::cerr << "DEBUG: Setting readable class names..." << std::endl;
	
	// BoundFunctor 类
	ClassRegistry::instance().setReadableClassName(typeid(Bo1_Disk_Aabb).name(), "Bo1_Disk_Aabb");
	ClassRegistry::instance().setReadableClassName(typeid(Bo1_Wall_Aabb).name(), "Bo1_Wall_Aabb");
	ClassRegistry::instance().setReadableClassName(typeid(Bo1_Fwall_Aabb).name(), "Bo1_Fwall_Aabb");
	ClassRegistry::instance().setReadableClassName(typeid(Bo1_Superellipse_Aabb).name(), "Bo1_Superellipse_Aabb");
	
	// IGeomFunctor 类
	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Disk_Disk_ScGeom).name(), "Ig2_Disk_Disk_ScGeom");
	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Wall_Disk_ScGeom).name(), "Ig2_Wall_Disk_ScGeom");
	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Fwall_Disk_ScGeom).name(), "Ig2_Fwall_Disk_ScGeom");

	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Wall_Superellipse_SuperellipseGeom).name(), "Ig2_Wall_Superellipse_SuperellipseGeom");
	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Superellipse_Superellipse_SuperellipseGeom).name(), "Ig2_Superellipse_Superellipse_SuperellipseGeom");
	ClassRegistry::instance().setReadableClassName(typeid(Ig2_Fwall_Superellipse_SuperellipseGeom).name(), "Ig2_Fwall_Superellipse_SuperellipseGeom");
	
	// IPhysFunctor 类
	ClassRegistry::instance().setReadableClassName(typeid(Ip2_FrictMat_FrictMat_FrictPhys).name(), "Ip2_FrictMat_FrictMat_FrictPhys");
	ClassRegistry::instance().setReadableClassName(typeid(Ip2_FrictMat_FrictMat_ViscoFrictPhys).name(), "Ip2_FrictMat_FrictMat_ViscoFrictPhys");

	ClassRegistry::instance().setReadableClassName(typeid(Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys).name(), "Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys");
	
	// LawFunctor 类
	ClassRegistry::instance().setReadableClassName(typeid(Law2_ScGeom_FrictPhys_CundallStrack).name(), "Law2_ScGeom_FrictPhys_CundallStrack");
	ClassRegistry::instance().setReadableClassName(typeid(Law2_ScGeom_ViscoFrictPhys_CundallStrack).name(), "Law2_ScGeom_ViscoFrictPhys_CundallStrack");

	ClassRegistry::instance().setReadableClassName(typeid(SuperellipseLaw).name(), "SuperellipseLaw");
	
	// Shape 类
	ClassRegistry::instance().setReadableClassName(typeid(Disk).name(), "Disk");
	ClassRegistry::instance().setReadableClassName(typeid(Superellipse).name(), "Superellipse");
	ClassRegistry::instance().setReadableClassName(typeid(Wall).name(), "Wall");
	ClassRegistry::instance().setReadableClassName(typeid(Fwall).name(), "Fwall");
	
	// IGeom 类
	ClassRegistry::instance().setReadableClassName(typeid(SuperellipseGeom).name(), "SuperellipseGeom");
	ClassRegistry::instance().setReadableClassName(typeid(ScGeom).name(), "ScGeom");
	
	// IPhys 类
	ClassRegistry::instance().setReadableClassName(typeid(SuperellipsePhys).name(), "SuperellipsePhys");
	ClassRegistry::instance().setReadableClassName(typeid(NormPhys).name(), "NormPhys");
	ClassRegistry::instance().setReadableClassName(typeid(NormShearPhys).name(), "NormShearPhys");
	ClassRegistry::instance().setReadableClassName(typeid(FrictPhys).name(), "FrictPhys");
	ClassRegistry::instance().setReadableClassName(typeid(ViscoFrictPhys).name(), "ViscoFrictPhys");
	
	// Material 类
	ClassRegistry::instance().setReadableClassName(typeid(ElastMat).name(), "ElastMat");
	ClassRegistry::instance().setReadableClassName(typeid(FrictMat).name(), "FrictMat");

	ClassRegistry::instance().setReadableClassName(typeid(SuperellipseMat).name(), "SuperellipseMat");
	
	// std::cerr << "DEBUG: Readable class names set" << std::endl;
	

	// Initialize merged submodules
	// std::cerr << "DEBUG: Initializing WeightedAverage2d submodule..." << std::endl;
	auto WeightedAverage2d = m.def_submodule("WeightedAverage2d", "Smoothing (2d gauss-weighted average) for postprocessing scalars in 2d.");
	pybind_init_WeightedAverage2d(WeightedAverage2d);
	
	// std::cerr << "DEBUG: Initializing _utils submodule..." << std::endl;
	auto _utils = m.def_submodule("_utils", "Utility functions for 2D SudoDEM.");
	pybind_init__utils(_utils);
	
	// std::cerr << "DEBUG: Initializing _customConverters submodule..." << std::endl;
	auto _customConverters = m.def_submodule("_customConverters", "Custom type converters for pybind11.");
	pybind_init__customConverters(_customConverters);
	
	// std::cerr << "DEBUG: Initializing _superellipse_utils submodule..." << std::endl;
	auto _superellipse_utils = m.def_submodule("_superellipse_utils", "Superellipse utility functions.");
	pybind_init__superellipse_utils(_superellipse_utils);
	
	// Expose submodules in main module namespace for direct import
	m.attr("_utils") = _utils;
	m.attr("_customConverters") = _customConverters;
	m.attr("_superellipse_utils") = _superellipse_utils;
	m.attr("WeightedAverage2d") = WeightedAverage2d;
	
	
	// Manually register engine classes that are NOT in ClassRegistry yet
	// Most engine classes are now registered by ClassRegistry system
	// Only BoundDispatcher is registered here (via SUDODEM_PLUGIN macro)
	// std::cerr << "DEBUG: Manually registering remaining engine classes..." << std::endl;
	
	// Register Dispatcher classes manually with proper constructors
	// These need manual registration because they have complex template bases
	#include<sudodem/pkg/common/Dispatching.hpp>
	// std::cerr << "DEBUG: Registering dispatcher classes with pybind11..." << std::endl;
	
	pybind11::class_<BoundDispatcher, Dispatcher, std::shared_ptr<BoundDispatcher>>(m, "BoundDispatcher", "Dispatcher calling :yref:`functors<BoundFunctor>` based on received argument type(s).")
		.def(pybind11::init<>())
		.def("add", static_cast<void (BoundDispatcher::*)(shared_ptr<BoundFunctor>)>(&BoundDispatcher::add), "Add functor to dispatcher.")
		.def_property("functors", &BoundDispatcher::functors_get, &BoundDispatcher::functors_set, "Functors associated with this dispatcher.")
		.def("dispMatrix", &BoundDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.")
		.def("dispFunctor", &BoundDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
	
	pybind11::class_<IGeomDispatcher, Dispatcher, std::shared_ptr<IGeomDispatcher>>(m, "IGeomDispatcher", "Dispatcher calling :yref:`functors<IGeomFunctor>` based on received argument type(s).")
		.def(pybind11::init<>())
		.def("add", static_cast<void (IGeomDispatcher::*)(shared_ptr<IGeomFunctor>)>(&IGeomDispatcher::add), "Add functor to dispatcher.")
		.def_property("functors", &IGeomDispatcher::functors_get, &IGeomDispatcher::functors_set, "Functors associated with this dispatcher.")
		.def("dispMatrix", &IGeomDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.")
		.def("dispFunctor", &IGeomDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
	
	pybind11::class_<IPhysDispatcher, Dispatcher, std::shared_ptr<IPhysDispatcher>>(m, "IPhysDispatcher", "Dispatcher calling :yref:`functors<IPhysFunctor>` based on received argument type(s).")
		.def(pybind11::init<>())
		.def("add", static_cast<void (IPhysDispatcher::*)(shared_ptr<IPhysFunctor>)>(&IPhysDispatcher::add), "Add functor to dispatcher.")
		.def_property("functors", &IPhysDispatcher::functors_get, &IPhysDispatcher::functors_set, "Functors associated with this dispatcher.")
		.def("dispMatrix", &IPhysDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.")
		.def("dispFunctor", &IPhysDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
	
	pybind11::class_<LawDispatcher, Dispatcher, std::shared_ptr<LawDispatcher>>(m, "LawDispatcher", "Dispatcher calling :yref:`functors<LawFunctor>` based on received argument type(s).")
		.def(pybind11::init<>())
		.def("add", static_cast<void (LawDispatcher::*)(shared_ptr<LawFunctor>)>(&LawDispatcher::add), "Add functor to dispatcher.")
		.def_property("functors", &LawDispatcher::functors_get, &LawDispatcher::functors_set, "Functors associated with this dispatcher.")
		.def("dispMatrix", &LawDispatcher::dump, pybind11::arg("names")=true, "Return dictionary with contents of the dispatch matrix.")
		.def("dispFunctor", &LawDispatcher::getFunctor, "Return functor that would be dispatched for given argument(s); None if no dispatch; ambiguous dispatch throws.");
	
	// std::cerr << "DEBUG: All classes registered" << std::endl;
}
