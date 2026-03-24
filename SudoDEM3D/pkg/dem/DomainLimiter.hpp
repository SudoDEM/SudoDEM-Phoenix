#include<sudodem/pkg/common/PeriodicEngines.hpp>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/pkg/common/Sphere.hpp>

class DomainLimiter: public PeriodicEngine{
	public:
		Vector3r lo;
		Vector3r hi;
		long nDeleted;
		Real mDeleted;
		Real vDeleted;
		int mask;
		
		virtual void action();
		
		// Explicit constructor with initial values
		DomainLimiter(): lo(Vector3r(0,0,0)), hi(Vector3r(0,0,0)), nDeleted(0), mDeleted(0.), vDeleted(0.), mask(-1) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(PeriodicEngine, lo, hi, nDeleted, mDeleted, vDeleted, mask);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(DomainLimiter, PeriodicEngine);

class LawTester: public PartialEngine{
	Body::id_t id1,id2;
	public:
		vector<Vector3r> disPath;
		vector<Vector3r> rotPath;
		vector<string> hooks;
		Vector6r uGeom;
		Vector6r uTest;
		Vector6r uTestNext;
		bool warnedDeprecPtRot;
		Vector3r shearTot;
		bool displIsRel;
		vector<int> pathSteps;
		vector<int> _pathT;
		vector<Vector6r> _path;
		shared_ptr<Interaction> I;
		Vector3r axX;
		Vector3r axY;
		Vector3r axZ;
		Matrix3r trsf;
		size_t _interpPos;
		Vector6r uuPrev;
		int step;
		string doneHook;
		Real renderLength;
		Real refLength;
		Vector3r contPt;
		Real idWeight;
		Real rotWeight;
		
		void init();
		virtual void action();
		void postLoad(LawTester&);
		void warnDeprec(const string& s1, const string& s2);
		Vector3r get_ptOurs();
		Vector3r get_ptGeom();
		Vector3r get_rotOurs();
		Vector3r get_rotGeom();
		
		// Explicit constructor with initial values
		LawTester(): uGeom(Vector6r::Zero()), uTest(Vector6r::Zero()), uTestNext(Vector6r::Zero()), warnedDeprecPtRot(false), shearTot(Vector3r::Zero()), displIsRel(true), _interpPos(0), uuPrev(Vector6r::Zero()), step(1), renderLength(0.), refLength(0.), contPt(Vector3r::Zero()), idWeight(1.), rotWeight(1.) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(PartialEngine, disPath, rotPath, hooks, uGeom, uTest, uTestNext, warnedDeprecPtRot, shearTot, displIsRel, pathSteps, _pathT, _path, axX, axY, axZ, trsf, _interpPos, uuPrev, step, doneHook, renderLength, refLength, contPt, idWeight, rotWeight);
		
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(LawTester, PartialEngine);

#ifdef SUDODEM_OPENGL
#include<sudodem/pkg/common/OpenGLRenderer.hpp>

class GlExtra_LawTester: public GlExtraDrawer{
	public:
		shared_ptr<LawTester> tester;
		
		DECLARE_LOGGER;
		virtual void render();
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlExtraDrawer, tester);
};
REGISTER_SERIALIZABLE_BASE(GlExtra_LawTester, GlExtraDrawer);

class GlExtra_OctreeCubes: public GlExtraDrawer{
	public:
		struct OctreeBox{ Vector3r center, extents; int fill; int level; };
		std::vector<OctreeBox> boxes;
		string boxesFile;
		Vector2i fillRangeFill;
		Vector2i fillRangeDraw;
		Vector2i levelRangeDraw;
		bool noFillZero;
		
		void postLoad(GlExtra_OctreeCubes&);
		virtual void render();
		
		// Explicit constructor with initial values
		GlExtra_OctreeCubes(): fillRangeFill(Vector2i(2,2)), fillRangeDraw(Vector2i(-2,2)), levelRangeDraw(Vector2i(-2,2)), noFillZero(true) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlExtraDrawer, boxesFile, fillRangeFill, fillRangeDraw, levelRangeDraw, noFillZero);
};
REGISTER_SERIALIZABLE_BASE(GlExtra_OctreeCubes, GlExtraDrawer);
#endif