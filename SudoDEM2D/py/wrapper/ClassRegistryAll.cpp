/*************************************************************************
*  ClassRegistryAll.cpp
*
*  This file defines registration functions that are called by the Python
*  wrapper to ensure all classes are properly registered with ClassRegistry.
*
*  All classes are registered explicitly to ensure proper ordering, avoiding
*  static initialization order issues across translation units.
*
*  Registration functions are organized by category for maintainability.
*************************************************************************/

#include <sudodem/lib/base/PluginMacros.hpp>
#include <sudodem/lib/factory/ClassRegistry.hpp>
#include <pybind11/pybind11.h>

// ============================================================================
// CORE CLASS REGISTRATION
// ============================================================================

// Include core base class headers
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
#include <sudodem/pkg/common/Dispatching.hpp>
#include <sudodem/pkg/common/Collider.hpp>
#include <sudodem/pkg/common/PeriodicEngines.hpp>
#include <sudodem/pkg/common/FieldApplier.hpp>
#include <sudodem/pkg/common/Callbacks.hpp>

// Register core base classes with ClassRegistry
// These are the fundamental classes that all other classes inherit from
extern "C" void initCoreClassRegistry() {
	std::cerr << "DEBUG: initCoreClassRegistry called" << std::endl;
	auto& registry = ClassRegistry::instance();

	// Register core base classes (no base)
	registry.registerClass<Serializable>(__FILE__);

	// Register Engine hierarchy
	registry.registerClassWithBase<Engine, Serializable>(__FILE__);
	registry.registerClassWithBase<GlobalEngine, Engine>(__FILE__);
	registry.registerClassWithBase<PartialEngine, Engine>(__FILE__);
	registry.registerClassWithBase<TimeStepper, Serializable>(__FILE__);

	// Register Dispatcher hierarchy
	registry.registerClassWithBase<Dispatcher, Serializable>(__FILE__);

	// Register Functor hierarchy
	registry.registerClassWithBase<Functor, Serializable>(__FILE__);

	// Register Functor base classes
	registry.registerClassWithBase<BoundFunctor, Functor1D<Shape, void, TYPELIST_4(const shared_ptr<Shape>&, shared_ptr<Bound>&, const Se2r&, const Body*)>>(__FILE__);
	registry.registerClassWithBase<IGeomFunctor, Functor2D<Shape, Shape, bool, TYPELIST_7(const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector2r&, const bool&, const shared_ptr<Interaction>&)>>(__FILE__);
	registry.registerClassWithBase<IPhysFunctor, Functor2D<Material, Material, void, TYPELIST_3(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&)>>(__FILE__);
	registry.registerClassWithBase<LawFunctor, Functor2D<IGeom, IPhys, bool, TYPELIST_3(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*)>>(__FILE__);

	// Register fundamental scene objects
	registry.registerClassWithBase<Body, Serializable>(__FILE__);
	registry.registerClassWithBase<Shape, Serializable>(__FILE__);
	registry.registerClassWithBase<Material, Serializable>(__FILE__);
	registry.registerClassWithBase<IGeom, Serializable>(__FILE__);
	registry.registerClassWithBase<IPhys, Serializable>(__FILE__);
	registry.registerClassWithBase<State, Serializable>(__FILE__);
	registry.registerClassWithBase<Scene, Serializable>(__FILE__);
	registry.registerClassWithBase<Cell, Serializable>(__FILE__);
	registry.registerClassWithBase<Interaction, Serializable>(__FILE__);

	// Register callback classes
#ifdef SUDODEM_BODY_CALLBACKS
	registry.registerClassWithBase<BodyCallback, Serializable>(__FILE__);
	registry.registerClassWithBase<IntrCallback, BodyCallback>(__FILE__);
#else
	registry.registerClassWithBase<IntrCallback, Serializable>(__FILE__);
#endif

	std::cerr << "DEBUG: initCoreClassRegistry completed, ClassRegistry size: " << registry.size() << std::endl;
}

// Function to register Engine base classes with Python
// This must be called before registerAll() to ensure base classes are available
extern "C" void register_engine_base_classes(pybind11::module_ m) {
	// Register Engine base classes that are not manually registered in sudodemWrapper.cpp
	// These are needed for derived classes to register correctly

	try {
		pybind11::class_<GlobalEngine, Engine, std::shared_ptr<GlobalEngine>>(m, "GlobalEngine", "Global engine that operates on the whole scene")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}

	try {
		pybind11::class_<PartialEngine, Engine, std::shared_ptr<PartialEngine>>(m, "PartialEngine", "Partial engine that operates on a subset of bodies")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}

	try {
		pybind11::class_<TimeStepper, GlobalEngine, std::shared_ptr<TimeStepper>>(m, "TimeStepper", "Engine defining time-step")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}

	try {
		pybind11::class_<Collider, GlobalEngine, std::shared_ptr<Collider>>(m, "Collider", "Collider")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}

	try {
		pybind11::class_<PeriodicEngine, GlobalEngine, std::shared_ptr<PeriodicEngine>>(m, "PeriodicEngine", "PeriodicEngine")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}

	try {
		pybind11::class_<FieldApplier, GlobalEngine, std::shared_ptr<FieldApplier>>(m, "FieldApplier", "FieldApplier")
			.def(pybind11::init<>());
	} catch (...) {
		// Already registered, ignore
	}
}

