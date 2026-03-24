// 2008 © Sergei Dorofeenko <sega@users.berlios.de>
// 2009,2010 © Václav Šmilauer <eudoxos@arcig.cz>

#include "InteractionContainer.hpp"
#include "Scene.hpp"

#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif
CREATE_LOGGER(InteractionContainer);
// begin internal functions

bool InteractionContainer::insert(const shared_ptr<Interaction>& i){
	assert(bodies);
	std::lock_guard<std::mutex> lock(drawloopmutex);

	Body::id_t id1=i->getId1();
	Body::id_t id2=i->getId2();

	if (id1>id2) swap(id1,id2);

	assert((Body::id_t)bodies->size()>id1); // the bodies must exist already
	assert((Body::id_t)bodies->size()>id2);

	const shared_ptr<Body>& b1=(*bodies)[id1];
	const shared_ptr<Body>& b2=(*bodies)[id2];

	if(!b1->intrs.insert(Body::MapId2IntrT::value_type(id2,i)).second) return false; // already exists
	if(!b2->intrs.insert(Body::MapId2IntrT::value_type(id1,i)).second) return false;

	linIntrs.resize(++currSize); // currSize updated
	linIntrs[currSize-1]=i; // assign last element
	i->linIx=currSize-1; // store the index back-reference in the interaction (so that it knows how to erase/move itself)

	const shared_ptr<Scene>& scene=Omega::instance().getScene();
	i->iterBorn=scene->iter;

	return true;
}


void InteractionContainer::clear(){
	assert(bodies);
	std::lock_guard<std::mutex> lock(drawloopmutex);
	for(const shared_ptr<Body>& b : *bodies) {
		if (b) b->intrs.clear(); // delete interactions from bodies
	}
	linIntrs.clear(); // clear the linear container
	currSize=0;
	dirty=true;
}


bool InteractionContainer::erase(Body::id_t id1,Body::id_t id2, int linPos){
	assert(bodies);
	std::lock_guard<std::mutex> lock(drawloopmutex);
	if (id1>id2) swap(id1,id2);
	if(id2>=(Body::id_t)bodies->size()) return false; // no such interaction

	const shared_ptr<Body>& b1((*bodies)[id1]);
	const shared_ptr<Body>& b2((*bodies)[id2]);
	LOG_DEBUG("InteractionContainer erase intrs id1=" << id1 << " id2=" << id2);
	int linIx=-1;
	if(!b1) linIx=linPos;
	else {
		Body::MapId2IntrT::iterator I(b1->intrs.find(id2));
		if(I==b1->intrs.end()) linIx=linPos;
		else {
			linIx=I->second->linIx;
			LOG_DEBUG("InteractionContainer linIx=" << linIx << " linPos=" << linPos);
			assert(linIx==linPos);
			//erase from body, we also erase from linIntrs below
			b1->intrs.erase(I);
			if (b2) {
				Body::MapId2IntrT::iterator I2(b2->intrs.find(id1));
				if (not(I2==b2->intrs.end())) {
					b2->intrs.erase(I2);
				}
			}
		}

	}
	if(linIx<0) return false;
	// Mark interaction as erased
	linIntrs[linIx]->linIx=-1;
	// If not the last element, swap last element to this position
	if(linIx<(int)currSize-1) {
		linIntrs[linIx]=linIntrs[currSize-1];
		linIntrs[linIx]->linIx=linIx; // update back-reference of moved element
	}
	// Resize to remove last element and decrement currSize
	linIntrs.resize(--currSize);
	dirty=true;
	return true;
}

bool InteractionContainer::insert(Body::id_t id1, Body::id_t id2){
	shared_ptr<Interaction> i(new Interaction(id1,id2));
	return insert(i);
}

const shared_ptr<Interaction>& InteractionContainer::find(Body::id_t id1, Body::id_t id2){
	// find the interaction object
	Body::id_t _id1(id1), _id2(id2);
	if(_id1>_id2) std::swap(_id1,_id2); // order ids
	const shared_ptr<Body>& b((*bodies)[_id1]);
	Body::MapId2IntrT::iterator i=b->intrs.find(_id2);
	if(i==b->intrs.end()) return empty;
	return i->second;
}

void InteractionContainer::requestErase(Interaction* I){
	I->reset();
}

void InteractionContainer::requestErase(Body::id_t id1, Body::id_t id2){
	const shared_ptr<Interaction> I=find(id1,id2);
	if(!I) return;
	I->reset();
}

void InteractionContainer::requestErase(const shared_ptr<Interaction>& I){
	I->reset();
}

void InteractionContainer::eraseNonReal(){
	for(const shared_ptr<Interaction>& i : *this) if(!i->isReal()) this->erase(i->getId1(),i->getId2());
}

// compare interaction based on their first id
struct compPtrInteraction{
	bool operator() (const shared_ptr<Interaction>& i1, const shared_ptr<Interaction>& i2) const {
		return (*i1)<(*i2);
	}
};

void InteractionContainer::preSave(InteractionContainer&){
	for(const shared_ptr<Interaction>& I : *this){
		if(I->geom || I->phys) interaction.push_back(I);
		// since requestErase'd interactions have no interaction physics/geom, they are not saved
	}
	if(serializeSorted) std::sort(interaction.begin(),interaction.end(),compPtrInteraction());
}
void InteractionContainer::postSave(InteractionContainer&){ interaction.clear(); }


void InteractionContainer::preLoad(InteractionContainer&){ interaction.clear(); }

void InteractionContainer::postLoad__calledFromScene(const shared_ptr<BodyContainer>& bb){
	bodies=&bb->body; // update the internal pointer
	clear();
	for(const shared_ptr<Interaction>& I : interaction){
		Body::id_t id1=I->getId1(), id2=I->getId2();
		if (!(*bodies)[id1] || !(*bodies)[id2]) {
			return;
		} else {
			insert(I);
		}
	}
	interaction.clear();
}