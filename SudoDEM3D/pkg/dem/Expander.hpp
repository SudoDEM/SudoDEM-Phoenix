/*************************************************************************
*  Copyright (C) 2016 by Zhswee                                          *
*  zhswee@gmail.com                                                      *
*  Engine for expanding polySuperEllipsoids                              *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include<sudodem/pkg/common/BoundaryController.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/pkg/dem/Superquadrics.hpp>
#include<array>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/pkg/common/Facet.hpp>
#include<fstream>
#include<string>
#ifdef SUDODEM_OPENMP
	#include<omp.h>
#endif

class State;

class Scene;
class State;


class Expander : public BoundaryController
{
	private :
		bool firstRun;
		std::vector<double> expandAlphaList;
		Real curExpandSlice;
	public :
		double initExpandAlpha;
		short int expandSlice;
		double preExpandAlpha;
		int cycleInterval;
		
		Expander() : firstRun(true), curExpandSlice(0), initExpandAlpha(0.1), 
			expandSlice(100), preExpandAlpha(0.1), cycleInterval(100) {}
		
		virtual ~Expander();
		virtual void action();
		void initial();
		void expand();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(BoundaryController, initExpandAlpha, expandSlice, preExpandAlpha, cycleInterval);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Expander, BoundaryController);