// ============================================================================
// ENGINE CLASSES REGISTRATION
// ============================================================================

#include <sudodem/pkg/common/ForceResetter.hpp>
#include <sudodem/pkg/dem/NewtonIntegrator.hpp>
#include <sudodem/pkg/common/InsertionSortCollider.hpp>
#include <sudodem/pkg/common/InteractionLoop.hpp>
#include <sudodem/pkg/common/PeriodicEngines.hpp>
#include <sudodem/pkg/common/PyRunner.hpp>
#include <sudodem/pkg/common/Recorder.hpp>
#include <sudodem/pkg/common/GravityEngines.hpp>
#include <sudodem/pkg/common/ParallelEngine.hpp>
#include <sudodem/pkg/common/MatchMaker.hpp>
#include <sudodem/pkg/dem/ElasticContactLaw.hpp>
#include <sudodem/pkg/common/BoundaryController.hpp>
#include <sudodem/pkg/dem/PeriTriaxial.hpp>
#include <sudodem/pkg/common/Collider.hpp>
#include <sudodem/pkg/common/FieldApplier.hpp>
#include <sudodem/core/TimeStepper.hpp>
#include <sudodem/pkg/common/StepDisplacer.hpp>
#include <sudodem/pkg/common/TorqueEngine.hpp>
#include <sudodem/pkg/common/ForceEngine.hpp>

extern "C" void register_engine_classes() {
	auto& registry = ClassRegistry::instance();

	// GlobalEngine classes
	registry.registerClassWithBase<ForceResetter, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<NewtonIntegrator, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<InsertionSortCollider, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<InteractionLoop, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<PeriodicEngine, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<PyRunner, PeriodicEngine>(__FILE__);
	registry.registerClassWithBase<Recorder, PeriodicEngine>(__FILE__);
	registry.registerClassWithBase<GravityEngine, FieldApplier>(__FILE__);
	registry.registerClassWithBase<CentralGravityEngine, FieldApplier>(__FILE__);
	registry.registerClassWithBase<AxialGravityEngine, FieldApplier>(__FILE__);
	registry.registerClassWithBase<ParallelEngine, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<MatchMaker, Serializable>(__FILE__);
	registry.registerClassWithBase<ElasticContactLaw, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<BoundaryController, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<PeriTriaxController, BoundaryController>(__FILE__);
	registry.registerClassWithBase<Collider, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<FieldApplier, GlobalEngine>(__FILE__);
	registry.registerClassWithBase<TimeStepper, GlobalEngine>(__FILE__);

	// PartialEngine classes
	registry.registerClassWithBase<StepDisplacer, PartialEngine>(__FILE__);
	registry.registerClassWithBase<TorqueEngine, PartialEngine>(__FILE__);
	registry.registerClassWithBase<ForceEngine, PartialEngine>(__FILE__);
	registry.registerClassWithBase<DragEngine, PartialEngine>(__FILE__);
	registry.registerClassWithBase<LinearDragEngine, PartialEngine>(__FILE__);
	registry.registerClassWithBase<HydroForceEngine, PartialEngine>(__FILE__);
}

// ============================================================================
// SHAPE AND MATERIAL CLASS REGISTRATION
// ============================================================================

#include <sudodem/pkg/common/Wall.hpp>
#include <sudodem/pkg/common/Disk.hpp>
#include <sudodem/pkg/common/ElastMat.hpp>
#include <sudodem/pkg/common/Aabb.hpp>

extern "C" void register_shape_material_classes() {
	auto& registry = ClassRegistry::instance();

	// Shape classes
	registry.registerClassWithBase<Fwall, Wall>(__FILE__);
	registry.registerClassWithBase<Disk, Shape>(__FILE__);

	// Material classes
	registry.registerClassWithBase<ElastMat, Material>(__FILE__);

	// Bo1 functors (BoundFunctor for creating/updating bounding boxes)
	registry.registerClassWithBase<Bo1_Wall_Aabb, BoundFunctor>(__FILE__);
	registry.registerClassWithBase<Bo1_Fwall_Aabb, BoundFunctor>(__FILE__);
}

// ============================================================================
// Ig2, Ip2, Law2, and Bo1 CLASSES REGISTRATION
// ============================================================================

#include <sudodem/pkg/dem/Ig2_Basic_ScGeom.hpp>
#include <sudodem/pkg/dem/ScGeom.hpp>
#include <sudodem/pkg/common/NormShearPhys.hpp>
#include <sudodem/pkg/dem/FrictPhys.hpp>
#include <sudodem/pkg/dem/ElasticContactLaw.hpp>

