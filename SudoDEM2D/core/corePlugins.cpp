// ClassFactory removed - using ClassRegistry for class registration
// make core classes known to the class factory
#include<sudodem/core/Body.hpp>
#include<sudodem/core/BodyContainer.hpp>
#include<sudodem/core/Bound.hpp>
#include<sudodem/core/Cell.hpp>
#include<sudodem/core/Dispatcher.hpp>
#include<sudodem/core/EnergyTracker.hpp>
#include<sudodem/core/Engine.hpp>
#include<sudodem/core/FileGenerator.hpp>
#include<sudodem/core/Functor.hpp>
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/InteractionContainer.hpp>
#include<sudodem/core/IGeom.hpp>
#include<sudodem/core/IPhys.hpp>
#include<sudodem/core/Material.hpp>
#include<sudodem/core/PartialEngine.hpp>
#include<sudodem/core/Shape.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/core/TimeStepper.hpp>


EnergyTracker::~EnergyTracker(){} // vtable