// Kovthanan …
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/pkg/common/Collider.hpp>


class SpheresFactory: public GlobalEngine {
	shared_ptr<Collider> collider;
	protected:
		virtual void pickRandomPosition(Vector3r&, Real);
		vector<Real> PSDCurMean;
		vector<Real> PSDCurProc;
		vector<Real> PSDNeedProc;
		bool PSDuse;
	public:
		Real massFlowRate;
		Real rMin;
		Real rMax;
		Real vMin;
		Real vMax;
		Real vAngle;
		Vector3r normal;
		Vector3r normalVel;
		int materialId;
		int mask;
		Vector3r color;
		vector<int> ids;
		Real totalMass;
		Real totalVolume;
		Real goalMass;
		int maxParticles;
		Real maxMass;
		int numParticles;
		int maxAttempt;
		bool silent;
		std::string blockedDOFs;
		vector<Real> PSDsizes;
		vector<Real> PSDcum;
		bool PSDcalculateMass;
		bool stopIfFailed;
		bool exactDiam;
		
		SpheresFactory() : massFlowRate(NaN), rMin(NaN), rMax(NaN), vMin(NaN), vMax(NaN), 
			vAngle(NaN), normal(Vector3r(NaN,NaN,NaN)), normalVel(Vector3r(NaN,NaN,NaN)), 
			materialId(-1), mask(-1), color(Vector3r(-1,-1,-1)), totalMass(0), totalVolume(0), 
			goalMass(0), maxParticles(100), maxMass(-1), numParticles(0), maxAttempt(5000), 
			silent(false), PSDcalculateMass(true), stopIfFailed(true), exactDiam(true), PSDuse(false) {}
		
		virtual void action();
		
		struct SpherCoord{
			Vector3r c; Real r;
			SpherCoord(const Vector3r& _c, Real _r){ c=_c; r=_r;}
		};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlobalEngine, massFlowRate, rMin, rMax, vMin, vMax, vAngle, normal, normalVel,
			materialId, mask, color, ids, totalMass, totalVolume, goalMass, maxParticles, maxMass,
			numParticles, maxAttempt, silent, blockedDOFs, PSDsizes, PSDcum, PSDcalculateMass, 
			stopIfFailed, exactDiam);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(SpheresFactory, GlobalEngine);

class CircularFactory: public SpheresFactory {
	protected:
		virtual void pickRandomPosition(Vector3r&, Real);
	public:
		Real radius;
		Real length;
		Vector3r center;
		
		CircularFactory() : radius(NaN), length(0), center(Vector3r(NaN,NaN,NaN)) {}
		
		virtual ~CircularFactory(){};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(SpheresFactory, radius, length, center);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(CircularFactory, SpheresFactory);

class BoxFactory: public SpheresFactory {
	protected:
		virtual void pickRandomPosition(Vector3r&, Real);
	public:
		Vector3r extents;
		Vector3r center;
		
		BoxFactory() : extents(Vector3r(NaN,NaN,NaN)), center(Vector3r(NaN,NaN,NaN)) {}
		
		virtual ~BoxFactory(){};
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(SpheresFactory, extents, center);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(BoxFactory, SpheresFactory);