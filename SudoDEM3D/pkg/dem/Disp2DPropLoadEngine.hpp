/*************************************************************************
*  Copyright (C) 2008 by Jerome Durades                                   *
*  jerome.duriez@hmg.inpg.fr                                             *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/core/Omega.hpp>
#include<sudodem/pkg/common/BoundaryController.hpp>
#include<sudodem/core/Body.hpp>



class Disp2DPropLoadEngine : public BoundaryController
{
	private :
		Real dgamma;
		Real dh;
		Real H0;
		Real X0;
		Real Fn0;
		Real Ft0;
		Real coordSs0;
		Real coordTot0;
		std::ofstream ofile;

		Real alpha;
		Real dalpha;

		bool firstIt;

		int it_begin;

		shared_ptr<Body> leftbox;
		shared_ptr<Body> rightbox;
		shared_ptr<Body> frontbox;
		shared_ptr<Body> backbox;
		shared_ptr<Body> topbox;
		shared_ptr<Body> boxbas;
		
	public:
		Body::id_t id_topbox;
		Body::id_t id_boxbas;
		Body::id_t id_boxleft;
		Body::id_t id_boxright;
		Body::id_t id_boxfront;
		Body::id_t id_boxback;
		Real theta;
		Real v;
		int nbre_iter;
		string Key;
		bool LOG;
		
		void saveData();
		void letDisturb();
		void stopMovement();
		void action();
		void computeAlpha();
		void postLoad(Disp2DPropLoadEngine&);

	// Explicit constructor with initial values
	Disp2DPropLoadEngine(): id_topbox(3), id_boxbas(1), id_boxleft(0), id_boxright(2), id_boxfront(5), id_boxback(4), theta(0.0), v(0.0), nbre_iter(0), firstIt(true), alpha(Mathr::PI/2.0), dgamma(0.), dh(0.), H0(0.), X0(0.), Fn0(0.), Ft0(0.), coordSs0(0.), coordTot0(0.), dalpha(0.), it_begin(0), LOG(false) {}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(BoundaryController, id_topbox, id_boxbas, id_boxleft, id_boxright, id_boxfront, id_boxback, theta, v, nbre_iter, Key, LOG);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(Disp2DPropLoadEngine, BoundaryController);