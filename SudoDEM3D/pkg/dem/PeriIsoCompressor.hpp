// 2009 © Václav Šmilauer <eudoxos@arcig.cz>

#pragma once

#include<sudodem/pkg/common/BoundaryController.hpp>

class PeriIsoCompressor: public BoundaryController{
	public:
		Real avgStiffness;
		Real maxDisplPerStep;
		Vector3r sumForces;
		Vector3r sigma;
		Real currUnbalanced;
		vector<Real> stresses;
		Real charLen;
		Real maxSpan;
		Real maxUnbalanced;
		int globalUpdateInt;
		size_t state;
		string doneHook;
		bool keepProportions;
		
		PeriIsoCompressor() : avgStiffness(-1), maxDisplPerStep(-1), sumForces(Vector3r::Zero()), sigma(Vector3r::Zero()), 
			currUnbalanced(-1), charLen(-1.), maxSpan(-1.), maxUnbalanced(1e-4), globalUpdateInt(20), 
			state(0), keepProportions(true) {}
		
		void action();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(BoundaryController, stresses, charLen, maxSpan, maxUnbalanced, globalUpdateInt, 
			state, doneHook, keepProportions, avgStiffness, maxDisplPerStep, sumForces, sigma, currUnbalanced);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PeriIsoCompressor, BoundaryController);

/* Engine for independently controlling stress or strain in periodic simulations.

strainStress contains absolute values for the controlled quantity, and stressMask determines
meaning of those values (0 for strain, 1 for stress): e.g. ( 1<<0 | 1<<2 ) = 1 | 4 = 5 means that
strainStress[0] and strainStress[2] are stress values, and strainStress[1] is strain.

See scripts/test/periodic-triax.py for a simple example.

*/

class PeriTriaxController: public BoundaryController{
	public:
		bool dynCell;
		Vector3r goal;
		int stressMask;
		Vector3r maxStrainRate;
		Real maxUnbalanced;
		Real absStressTol;
		Real relStressTol;
		Real growDamping;
		int globUpdate;
		string doneHook;
		Vector3r maxBodySpan;
		Matrix3r stressTensor;
		Vector3r stress;
		Vector3r strain;
		Vector3r strainRate;
		Vector3r stiff;
		Real currUnbalanced;
		Vector3r prevGrow;
		Real mass;
		Real externalWork;
		int velGradWorkIx;
		
		PeriTriaxController() : dynCell(false), stressMask(0), maxStrainRate(Vector3r(1,1,1)), 
			maxUnbalanced(1e-4), absStressTol(1e3), relStressTol(3e-5), growDamping(.25), 
			globUpdate(5), maxBodySpan(Vector3r::Zero()), stressTensor(Matrix3r::Zero()), 
			stress(Vector3r::Zero()), strain(Vector3r::Zero()), strainRate(Vector3r::Zero()), 
			stiff(Vector3r::Zero()), currUnbalanced(NaN), prevGrow(Vector3r::Zero()), 
			mass(NaN), externalWork(0), velGradWorkIx(-1) {}
		
		virtual void action();
		void strainStressStiffUpdate();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(BoundaryController, dynCell, goal, stressMask, maxStrainRate, maxUnbalanced, 
			absStressTol, relStressTol, growDamping, globUpdate, doneHook, maxBodySpan, stressTensor, 
			stress, strain, strainRate, stiff, currUnbalanced, prevGrow, mass, externalWork, velGradWorkIx);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(PeriTriaxController, BoundaryController);

class Peri3dController: public BoundaryController{
	public:
		Vector6r stressOld;
		Matrix3r sigma;
		Matrix3r epsilon;
		Matrix3r epsilonRate;
		Matrix3r rot;
		Matrix3r nonrot;
		Vector6r stress;
		Vector6r strain;
		Vector6r strainRate;
		Vector6r stressRate;
		Vector6r stressIdeal;
		Vector6r goal;
		int stressMask;
		int nSteps;
		Real progress;
		Real mod;
		string doneHook;
		vector<Vector2r> xxPath;
		vector<Vector2r> yyPath;
		vector<Vector2r> zzPath;
		vector<Vector2r> yzPath;
		vector<Vector2r> zxPath;
		vector<Vector2r> xyPath;
		Real maxStrainRate;
		Real maxStrain;
		Real youngEstimation;
		Real poissonEstimation;
		Vector6r stressGoal;
		Vector6r strainGoal;
		Vector6i pe;
		Vector6i ps;
		Vector6i pathSizes;
		Vector6i pathsCounter;
		int lenPe;
		int lenPs;
		
		Peri3dController() : stress(Vector6r::Zero()), strain(Vector6r::Zero()), strainRate(Vector6r::Zero()),
			stressRate(Vector6r::Zero()), stressIdeal(Vector6r::Zero()), goal(Vector6r::Zero()),
			stressMask(0), nSteps(1000), progress(0.), mod(.1), maxStrainRate(1e3), maxStrain(1e6),
			youngEstimation(1e20), poissonEstimation(.25), stressGoal(Vector6r::Zero()), 
			strainGoal(Vector6r::Zero()), pe(Vector6i::Zero()), ps(Vector6i::Zero()),
			pathSizes(Vector6i::Zero()), pathsCounter(Vector6i::Zero()), lenPe(NaN), lenPs(NaN) {}
		
		virtual void action();
		
		// Serialize only essential configuration and state (exclude runtime-only variables)
		REGISTER_ATTRIBUTES(BoundaryController, stressOld, stress, strain, strainRate, stressRate, stressIdeal,
			goal, stressMask, nSteps, mod, doneHook, maxStrainRate, maxStrain,
			youngEstimation, poissonEstimation, stressGoal, strainGoal);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Peri3dController, BoundaryController);