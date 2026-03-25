/*************************************************************************
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#include <sudodem/pkg/common/Collider.hpp>


int Collider::avoidSelfInteractionMask = 0 ;

bool Collider::mayCollide(const Body* b1, const Body* b2){
	return
		// might be called with deleted bodies, i.e. NULL pointers
		(b1!=NULL && b2!=NULL) &&
		// only collide if at least one particle is standalone or they belong to different clumps
		(b1->isStandalone() || b2->isStandalone() || b1->clumpId!=b2->clumpId ) &&
		 // do not collide clumps, since they are just containers, never interact
		!b1->isClump() && !b2->isClump() &&
		// masks must have at least 1 bit in common
		b1->maskCompatible(b2->groupMask) &&
		// avoid contact between particles having the same mask compatible with the avoidSelfInteractionMask.
 		!( (b1->groupMask == b2->groupMask) && b1->maskCompatible(avoidSelfInteractionMask) )
	;
}

void Collider::pyHandleCustomCtorArgs(pybind11::tuple& t, pybind11::dict& d){
	if(pybind11::len(t)==0) return; // nothing to do
	if(pybind11::len(t)!=1) throw invalid_argument(("Collider optionally takes exactly one list of BoundFunctor's as non-keyword argument for constructor ("+std::to_string(pybind11::len(t))+" non-keyword ards given instead)").c_str());
	typedef std::vector<shared_ptr<BoundFunctor> > vecBound;
	vecBound vb=t[0].cast<vecBound>();
	for (auto bf : vb) this->boundDispatcher->add(bf);
	t=pybind11::tuple(); // empty the args
}

void Collider::findBoundDispatcherInEnginesIfNoFunctorsAndWarn(){
	if(boundDispatcher->functors.size()>0) return;
	shared_ptr<BoundDispatcher> bd;
	for (auto& e : scene->engines){ bd=SUDODEM_PTR_DYN_CAST<BoundDispatcher>(e); if(bd) break; }
	if(!bd) return;
	LOG_WARN("Collider.boundDispatcher had no functors defined, while there was a BoundDispatcher found in O.engines. Since version 0.60 (r2396), Collider has boundDispatcher integrated in itself; separate BoundDispatcher should not be used anymore. For now, I will fix it for you, but change your script! Where it reads e.g.\n\n\tO.engines=[...,\n\t\tBoundDispatcher([Bo1_Disk_Aabb(),Bo1_Facet_Aabb()]),\n\t\t"<<getClassName()<<"(),\n\t\t...\n\t]\n\nit should become\n\n\tO.engines=[...,\n\t\t"<<getClassName()<<"([Bo1_Disk_Aabb(),Bo1_Facet_Aabb()]),\n\t\t...\n\t]\n\ninstead.")
	boundDispatcher=bd;
	boundDispatcher->activated=false; // deactivate in the engine loop, the collider will call it itself
}