extern "C" void register_common_functors() {
	auto& registry = ClassRegistry::instance();

	// Ig2 functors - Geometry creation for basic shapes
	registry.registerClassWithBase<Ig2_Disk_Disk_ScGeom, IGeomFunctor>(__FILE__);
	registry.registerClassWithBase<Ig2_Wall_Disk_ScGeom, IGeomFunctor>(__FILE__);
	registry.registerClassWithBase<Ig2_Fwall_Disk_ScGeom, IGeomFunctor>(__FILE__);

	// Ip2 functors - Physics creation
	registry.registerClassWithBase<Ip2_FrictMat_FrictMat_FrictPhys, IPhysFunctor>(__FILE__);
	registry.registerClassWithBase<Ip2_FrictMat_FrictMat_ViscoFrictPhys, IPhysFunctor>(__FILE__);

	// LawFunctor functors - Contact laws
	registry.registerClassWithBase<Law2_ScGeom_FrictPhys_CundallStrack, LawFunctor>(__FILE__);
	registry.registerClassWithBase<Law2_ScGeom_ViscoFrictPhys_CundallStrack, LawFunctor>(__FILE__);
}

extern "C" void register_geom_phys_classes() {
	auto& registry = ClassRegistry::instance();

	// IGeom classes
	registry.registerClassWithBase<ScGeom, IGeom>(__FILE__);

	// IPhys classes - inheritance hierarchy: IPhys -> NormPhys -> NormShearPhys -> FrictPhys -> {ViscoFrictPhys}
	registry.registerClassWithBase<NormPhys, IPhys>(__FILE__);
	registry.registerClassWithBase<NormShearPhys, NormPhys>(__FILE__);
	registry.registerClassWithBase<FrictPhys, NormShearPhys>(__FILE__);
	registry.registerClassWithBase<ViscoFrictPhys, FrictPhys>(__FILE__);
}

// ============================================================================
// SUPERELLIPSE CLASSES REGISTRATION
// ============================================================================

#include <sudodem/pkg/dem/Superellipse.hpp>
#include <sudodem/pkg/dem/Superellipse_Ig2.hpp>

extern "C" void register_superellipse_classes() {
	static bool registered = false;
	if (registered) return;
	registered = true;

	auto& registry = ClassRegistry::instance();

	// Superellipse shape and material
	registry.registerClassWithBase<Superellipse, Shape>(__FILE__);
	registry.registerClassWithBase<SuperellipseGeom, IGeom>(__FILE__);
	registry.registerClassWithBase<Bo1_Superellipse_Aabb, BoundFunctor>(__FILE__);
	registry.registerClassWithBase<SuperellipseMat, Material>(__FILE__);
	registry.registerClassWithBase<SuperellipseMat2, SuperellipseMat>(__FILE__);
	registry.registerClassWithBase<SuperellipsePhys, FrictPhys>(__FILE__);
	registry.registerClassWithBase<Ip2_SuperellipseMat_SuperellipseMat_SuperellipsePhys, IPhysFunctor>(__FILE__);
	registry.registerClassWithBase<SuperellipseLaw, LawFunctor>(__FILE__);

	// Register Ig2 functors for geometry creation
	registry.registerClassWithBase<Ig2_Wall_Superellipse_SuperellipseGeom, IGeomFunctor>(__FILE__);
	registry.registerClassWithBase<Ig2_Superellipse_Superellipse_SuperellipseGeom, IGeomFunctor>(__FILE__);
	registry.registerClassWithBase<Ig2_Fwall_Superellipse_SuperellipseGeom, IGeomFunctor>(__FILE__);

#ifdef SUDODEM_OPENGL
	registry.registerClassWithBase<Gl1_Superellipse, GlShapeFunctor>(__FILE__);
	registry.registerClassWithBase<Gl1_SuperellipseGeom, GlIGeomFunctor>(__FILE__);
#endif
}

// ============================================================================
// CORE EXTRA CLASSES REGISTRATION
// ============================================================================

extern "C" void register_core_extra_classes() {
	std::cerr << "DEBUG: register_core_extra_classes called" << std::endl;
	auto& registry = ClassRegistry::instance();

	// Call all the registration functions to ensure all classes are registered
	register_shape_material_classes();
	register_engine_classes();
	register_common_functors();
	register_geom_phys_classes();
	register_superellipse_classes();

	std::cerr << "DEBUG: register_core_extra_classes done, ClassRegistry size: " << registry.size() << std::endl;
}

// ============================================================================
// POLYHEDRON CLASSES REGISTRATION (placeholder for future use)
// ============================================================================

extern "C" void register_polyhedron_classes() {
	std::cerr << "DEBUG: register_polyhedron_classes called" << std::endl;
	// Polyhedron classes are not available in 2D version
	// This function exists for backward compatibility
}
