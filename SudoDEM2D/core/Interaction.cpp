/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include"Interaction.hpp"

#include<sudodem/core/Scene.hpp>

Interaction::Interaction(Body::id_t newId1,Body::id_t newId2): id1(newId1), id2(newId2), cellDist(Vector2i(0,0)){ reset(); }

bool Interaction::isFresh(Scene* rb){ return iterMadeReal==rb->iter;}

void Interaction::init(){
	iterMadeReal=-1;
	functorCache.geomExists=true;
	isActive=true;
}

void Interaction::reset(){
	geom=shared_ptr<IGeom>();
	phys=shared_ptr<IPhys>();
	init();
}


void Interaction::swapOrder(){
	if(geom || phys){
		throw std::logic_error("Bodies in interaction cannot be swapped if they have geom or phys.");
	}
	std::swap(id1,id2);
	cellDist*=-1;
}

void Interaction::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Interaction");
	pybind11::class_<Interaction, Serializable, std::shared_ptr<Interaction>> _classObj(_module, "Interaction", "Interaction between pair of bodies.");
	_classObj.def(pybind11::init<>());
	_classObj.def(pybind11::init<Body::id_t, Body::id_t>());
	_classObj.def_readonly("id1", &Interaction::id1, ":yref:`Id<Body::id>` of the first body in this interaction.");
	_classObj.def_readonly("id2", &Interaction::id2, ":yref:`Id<Body::id>` of the second body in this interaction.");
	_classObj.def_readwrite("iterMadeReal", &Interaction::iterMadeReal, "Step number at which the interaction was fully (in the sense of geom and phys) created. (Should be touched only by :yref:`IPhysDispatcher` and :yref:`InteractionLoop`, therefore they are made friends of Interaction)");
	_classObj.def_readwrite("iterLastSeen", &Interaction::iterLastSeen, "At which step this interaction was last detected by the collider. InteractionLoop will remove it if InteractionContainer::iterColliderLastRun==scene->iter, InteractionContainer::iterColliderLastRun is positive (some colliders manage interaction deletion themselves, such as :yref:`InsertionSortCollider`) and iterLastSeen<scene->iter.");
	_classObj.def_readwrite("geom", &Interaction::geom, "Geometry part of the interaction.");
	_classObj.def_readwrite("phys", &Interaction::phys, "Physical (material) part of the interaction.");
	_classObj.def_readwrite("cellDist", &Interaction::cellDist, "Distance of bodies in cell size units, if using periodic boundary conditions; id2 is shifted by this number of cells from its :yref:`State::pos` coordinates for this interaction to exist. Assigned by the collider.");
	_classObj.def_readwrite("linIx", &Interaction::linIx, "Index in the linear interaction container. For internal use by InteractionContainer only.");
	_classObj.def_readwrite("iterBorn", &Interaction::iterBorn, "Step number at which the interaction was added to simulation.");
	_classObj.def_property_readonly("isReal", &Interaction::isReal, "True if this interaction has both geom and phys; False otherwise.");
	_classObj.def_readwrite("isActive", &Interaction::isActive, "True if this interaction is active. Otherwise the forces from this interaction will not be taken into account. True by default.");
	_classObj.def("getId1", &Interaction::getId1, "Get id of first body");
	_classObj.def("getId2", &Interaction::getId2, "Get id of second body");
	_classObj.def("swapOrder", &Interaction::swapOrder, "swaps order of bodies within the interaction");
	_classObj.def("reset", &Interaction::reset, "Reset interaction to the intial state (keep only body ids)");
}
