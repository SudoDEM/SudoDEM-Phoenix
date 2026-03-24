/*************************************************************************
*  ClassRegistryAll.cpp
*
*  This file defines registration functions that directly call pyRegisterClass
*  for each class using std::make_shared<Class>()->pyRegisterClass(m).
*
*  This replaces the old ClassRegistry::registerClassWithBase approach with
*  direct pybind11 registration calls organized in correct order.
*
*  Registration order is critical: base classes must be registered before
*  derived classes for pybind11 inheritance to work correctly.
*************************************************************************/

#include <sudodem/lib/base/PluginMacros.hpp>
#include <sudodem/lib/factory/ClassRegistry.hpp>
#include <pybind11/pybind11.h>

// ============================================================================
// MACRO: Register class for both pybind11 and factory
// ============================================================================
#define REGISTER_CLASS_BOTH(m, T) \
    do { \
        try { std::make_shared<T>()->pyRegisterClass(m); } catch (...) {} \
        ClassRegistry::instance().registerClass<T>(#T); \
        ClassRegistry::instance().setReadableClassName(typeid(T).name(), #T); \
    } while(0)

// ============================================================================
// MACRO: Register class with explicit base class info (for inheritance tracking)
// ============================================================================
#define REGISTER_CLASS_WITH_BASE(m, T, BaseT) \
    do { \
        try { std::make_shared<T>()->pyRegisterClass(m); } catch (...) {} \
        ClassRegistry::instance().registerClass<T>(#T); \
        ClassRegistry::instance().setReadableClassName(typeid(T).name(), #T); \
        ClassRegistry::instance().setBaseClassName(typeid(T).name(), typeid(BaseT).name()); \
    } while(0)

// ============================================================================
// CORE BASE CLASS HEADERS
// ============================================================================
#include <sudodem/lib/serialization/Serializable.hpp>
#include <sudodem/core/Engine.hpp>
#include <sudodem/core/GlobalEngine.hpp>
#include <sudodem/core/PartialEngine.hpp>
#include <sudodem/core/Functor.hpp>
#include <sudodem/core/Dispatcher.hpp>
#include <sudodem/core/Body.hpp>
#include <sudodem/core/Shape.hpp>
#include <sudodem/core/Material.hpp>
#include <sudodem/core/IGeom.hpp>
#include <sudodem/core/IPhys.hpp>
#include <sudodem/core/State.hpp>
#include <sudodem/core/Scene.hpp>
#include <sudodem/core/Cell.hpp>
#include <sudodem/core/Interaction.hpp>
#include <sudodem/core/Bound.hpp>
#include <sudodem/core/TimeStepper.hpp>
#include <sudodem/core/Clump.hpp>

// ============================================================================
// COMMON CLASS HEADERS
// ============================================================================
#include <sudodem/pkg/common/Dispatching.hpp>
#include <sudodem/pkg/common/Collider.hpp>
#include <sudodem/pkg/common/PeriodicEngines.hpp>
#include <sudodem/pkg/common/InteractionLoop.hpp>
#include <sudodem/pkg/common/Recorder.hpp>
#include <sudodem/pkg/common/FieldApplier.hpp>
#include <sudodem/pkg/common/BoundaryController.hpp>
#include <sudodem/pkg/common/Sphere.hpp>
#include <sudodem/pkg/common/Wall.hpp>
#include <sudodem/pkg/common/Box.hpp>
#include <sudodem/pkg/common/Facet.hpp>
#include <sudodem/pkg/common/GLDrawFunctors.hpp>
#include <sudodem/pkg/common/Facet.hpp>
#include <sudodem/pkg/common/Wall.hpp>
#include <sudodem/pkg/common/ElastMat.hpp>
#include <sudodem/pkg/common/NormShearPhys.hpp>
#include <sudodem/pkg/common/ForceResetter.hpp>
#include <sudodem/pkg/common/PyRunner.hpp>
#include <sudodem/pkg/common/GravityEngines.hpp>
#include <sudodem/pkg/common/ForceEngine.hpp>
#include <sudodem/pkg/common/ParallelEngine.hpp>
#include <sudodem/pkg/common/MatchMaker.hpp>
#include <sudodem/pkg/common/Aabb.hpp>
#include <sudodem/pkg/common/InsertionSortCollider.hpp>
#include <sudodem/pkg/common/TorqueEngine.hpp>
#include <sudodem/pkg/common/KinematicEngines.hpp>
#include <sudodem/pkg/common/StepDisplacer.hpp>
#include <sudodem/pkg/common/Callbacks.hpp>

// ============================================================================
// DEM CLASS HEADERS
// ============================================================================
#include <sudodem/pkg/dem/DemXDofGeom.hpp>
#include <sudodem/pkg/dem/ScGeom.hpp>
#include <sudodem/pkg/dem/FrictPhys.hpp>
#include <sudodem/pkg/dem/Ig2_Basic_ScGeom.hpp>
#include <sudodem/pkg/dem/Ip2_ElastMat.hpp>
#include <sudodem/pkg/dem/ElasticContactLaw.hpp>
#include <sudodem/pkg/dem/RollingResistanceLaw.hpp>
#include <sudodem/pkg/dem/NewtonIntegrator.hpp>
#include <sudodem/pkg/dem/Integrator.hpp>
#include <sudodem/pkg/dem/GeneralIntegratorInsertionSortCollider.hpp>
#include <sudodem/pkg/dem/GlobalStiffnessTimeStepper.hpp>
#include <sudodem/pkg/dem/FlatGridCollider.hpp>
#include <sudodem/pkg/dem/PeriIsoCompressor.hpp>
#include <sudodem/pkg/dem/PolyCompression.hpp>
#include <sudodem/pkg/dem/SpheresFactory.hpp>
#include <sudodem/pkg/dem/Expander.hpp>
#include <sudodem/pkg/dem/CubicCompressionEngine.hpp>
#include <sudodem/pkg/dem/CylindricalCompressionEngine.hpp>
#include <sudodem/pkg/dem/Disp2DPropLoadEngine.hpp>
#include <sudodem/pkg/dem/FacetTopologyAnalyzer.hpp>
#include <sudodem/pkg/dem/Superquadrics.hpp>
#include <sudodem/pkg/dem/Superquadrics_Ig2.hpp>


// ============================================================================
// INIT CORE CLASS REGISTRY
// Register base classes with ClassRegistry for factory functionality
// ============================================================================
extern "C" void initCoreClassRegistry() {
	std::cerr << "DEBUG: initCoreClassRegistry called" << std::endl;
	auto& registry = ClassRegistry::instance();

	// Register core base classes (no base)
	registry.registerClass<Serializable>(__FILE__);
	registry.registerClass<Engine>(__FILE__);
	registry.registerClass<GlobalEngine>(__FILE__);
	registry.registerClass<PartialEngine>(__FILE__);
	registry.registerClass<TimeStepper>(__FILE__);
	registry.registerClass<PeriodicEngine>(__FILE__);
	registry.registerClass<Collider>(__FILE__);
	registry.registerClass<FieldApplier>(__FILE__);
	registry.registerClass<BoundaryController>(__FILE__);
	registry.registerClass<Dispatcher>(__FILE__);
	registry.registerClass<Functor>(__FILE__);
	registry.registerClass<BoundFunctor>(__FILE__);
	registry.registerClass<IGeomFunctor>(__FILE__);
	registry.registerClass<IPhysFunctor>(__FILE__);
	registry.registerClass<LawFunctor>(__FILE__);
	registry.registerClass<Body>(__FILE__);
	registry.registerClass<Shape>(__FILE__);
	registry.registerClass<Bound>(__FILE__);
	registry.registerClass<Material>(__FILE__);
	registry.registerClass<IGeom>(__FILE__);
	registry.registerClass<IPhys>(__FILE__);
	registry.registerClass<State>(__FILE__);
	registry.registerClass<Scene>(__FILE__);
	registry.registerClass<Cell>(__FILE__);
	registry.registerClass<Interaction>(__FILE__);
	registry.registerClass<Clump>(__FILE__);

	std::cerr << "DEBUG: initCoreClassRegistry completed, ClassRegistry size: " << registry.size() << std::endl;
}

// ============================================================================
// REGISTER ALL CLASSES DIRECTLY
// This function calls pyRegisterClass directly for each class in correct order
// This replaces ClassRegistry::registerAll
// ============================================================================
extern "C" void registerAllDirectly(pybind11::module_& m) {
	std::cerr << "DEBUG: registerAllDirectly starting..." << std::endl;

	// =========================================================================
	// STEP 1: CORE BASE CLASSES (most fundamental - must be first)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Serializable);
	REGISTER_CLASS_BOTH(m, Body);
	REGISTER_CLASS_BOTH(m, Shape);
	REGISTER_CLASS_BOTH(m, Bound);
	REGISTER_CLASS_BOTH(m, Material);
	REGISTER_CLASS_BOTH(m, IGeom);
	REGISTER_CLASS_BOTH(m, IPhys);
	REGISTER_CLASS_BOTH(m, State);
	REGISTER_CLASS_BOTH(m, Scene);
	REGISTER_CLASS_BOTH(m, Cell);
	REGISTER_CLASS_BOTH(m, Interaction);
	REGISTER_CLASS_BOTH(m, Clump);

	// =========================================================================
	// STEP 2: ENGINE BASE CLASSES (must be before NewtonIntegrator)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Engine);
	REGISTER_CLASS_BOTH(m, GlobalEngine);
	REGISTER_CLASS_BOTH(m, PartialEngine);
	REGISTER_CLASS_BOTH(m, TimeStepper);
	REGISTER_CLASS_BOTH(m, PeriodicEngine);
	REGISTER_CLASS_BOTH(m, Collider);
	REGISTER_CLASS_BOTH(m, FieldApplier);
	REGISTER_CLASS_BOTH(m, BoundaryController);
	REGISTER_CLASS_BOTH(m, Dispatcher);
	REGISTER_CLASS_BOTH(m, Functor);
	REGISTER_CLASS_BOTH(m, BoundFunctor);
	REGISTER_CLASS_BOTH(m, IGeomFunctor);
	REGISTER_CLASS_BOTH(m, IPhysFunctor);
	REGISTER_CLASS_BOTH(m, LawFunctor);

	// =========================================================================
	// STEP 3: DISPATCHERS
	// =========================================================================
	REGISTER_CLASS_BOTH(m, BoundDispatcher);
	REGISTER_CLASS_BOTH(m, IGeomDispatcher);
	REGISTER_CLASS_BOTH(m, IPhysDispatcher);
	REGISTER_CLASS_BOTH(m, LawDispatcher);

	// =========================================================================
	// STEP 4: MATERIAL CLASSES (base first, then derived)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, ElastMat);
	REGISTER_CLASS_BOTH(m, FrictMat);
	REGISTER_CLASS_BOTH(m, NormPhys);
	REGISTER_CLASS_BOTH(m, NormShearPhys);
	REGISTER_CLASS_BOTH(m, FrictPhys);
	REGISTER_CLASS_BOTH(m, SuperquadricsMat);
	REGISTER_CLASS_BOTH(m, SuperquadricsMat2);
	REGISTER_CLASS_BOTH(m, SuperquadricsPhys);
	REGISTER_CLASS_BOTH(m, SuperquadricsHertzMindlinPhys);

	// =========================================================================
	// STEP 5: SHAPE CLASSES (base first, then derived)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Sphere);
	REGISTER_CLASS_BOTH(m, Wall);
	REGISTER_CLASS_BOTH(m, Fwall);
	REGISTER_CLASS_BOTH(m, Box);
	REGISTER_CLASS_BOTH(m, Facet);
	REGISTER_CLASS_BOTH(m, Superquadrics);

	// =========================================================================
	// STEP 5: GEOMETRY CLASSES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, GenericSpheresContact);
	REGISTER_CLASS_BOTH(m, ScGeom);
	REGISTER_CLASS_BOTH(m, ScGeom6D);
	REGISTER_CLASS_BOTH(m, SuperquadricsGeom);
	REGISTER_CLASS_BOTH(m, SuperquadricsGeom2);

	// =========================================================================
	// STEP 6: Ig2 FUNCTORS (Interaction Geometry)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Ig2_Sphere_Sphere_ScGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Sphere_Sphere_ScGeom6D);
	REGISTER_CLASS_BOTH(m, Ig2_Wall_Sphere_ScGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Box_Sphere_ScGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Box_Sphere_ScGeom6D);
	REGISTER_CLASS_BOTH(m, Ig2_Facet_Sphere_ScGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Facet_Sphere_ScGeom6D);
	REGISTER_CLASS_BOTH(m, Ig2_Superquadrics_Superquadrics_SuperquadricsGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Superquadrics_Superquadrics_SuperquadricsGeom2);
	REGISTER_CLASS_BOTH(m, Ig2_Wall_Superquadrics_SuperquadricsGeom);
	REGISTER_CLASS_BOTH(m, Ig2_Wall_Superquadrics_SuperquadricsGeom2);
	REGISTER_CLASS_BOTH(m, Ig2_Facet_Superquadrics_SuperquadricsGeom);

	// =========================================================================
	// STEP 7: Ip2 FUNCTORS (Interaction Physics)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Ip2_ElastMat_ElastMat_NormPhys);
	REGISTER_CLASS_BOTH(m, Ip2_ElastMat_ElastMat_NormShearPhys);
	REGISTER_CLASS_BOTH(m, Ip2_FrictMat_FrictMat_FrictPhys);
	REGISTER_CLASS_BOTH(m, Ip2_SuperquadricsMat_SuperquadricsMat_SuperquadricsPhys);
	REGISTER_CLASS_BOTH(m, Ip2_SuperquadricsMat2_SuperquadricsMat2_HertzMindlinPhys);

	// =========================================================================
	// STEP 8: BOUND FUNCTORS (Bo1)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Bo1_Sphere_Aabb);
	REGISTER_CLASS_BOTH(m, Bo1_Box_Aabb);
	REGISTER_CLASS_BOTH(m, Bo1_Wall_Aabb);
	REGISTER_CLASS_BOTH(m, Bo1_Facet_Aabb);
	REGISTER_CLASS_BOTH(m, Bo1_Superquadrics_Aabb);

	// =========================================================================
	// STEP 9: LAW FUNCTORS (Contact Laws)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Law2_ScGeom_FrictPhys_CundallStrack);
	REGISTER_CLASS_BOTH(m, ElasticContactLaw);
	REGISTER_CLASS_BOTH(m, SuperquadricsLaw);
	REGISTER_CLASS_BOTH(m, SuperquadricsLaw2);

	// =========================================================================
	// STEP 10: ENGINE CLASSES (base first, then derived)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, Integrator);
	REGISTER_CLASS_BOTH(m, NewtonIntegrator);
	REGISTER_CLASS_BOTH(m, InsertionSortCollider);
	REGISTER_CLASS_BOTH(m, GeneralIntegratorInsertionSortCollider);
	REGISTER_CLASS_BOTH(m, FlatGridCollider);
	REGISTER_CLASS_BOTH(m, GlobalStiffnessTimeStepper);
	REGISTER_CLASS_BOTH(m, ForceResetter);
	REGISTER_CLASS_BOTH(m, InteractionLoop);
	REGISTER_CLASS_BOTH(m, PyRunner);
	REGISTER_CLASS_BOTH(m, ParallelEngine);
	REGISTER_CLASS_BOTH(m, Recorder);
	REGISTER_CLASS_BOTH(m, StepDisplacer);
	REGISTER_CLASS_BOTH(m, TorqueEngine);

	// =========================================================================
	// STEP 11: GRAVITY ENGINES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, GravityEngine);
	REGISTER_CLASS_BOTH(m, CentralGravityEngine);
	REGISTER_CLASS_BOTH(m, AxialGravityEngine);

	// =========================================================================
	// STEP 12: FORCE ENGINES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, ForceEngine);
	REGISTER_CLASS_BOTH(m, InterpolatingDirectedForceEngine);
	REGISTER_CLASS_BOTH(m, RadialForceEngine);
	REGISTER_CLASS_BOTH(m, DragEngine);
	REGISTER_CLASS_BOTH(m, LinearDragEngine);
	REGISTER_CLASS_BOTH(m, HydroForceEngine);

	// =========================================================================
	// STEP 13: KINEMATIC ENGINES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, KinematicEngine);
	REGISTER_CLASS_BOTH(m, CombinedKinematicEngine);
	REGISTER_CLASS_BOTH(m, TranslationEngine);
	REGISTER_CLASS_BOTH(m, RotationEngine);
	REGISTER_CLASS_BOTH(m, HarmonicMotionEngine);
	REGISTER_CLASS_BOTH(m, ServoPIDController);
	REGISTER_CLASS_BOTH(m, BicyclePedalEngine);

	// =========================================================================
	// STEP 14: OTHER CLASSES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, MatchMaker);
	REGISTER_CLASS_BOTH(m, Aabb);
	REGISTER_CLASS_BOTH(m, Gl1_Aabb);
	REGISTER_CLASS_BOTH(m, IntrCallback);

	// =========================================================================
	// STEP 15: COMPRESSION ENGINES (PolyCompressionEngine is only available with SUDODEM_CGAL)
	// =========================================================================
	REGISTER_CLASS_BOTH(m, PeriIsoCompressor);
	REGISTER_CLASS_BOTH(m, PeriTriaxController);
	REGISTER_CLASS_BOTH(m, Peri3dController);
	REGISTER_CLASS_BOTH(m, CubicCompressionEngine);
	REGISTER_CLASS_BOTH(m, CylindricalCompressionEngine);
	REGISTER_CLASS_BOTH(m, Disp2DPropLoadEngine);
	REGISTER_CLASS_BOTH(m, Expander);
	REGISTER_CLASS_BOTH(m, FacetTopologyAnalyzer);

	// =========================================================================
	// STEP 16: FACTORIES
	// =========================================================================
	REGISTER_CLASS_BOTH(m, SpheresFactory);
	REGISTER_CLASS_BOTH(m, CircularFactory);
	REGISTER_CLASS_BOTH(m, BoxFactory);

	// =========================================================================
	// STEP 17: ROLLING RESISTANCE LAW
	// =========================================================================
	REGISTER_CLASS_BOTH(m, RolFrictMat);
	REGISTER_CLASS_BOTH(m, RolFrictPhys);
	REGISTER_CLASS_BOTH(m, Ip2_RolFrictMat_RolFrictMat_RolFrictPhys);
	REGISTER_CLASS_BOTH(m, RollingResistanceLaw);

	// =========================================================================
	// STEP 18: GL DRAWING FUNCTORS (3D OpenGL rendering)
	// =========================================================================
	std::cerr << "DEBUG: Registering GL1_Sphere..." << std::endl;
	try {
		// Register GL shape functors with base class info for OpenGLRenderer detection
		REGISTER_CLASS_WITH_BASE(m, Gl1_Sphere, GlShapeFunctor);
		REGISTER_CLASS_WITH_BASE(m, Gl1_Box, GlShapeFunctor);
		REGISTER_CLASS_WITH_BASE(m, Gl1_Wall, GlShapeFunctor);
		REGISTER_CLASS_WITH_BASE(m, Gl1_Fwall, GlShapeFunctor);
		REGISTER_CLASS_WITH_BASE(m, Gl1_Facet, GlShapeFunctor);
		REGISTER_CLASS_WITH_BASE(m, Gl1_Superquadrics, GlShapeFunctor);
		REGISTER_CLASS_BOTH(m, Bo1_Sphere_Aabb);
		REGISTER_CLASS_BOTH(m, Bo1_Box_Aabb);
		REGISTER_CLASS_BOTH(m, Bo1_Wall_Aabb);
		REGISTER_CLASS_BOTH(m, Bo1_Fwall_Aabb);
		REGISTER_CLASS_BOTH(m, Bo1_Facet_Aabb);
		std::cerr << "DEBUG: GL shape functors registered successfully" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "ERROR registering GL shape functors: " << e.what() << std::endl;
	}

	std::cerr << "DEBUG: registerAllDirectly completed!" << std::endl;
}

// ============================================================================
// LEGACY FUNCTIONS (kept for compatibility)
// ============================================================================
