// 2009 © Václav Šmilauer <eudoxos@arcig.cz>

#pragma once

#include<sudodem/pkg/common/BoundaryController.hpp>
/* Engine for independently controlling stress or strain in periodic simulations.

strainStress contains absolute values for the controlled quantity, and stressMask determines
meaning of those values (0 for strain, 1 for stress): e.g. ( 1<<0 | 1<<2 ) = 1 | 4 = 5 means that
strainStress[0] and strainStress[2] are stress values, and strainStress[1] is strain.

See scripts/test/periodic-triax.py for a simple example.

*/

class PeriTriaxController: public BoundaryController{
	public:
		bool dynCell;
		Real z_dim;
		Vector2r goal;
		int stressMask;
		Vector2r maxStrainRate;
		Real maxUnbalanced;
		Real absStressTol;
		Real relStressTol;
		Real growDamping;
		int globUpdate;
		std::string doneHook;
		Vector2r maxBodySpan;
		Matrix2r stressTensor;
		Vector2r stress;
		Vector2r strain;
		Vector2r strainRate;
		Vector2r stiff;
		Real currUnbalanced;
		Vector2r prevGrow;
		Real mass;
		Real externalWork;
		int velGradWorkIx;

		std::string label;

		virtual void action() override;
		void strainStressStiffUpdate();
		
		PeriTriaxController() : dynCell(true), z_dim(1.0), goal(Vector2r::Zero()), stressMask(0), maxStrainRate(Vector2r(1,1)), maxUnbalanced(1e-4), absStressTol(1e3), relStressTol(3e-5), growDamping(.25), globUpdate(5), doneHook(""), maxBodySpan(Vector2r::Zero()), stressTensor(Matrix2r::Zero()), stress(Vector2r::Zero()), strain(Vector2r::Zero()), strainRate(Vector2r::Zero()), stiff(Vector2r::Zero()), currUnbalanced(NaN), prevGrow(Vector2r::Zero()), mass(NaN), externalWork(0), velGradWorkIx(-1) {}
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
		
		
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(PeriTriaxController, GlobalEngine);