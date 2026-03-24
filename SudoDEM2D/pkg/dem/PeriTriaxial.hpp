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
		string doneHook;
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
		
		virtual void action() override;
		void strainStressStiffUpdate();
		
		PeriTriaxController() : dynCell(true), z_dim(1.0), goal(Vector2r::Zero()), stressMask(0), maxStrainRate(Vector2r(1,1)), maxUnbalanced(1e-4), absStressTol(1e3), relStressTol(3e-5), growDamping(.25), globUpdate(5), doneHook(""), maxBodySpan(Vector2r::Zero()), stressTensor(Matrix2r::Zero()), stress(Vector2r::Zero()), strain(Vector2r::Zero()), strainRate(Vector2r::Zero()), stiff(Vector2r::Zero()), currUnbalanced(NaN), prevGrow(Vector2r::Zero()), mass(NaN), externalWork(0), velGradWorkIx(-1) {}
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<PeriTriaxController, BoundaryController, std::shared_ptr<PeriTriaxController>> _classObj(_module, "PeriTriaxController", "Engine for independently controlling stress or strain in periodic simulations.\\n\\n\\`\\`strainStress\\`\\` contains absolute values for the controlled quantity, and \\`\\`stressMask\\`\\` determines meaning of those values (0 for strain, 1 for stress): e.g. \\`\\`( 1<<0 | 1<<2 ) = 1 | 4 = 5\\`\\` means that \\`\\`strainStress[0]\\`\\` and \\`\\`strainStress[2]\\`\\` are stress values, and \\`\\`strainStress[1]\\`\\` is strain. \\n\\nSee scripts/test/periodic-triax.py for a simple example.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("dynCell", &PeriTriaxController::dynCell, "Imposed stress can be controlled using the packing stiffness or by applying the laws of dynamic (dynCell=true). Don't forget to assign a :yref:\\`mass<PeriTriaxController.mass>\\` to the cell.");
			_classObj.def_readwrite("z_dim", &PeriTriaxController::z_dim, "The extention along the z direction, like the length of cyliners in 3D. The value is involed in calculation of the stress tensor, and a recommended value is the mean particle size (FIXME)");
			_classObj.def_readwrite("goal", &PeriTriaxController::goal, "Desired stress or strain values (depending on stressMask), strains defined as \\`\\`strain(i)=log(Fii)\\`\\`.\\n\\n.. warning:: Strains are relative to the :yref:\\`O.cell.refSize<Cell.refSize>\\` (reference cell size), not the current one (e.g. at the moment when the new strain value is set).");
			_classObj.def_readwrite("stressMask", &PeriTriaxController::stressMask, "mask determining strain/stress (0/1) meaning for goal components");
			_classObj.def_readwrite("maxStrainRate", &PeriTriaxController::maxStrainRate, "Maximum strain rate of the periodic cell.");
			_classObj.def_readwrite("maxUnbalanced", &PeriTriaxController::maxUnbalanced, "maximum unbalanced force.");
			_classObj.def_readwrite("absStressTol", &PeriTriaxController::absStressTol, "Absolute stress tolerance");
			_classObj.def_readwrite("relStressTol", &PeriTriaxController::relStressTol, "Relative stress tolerance");
			_classObj.def_readwrite("growDamping", &PeriTriaxController::growDamping, "Damping of cell resizing (0=perfect control, 1=no control at all); see also \\`\\`wallDamping\\`\\` in :yref:\\`TriaxialStressController\\`.");
			_classObj.def_readwrite("globUpdate", &PeriTriaxController::globUpdate, "How often to recompute average stress, stiffness and unbalaced force.");
			_classObj.def_readwrite("doneHook", &PeriTriaxController::doneHook, "python command to be run when the desired state is reached");
			_classObj.def_readwrite("maxBodySpan", &PeriTriaxController::maxBodySpan, "maximum body dimension |ycomp|");
			_classObj.def_readwrite("stressTensor", &PeriTriaxController::stressTensor, "average stresses, updated at every step (only every globUpdate steps recomputed from interactions if !dynCell)");
			_classObj.def_readwrite("stress", &PeriTriaxController::stress, "diagonal terms of the stress tensor");
			_classObj.def_readwrite("strain", &PeriTriaxController::strain, "cell strain |yupdate|");
			_classObj.def_readwrite("strainRate", &PeriTriaxController::strainRate, "cell strain rate |yupdate|");
			_classObj.def_readwrite("stiff", &PeriTriaxController::stiff, "average stiffness (only every globUpdate steps recomputed from interactions) |yupdate|");
			_classObj.def_readwrite("currUnbalanced", &PeriTriaxController::currUnbalanced, "current unbalanced force (updated every globUpdate) |yupdate|");
			_classObj.def_readwrite("prevGrow", &PeriTriaxController::prevGrow, "previous cell grow");
			_classObj.def_readwrite("mass", &PeriTriaxController::mass, "mass of the cell (user set); if not set and :yref:\\`dynCell<PeriTriaxController.dynCell>\\` is used, it will be computed as sum of masses of all particles.");
			_classObj.def_readwrite("externalWork", &PeriTriaxController::externalWork, "Work input from boundary controller.");
		}
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE_BASE(PeriTriaxController, GlobalEngine);