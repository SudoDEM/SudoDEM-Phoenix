// Include all necessary headers
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <sudodem/lib/base/Math.hpp>
#include <sudodem/core/Omega.hpp>
#include <sudodem/core/Body.hpp>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/core/Material.hpp>
#include <sudodem/core/Clump.hpp>
#include <sudodem/core/Timing.hpp>
#include <sudodem/lib/serialization/ObjectIO.hpp>
#include <sudodem/pkg/common/Sphere.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include <sudodem/lib/pyutil/gil.hpp>

#include <thread>
#include <chrono>
#include <limits>

namespace py = pybind11;
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
		typedef std::map<Body::id_t,Se3r> MemberMap;
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
				std::map<Body::id_t,Se3r>& members = clump->members;
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
		//set a random generator (code copied from pkg/dem/SpherePack.cpp):
		static std::mt19937 randGen(static_cast<unsigned int>(std::time(nullptr)));
		static std::uniform_real_distribution<> randDist(-1, 1);

		//get number of spherical particles and a list of all spheres:
		vector<shared_ptr<Body> > sphereList;
		shared_ptr<Sphere> sph (new Sphere);
		int Sph_Index = sph->getClassIndexStatic();
		for (const shared_ptr<Body>& b : *proxee) if ( (b->shape->getClassIndex() == Sph_Index) && (b->isStandalone()) ) sphereList.push_back(b);
		int num = sphereList.size();

		//loop over templates:
		int numSphereList = num, numTemplates = amounts.size();
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
			vector<Vector3r> relPosTmp(numCM);
			Vector3r relPosTmpMean = Vector3r::Zero();
			Real rMin=std::numeric_limits<Real>::infinity(); AlignedBox3r aabb;
			for (int jj = 0; jj < numCM; jj++) {
				relRadTmp[jj] = pybind11::cast<Real>(relRadListTmp[jj]);
				relVolTmp[jj] = (4./3.)*Mathr::PI*pow(relRadTmp[jj],3.);
				relPosTmp[jj] = pybind11::cast<Vector3r>(relPosListTmp[jj]);
				relPosTmpMean += relPosTmp[jj];
				aabb.extend(relPosTmp[jj] + Vector3r::Constant(relRadTmp[jj]));
				aabb.extend(relPosTmp[jj] - Vector3r::Constant(relRadTmp[jj]));
				rMin=min(rMin,relRadTmp[jj]);
			}
			relPosTmpMean /= numCM;//balance point

			//get volume of the clump template using regular cubic cell array inside axis aligned bounding box of the clump:
			//(some parts are duplicated from intergration algorithm in Clump::updateProperties)
			Real dx = rMin/5.; 	//edge length of cell
			Real aabbMax = max(aabb.max().x()-aabb.min().x(),aabb.max().y()-aabb.min().y());
			if (aabbMax/dx > 150) dx = aabbMax/150;//limit dx
			Real dv = pow(dx,3);		//volume of a single cell
			Vector3r x;			//position vector (center) of cell
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
						distCMTmp = (relPosTmp[jj] - relPosTmp[kk]).norm(); //distance between two spheres
						overlapTmp = (relRadTmp[jj] + relRadTmp[kk]) - distCMTmp;//positive if overlapping ...
						if (overlapTmp > 0.0) {//calculation of overlapping spheres, see http://mathworld.wolfram.com/Sphere-SphereIntersection.html
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

			//get pointer lists of spheres, that should be replaced:
			int numReplaceTmp = round(num*amounts[ii]);
			vector<shared_ptr<Body> > bpListTmp(numReplaceTmp);
			int a = 0, c = 0;//counters
			vector<int> posTmp;
			for (const shared_ptr<Body>& b : sphereList) {
				if (c == a*numSphereList/numReplaceTmp) {
					bpListTmp[a] = b; a++;
					posTmp.push_back(c);//remember position in sphereList
				} c++;
			}
			for (int jj = 0; jj < a; jj++) {
				sphereList.erase(sphereList.begin()+posTmp[jj]-jj);//remove bodies from sphereList, that were already found
				numSphereList--;
			}

			//adapt position- and radii-informations and replace spheres from bpListTmp by clumps:
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
				//get sphere, that should be replaced:
				const Sphere* sphere = SUDODEM_CAST<Sphere*> (b->shape.get());
				shared_ptr<Material> matTmp = b->material;

				//get a random rotation quaternion:
				//Quaternionr randAxisTmp = (Quaternionr) AngleAxisr(2*Mathr::PI*randGen(),Vector3r(randGen(),randGen());
				//randAxisTmp.normalize();

				//convert geometries in global coordinates (scaling):
				Real scalingFactorVolume = ((4./3.)*Mathr::PI*pow(sphere->radius,3.))/relVolSumTmp;
				Real scalingFactor1D = pow(scalingFactorVolume,1./3.);//=((vol. sphere)/(relative clump volume))^(1/3)
				vector<Vector3r> newPosTmp(numCM);
				vector<Real> newRadTmp(numCM);
				vector<Body::id_t> idsTmp(numCM);
				for (int jj = 0; jj < numCM; jj++) {
					newPosTmp[jj] = relPosTmp[jj] - relPosTmpMean;	//shift position, to get balance point at (0,0,0)
					newPosTmp[jj] =/* randAxisTmp*/newPosTmp[jj];	//rotate around balance point
					newRadTmp[jj] = relRadTmp[jj] * scalingFactor1D;//scale radii
					newPosTmp[jj] = newPosTmp[jj] * scalingFactor1D;//scale position
					newPosTmp[jj] += b->state->pos;			//translate new position to spheres center

					//create spheres:
					shared_ptr<Body> newSphere = shared_ptr<Body>(new Body());
					newSphere->state->blockedDOFs = State::DOF_NONE;
					newSphere->state->mass = scalingFactorVolume*relVolTmp[jj]*matTmp->density;//vol. corrected mass for clump members
					Real inertiaTmp = 2.0/5.0*newSphere->state->mass*newRadTmp[jj]*newRadTmp[jj];
					newSphere->state->inertia	= Vector3r(inertiaTmp, inertiaTmp, inertiaTmp);
					newSphere->state->pos = newPosTmp[jj];
					newSphere->material = matTmp;

					shared_ptr<Sphere> sphereTmp = shared_ptr<Sphere>(new Sphere());
					sphereTmp->radius = newRadTmp[jj];
					sphereTmp->color = Vector3r(Mathr::UnitRandom(),Mathr::UnitRandom(),Mathr::UnitRandom());
					sphereTmp->color.normalize();
					newSphere->shape = sphereTmp;

					shared_ptr<Aabb> aabbTmp = shared_ptr<Aabb>(new Aabb());
					aabbTmp->color = Vector3r(0,1,0);
					newSphere->bound = aabbTmp;
					proxee->insert(newSphere);
					LOG_DEBUG("New body (sphere) "<<newSphere->id<<" added.");
					idsTmp[jj] = newSphere->id;
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
		shared_ptr<Sphere> sph (new Sphere);
		int Sph_Index = sph->getClassIndexStatic();		// get sphere index for checking if bodies are spheres
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
					std::map<Body::id_t,Se3r>& members = clump->members;
					for (MemberMap::value_type& mm : members){
						const Body::id_t& memberId=mm.first;
						const shared_ptr<Body>& member=Body::byId(memberId,scene);
						assert(member->isClumpMember());
						if (member->shape->getClassIndex() ==  Sph_Index){//clump member should be a sphere
							const Sphere* sphere = SUDODEM_CAST<Sphere*> (member->shape.get());
							R2 = max((member->state->pos - b->state->pos).norm() + sphere->radius, R2);	//get minimum radius of a sphere, that imbeds clump
							dens = member->material->density;
						}
					}
					if (dens > 0.) vol = b->state->mass/dens;
					R1 = pow((3.*vol)/(4.*Mathr::PI),1./3.);	//get theoretical radius of a sphere, with same volume as clump
					if (R2 < R1) {PyErr_Warn(PyExc_UserWarning,("Something went wrong in getRoundness method (R2 < R1 detected).")); return 0;}
					RC_sum += R1/R2; c += 1;
				}
			}
		}
		if (c == 0) c = 1;	//in case no spheres and no clumps are present in the scene: RC = 0
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
		Vector3r force_get(long id, bool sync){  checkId(id); if (!sync) return scene->forces.getForceSingle(id); scene->forces.sync(); return scene->forces.getForce(id);}
		Vector3r torque_get(long id, bool sync){ checkId(id); if (!sync) return scene->forces.getTorqueSingle(id); scene->forces.sync(); return scene->forces.getTorque(id);}
		Vector3r move_get(long id){ checkId(id); return scene->forces.getMoveSingle(id); }
		Vector3r rot_get(long id){ checkId(id); return scene->forces.getRotSingle(id); }
		void force_add(long id, const Vector3r& f, bool permanent){  checkId(id); if (!permanent) scene->forces.addForce (id,f); else scene->forces.addPermForce (id,f); }
		void torque_add(long id, const Vector3r& t, bool permanent){ checkId(id); if (!permanent) scene->forces.addTorque(id,t); else scene->forces.addPermTorque(id,t);}
		void move_add(long id, const Vector3r& t){   checkId(id); scene->forces.addMove(id,t);}
		void rot_add(long id, const Vector3r& t){    checkId(id); scene->forces.addRot(id,t);}
		Vector3r permForce_get(long id){  checkId(id); return scene->forces.getPermForce(id);}
		Vector3r permTorque_get(long id){  checkId(id); return scene->forces.getPermTorque(id);}
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
		timespec t1,t2; t1.tv_sec=0; t1.tv_nsec=40000000; /* 40 ms */ 
		Py_BEGIN_ALLOW_THREADS; 
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
