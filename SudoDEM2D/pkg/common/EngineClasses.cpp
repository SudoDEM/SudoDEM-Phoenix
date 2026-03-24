/*************************************************************************
*  Engine Classes Registration for ClassRegistry
*  This file registers all Engine-derived classes with the ClassRegistry
*  for automatic pybind11 registration
*************************************************************************/

#include <sudodem/lib/factory/ClassRegistry.hpp>

// GlobalEngine headers
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

// PartialEngine headers
#include <sudodem/pkg/common/StepDisplacer.hpp>
#include <sudodem/pkg/common/TorqueEngine.hpp>
#include <sudodem/pkg/common/ForceEngine.hpp>

// Explicit registration function for Engine classes
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