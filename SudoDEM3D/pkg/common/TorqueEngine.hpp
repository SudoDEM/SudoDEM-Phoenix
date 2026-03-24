/*************************************************************************
*  Copyright (C) 2008 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include <sudodem/core/PartialEngine.hpp>
#include <sudodem/core/Scene.hpp>

class TorqueEngine: public PartialEngine{
	public:
		Vector3r moment;
		
		virtual void action() override {
			for(const Body::id_t id : ids){
				scene->forces.addTorque(id,moment);
			}
		}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(TorqueEngine, PartialEngine);
