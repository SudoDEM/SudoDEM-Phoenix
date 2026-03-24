// 2007,2008 © Václav Šmilauer <eudoxos@arcig.cz>

#include<sudodem/lib/base/Math.hpp>
#include<list>
#include<signal.h>
#include<memory>
#include<functional>
#include<thread>
#include<chrono>
#include<algorithm>
#include<locale>
#include<random>
#include<ctime>

#include<pybind11/pybind11.h>
#include<pybind11/stl.h>
#include<pybind11/eigen.h>
#include<pybind11/functional.h>
#include<pybind11/chrono.h>

#include<sudodem/lib/base/Logging.hpp>
#include<sudodem/lib/pyutil/gil.hpp>
#include<sudodem/lib/pyutil/doc_opts.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/ThreadRunner.hpp>
#include<sudodem/core/Timing.hpp>
#include<sudodem/lib/serialization/ObjectIO.hpp>
#include<sudodem/lib/serialization/Serializable.hpp>
#include<sudodem/pkg/common/InteractionLoop.hpp>
#include<sudodem/pkg/common/Collider.hpp>
#include<sudodem/core/Interaction.hpp>
#include<sudodem/pkg/common/Dispatching.hpp>

// Forward declaration for the direct registration function
extern "C" void registerAllDirectly(pybind11::module_& m);

// Forward declarations for submodule initialization functions
void pybind_init__utils(pybind11::module& m);
void pybind_init__customConverters(pybind11::module& m);
void pybind_init__superquadrics_utils(pybind11::module& m);
void pybind_init__fem_utils(pybind11::module& m);
void pybind_init__gjkparticle_utils(pybind11::module& m);

// Include wrapper class declarations
#include "PyWrapperClasses.hpp"

namespace py = pybind11;


PYBIND11_MODULE(wrapper, m)
{
	m.doc() = "Wrapper for c++ internals of sudodem.";

	SUDODEM_SET_DOCSTRING_OPTS;

	pybind11::enum_<sudodem::Attr::flags>(m, "AttrFlags")
		.value("noSave",sudodem::Attr::noSave)
		.value("readonly",sudodem::Attr::readonly)
		.value("triggerPostLoad",sudodem::Attr::triggerPostLoad)
		.value("noResize",sudodem::Attr::noResize)
    ;

	pybind11::class_<pyOmega>(m, "Omega")
		.def(pybind11::init<>())
		.def_property_readonly("iter",&pyOmega::iter,"Get current step number")
		.def_property_readonly("subStep",&pyOmega::subStep,"Get the current subStep number (only meaningful if O.subStepping==True); -1 when outside the loop, otherwise either 0 (O.subStepping==False) or number of engine to be run (O.subStepping==True)")
		.def_property("subStepping",&pyOmega::subStepping_get,&pyOmega::subStepping_set,"Get/set whether subStepping is active.")
		.def_property("stopAtIter",&pyOmega::stopAtIter_get,&pyOmega::stopAtIter_set,"Get/set number of iteration after which the simulation will stop.")
		.def_property("stopAtTime",&pyOmega::stopAtTime_get,&pyOmega::stopAtTime_set,"Get/set time after which the simulation will stop.")
		.def_property_readonly("time",&pyOmega::time,"Return virtual (model world) time of the simulation.")
		.def_property_readonly("realtime",&pyOmega::realTime,"Return clock (human world) time the simulation has been running.")
		.def_property_readonly("speed",&pyOmega::speed,"Return current calculation speed [iter/sec].")
		.def_property("dt",&pyOmega::dt_get,&pyOmega::dt_set,"Current timestep (Δt) value.")
		.def_property("dynDt",&pyOmega::dynDt_get,&pyOmega::dynDt_set,"Whether a :yref:`TimeStepper` is used for dynamic Δt control. See :yref:`dt<Omega.dt>` on how to enable/disable :yref:`TimeStepper`.")
		.def_property_readonly("dynDtAvailable",&pyOmega::dynDtAvailable_get,"Whether a :yref:`TimeStepper` is amongst :yref:`O.engines<Omega.engines>`, activated or not.")
		.def("load",&pyOmega::load,pybind11::arg("file"),pybind11::arg("quiet")=false,"Load simulation from file. The file should be :yref:`saved<Omega.save>` in the same version of SudoDEM, otherwise compatibility is not guaranteed.")
		.def("reload",&pyOmega::reload,pybind11::arg("quiet")=false,"Reload current simulation")
		.def("save",&pyOmega::save,pybind11::arg("file"),pybind11::arg("quiet")=false,"Save current simulation to file (should be .xml or .xml.bz2). The file should be :yref:`loaded<Omega.load>` in the same version of SudoDEM, otherwise compatibility is not guaranteed.")
		.def("loadTmp",&pyOmega::loadTmp,pybind11::arg("mark")="",pybind11::arg("quiet")=false,"Load simulation previously stored in memory by saveTmp. *mark* optionally distinguishes multiple saved simulations")
		.def("saveTmp",&pyOmega::saveTmp,pybind11::arg("mark")="",pybind11::arg("quiet")=false,"Save simulation to memory (disappears at shutdown), can be loaded later with loadTmp. *mark* optionally distinguishes different memory-saved simulations.")
		.def("lsTmp",&pyOmega::lsTmp,"Return list of all memory-saved simulations.")
		.def("tmpToFile",&pyOmega::tmpToFile,pybind11::arg("mark"),pybind11::arg("fileName"),"Save XML of :yref:`saveTmp<Omega.saveTmp>`'d simulation into *fileName*.")
		.def("tmpToString",&pyOmega::tmpToString,pybind11::arg("mark")="","Return XML of :yref:`saveTmp<Omega.saveTmp>`'d simulation as string.")
		.def("run",&pyOmega::run,pybind11::arg("nSteps")=-1,pybind11::arg("wait")=false,"Run the simulation. *nSteps* how many steps to run, then stop (if positive); *wait* will cause not returning to python until simulation will have stopped.")
		.def("pause",&pyOmega::pause,"Stop simulation execution. (May be called from within the loop, and it will stop after the current step).")
		.def("step",&pyOmega::step,"Advance the simulation by one step. Returns after the step will have finished.")
		.def("wait",&pyOmega::wait,"Don't return until the simulation will have been paused. (Returns immediately if not running).")
		.def_property_readonly("running",&pyOmega::isRunning,"Whether background thread is currently running a simulation.")
		.def_property_readonly("filename",&pyOmega::get_filename,"Filename under which the current simulation was saved (None if never saved).")
		.def("reset",&pyOmega::reset,"Reset simulations completely (including another scenes!).")
		.def("resetThisScene",&pyOmega::resetThisScene,"Reset current scene.")
		.def("resetCurrentScene",&pyOmega::resetCurrentScene,"Reset current scene.")
		.def("resetAllScenes",&pyOmega::resetAllScenes,"Reset all scenes.")
		.def("addScene",&pyOmega::addScene,"Add new scene to Omega, returns its number")
		.def("switchToScene",&pyOmega::switchToScene,"Switch to defined scene. Default scene has number 0, other scenes have to be created by addScene method.")
		.def("switchScene",&pyOmega::switchScene,"Switch to alternative simulation (while keeping the old one). Calling the function again switches back to the first one. Note that most variables from the first simulation will still refer to the first simulation even after the switch\n(e.g. b=O.bodies[4]; O.switchScene(); [b still refers to the body in the first simulation here])")
		.def("sceneToString",&pyOmega::sceneToString,"Return the entire scene as a string. Equivalent to using O.save(...) except that the scene goes to a string instead of a file. (see also stringToScene())")
		.def("stringToScene",&pyOmega::stringToScene,pybind11::arg("sstring"),pybind11::arg("mark")="","Load simulation from a string passed as argument (see also sceneToString).")
		.def("labeledEngine",&pyOmega::labeled_engine_get,"Return instance of engine/functor with the given label. This function shouldn't be called by the user directly; every ehange in O.engines will assign respective global python variables according to labels.\n\nFor example:\n\n\t *O.engines=[InsertionSortCollider(label='collider')]*\n\n\t *collider.nBins=5 # collider has become a variable after assignment to O.engines automatically*")
		.def("resetTime",&pyOmega::resetTime,"Reset simulation time: step number, virtual and real time. (Doesn't touch anything else, including timings).")
		.def("plugins",&pyOmega::plugins_get,"Return list of all plugins registered in the class factory.")
		.def("_sceneObj",&pyOmega::scene_get,"Return the :yref:`scene <Scene>` object. Debugging only, all (or most) :yref:`Scene` functionality is proxies through :yref:`Omega`.")
		.def_property("engines",&pyOmega::engines_get,&pyOmega::engines_set,"List of engines in the simulation (Scene::engines).")
		.def_property_readonly("_currEngines",&pyOmega::currEngines_get,"Currently running engines; debugging only!")
		.def_property_readonly("_nextEngines",&pyOmega::nextEngines_get,"Engines for the next step, if different from the current ones, otherwise empty; debugging only!")
		.def_property("miscParams",&pyOmega::miscParams_get,&pyOmega::miscParams_set,"MiscParams in the simulation (Scene::mistParams), usually used to save serializables that don't fit anywhere else, like GL functors")
		.def_property_readonly("bodies",&pyOmega::bodies_get,"Bodies in the current simulation (container supporting index access by id and iteration)")
		.def_property_readonly("interactions",&pyOmega::interactions_get,"Interactions in the current simulation (container supporting index acces by either (id1,id2) or interactionNumber and iteration)")
		.def_property_readonly("materials",&pyOmega::materials_get,"Shared materials; they can be accessed by id or by label")
		.def_property_readonly("forces",&pyOmega::forces_get,":yref:`ForceContainer` (forces, torques, displacements) in the current simulation.")
		.def_property_readonly("energy",&pyOmega::energy_get,":yref:`EnergyTracker` of the current simulation. (meaningful only with :yref:`O.trackEnergy<Omega.trackEnergy>`)")
		.def_property("trackEnergy",&pyOmega::trackEnergy_get,&pyOmega::trackEnergy_set,"When energy tracking is enabled or disabled in this simulation.")
		.def_property_readonly("tags",&pyOmega::tags_get,"Tags (string=string dictionary) of the current simulation (container supporting string-index access/assignment)")
		.def("childClassesNonrecursive",&pyOmega::listChildClassesNonrecursive,"Return list of all classes deriving from given class, as registered in the class factory")
		.def("isChildClassOf",&pyOmega::isChildClassOf,"Tells whether the first class derives from the second one (both given as strings).")
		.def_property("timingEnabled",&pyOmega::timingEnabled_get,&pyOmega::timingEnabled_set,"Globally enable/disable timing services (see documentation of the :yref:`timing module<sudodem.timing>`).")
		.def_property("forceSyncCount",&pyOmega::forceSyncCount_get,&pyOmega::forceSyncCount_set,"Counter for number of syncs in ForceContainer, for profiling purposes.")
		.def_property_readonly("numThreads",&pyOmega::numThreads_get /* ,&pyOmega::numThreads_set*/ ,"Get maximum number of threads openMP can use.")
		.def_property_readonly("cell",&pyOmega::cell_get,"Periodic cell of the current scene (None if the scene is aperiodic).")
		.def_property("periodic",&pyOmega::periodic_get,&pyOmega::periodic_set,"Get/set whether the scene is periodic or not (True/False).")
		.def("exitNoBacktrace",&pyOmega::exitNoBacktrace,pybind11::arg("status")=0,"Disable SEGV handler and exit, optionally with given status number.")
		.def("disableGdb",&pyOmega::disableGdb,"Revert SEGV and ABRT handlers to system defaults.")
		.def("runEngine",&pyOmega::runEngine,"Run given engine exactly once; simulation time, step number etc. will not be incremented (use only if you know what you do).")
		.def("tmpFilename",&pyOmega::tmpFilename,"Return unique name of file in temporary directory which will be deleted when sudodem exits.")
		;
	pybind11::class_<pyTags>(m, "TagsWrapper","Container emulating dictionary semantics for accessing tags associated with simulation. Tags are accesed by strings.")
		.def("__getitem__",&pyTags::getItem)
		.def("__setitem__",&pyTags::setItem)
		.def("keys",&pyTags::keys)
		.def("has_key",&pyTags::hasKey);
	pybind11::class_<pyBodyContainer>(m, "BodyContainer")
		.def("__getitem__",&pyBodyContainer::pyGetitem)
		.def("__len__",&pyBodyContainer::length)
		.def("__iter__",&pyBodyContainer::pyIter)
		.def("append",&pyBodyContainer::append,"Append one Body instance, return its id.")
		.def("append",&pyBodyContainer::appendList,"Append list of Body instance, return list of ids")
		.def("appendClumped",&pyBodyContainer::appendClump,pybind11::arg("bb"),pybind11::arg("discretization")=0,"Append given list of bodies as a clump (rigid aggregate); returns a tuple of ``(clumpId,[memberId1,memberId2,...])``. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).")
		.def("clump",&pyBodyContainer::clump,pybind11::arg("ids"),pybind11::arg("discretization")=0,"Clump given bodies together (creating a rigid aggregate); returns ``clumpId``. Clump masses and inertia are adapted automatically when discretization>0. If clump members are overlapping this is done by integration/summation over mass points using a regular grid of cells (number of grid cells in one direction is defined as $R_{min}/discretization$, where $R_{min}$ is minimum clump member radius). For non-overlapping members inertia of the clump is the sum of inertias from members. If discretization<=0 sum of inertias from members is used (faster, but inaccurate).")
		.def("updateClumpProperties",&pyBodyContainer::updateClumpProperties,pybind11::arg("excludeList")=pybind11::list(),pybind11::arg("discretization")=5,"Update clump properties mass, volume and inertia (for details of 'discretization' value see :yref:`clump()<BodyContainer.clump>`). Clumps can be excluded from the calculation by giving a list of ids: *O.bodies.updateProperties([ids])*.")
		.def("addToClump",&pyBodyContainer::addToClump,pybind11::arg("bids"),pybind11::arg("cid"),pybind11::arg("discretization")=0,"Add body b (or a list of bodies) to an existing clump c. c must be clump and b may not be a clump member of c. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).\n\nSee :ysrc:`examples/clumps/addToClump-example.py` for an example script.\n\n.. note:: If b is a clump itself, then all members will be added to c and b will be deleted. If b is a clump member of clump d, then all members from d will be added to c and d will be deleted. If you need to add just clump member b, :yref:`release<BodyContainer.releaseFromClump>` this member from d first.")
		.def("releaseFromClump",&pyBodyContainer::releaseFromClump,pybind11::arg("bid"),pybind11::arg("cid"),pybind11::arg("discretization")=0,"Release body b from clump c. b must be a clump member of c. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`).\n\nSee :ysrc:`examples/clumps/releaseFromClump-example.py` for an example script.\n\n.. note:: If c contains only 2 members b will not be released and a warning will appear. In this case clump c should be :yref:`erased<BodyContainer.erase>`.")
		.def("replaceByClumps",&pyBodyContainer::replaceByClumps,pybind11::arg("ctList"),pybind11::arg("amounts"),pybind11::arg("discretization")=0,"Replace spheres by clumps using a list of clump templates and a list of amounts; returns a list of tuples: ``[(clumpId1,[memberId1,memberId2,...]),(clumpId2,[memberId1,memberId2,...]),...]``. A new clump will have the same volume as the sphere, that was replaced. Clump masses and inertia are adapted automatically (for details see :yref:`clump()<BodyContainer.clump>`). \n\n\t *O.bodies.replaceByClumps( [utils.clumpTemplate([1,1],[.5,.5])] , [.9] ) #will replace 90 % of all standalone spheres by 'dyads'*\n\nSee :ysrc:`examples/clumps/replaceByClumps-example.py` for an example script.")
		.def("getRoundness",&pyBodyContainer::getRoundness,pybind11::arg("excludeList")=pybind11::list(),"Returns roundness coefficient RC = R2/R1. R1 is the theoretical radius of a sphere, with same volume as clump. R2 is the minimum radius of a sphere, that imbeds clump. If just spheres are present RC = 1. If clumps are present 0 < RC < 1. Bodies can be excluded from the calculation by giving a list of ids: *O.bodies.getRoundness([ids])*.\n\nSee :ysrc:`examples/clumps/replaceByClumps-example.py` for an example script.")
		.def("clear", &pyBodyContainer::clear,"Remove all bodies (interactions not checked)")
		.def("erase", &pyBodyContainer::erase,pybind11::arg("id"),pybind11::arg("eraseClumpMembers")=0,"Erase body with the given id; all interaction will be deleted by InteractionLoop in the next step. If a clump is erased use *O.bodies.erase(clumpId,True)* to erase the clump AND its members.")
		.def("replace",&pyBodyContainer::replace);
	pybind11::class_<pyBodyIterator>(m, "BodyIterator").def(pybind11::init<const pyBodyIterator&>())
		.def("__iter__",&pyBodyIterator::pyIter)
		.def("__next__",&pyBodyIterator::pyNext);

	pybind11::class_<pyInteractionContainer>(m, "InteractionContainer", "Access to :yref:`interactions<Interaction>` of simulation, by using \n\n#. id's of both :yref:`Bodies<Body>` of the interactions, e.g. ``O.interactions[23,65]``\n#. iteraction over the whole container::\n\n\tfor i in O.interactions: print i.id1,i.id2\n\n.. note::\n\tIteration silently skips interactions that are not :yref:`real<Interaction.isReal>`.").def(pybind11::init<const pyInteractionContainer&>())
		.def("__iter__",&pyInteractionContainer::pyIter)
		.def("__getitem__",&pyInteractionContainer::pyGetitem)
		.def("__len__",&pyInteractionContainer::len)
		.def("countReal",&pyInteractionContainer::countReal,"Return number of interactions that are \"real\", i.e. they have phys and geom.")
		.def("nth",&pyInteractionContainer::pyNth,"Return n-th interaction from the container (usable for picking random interaction).")
		.def("withBody",&pyInteractionContainer::withBody,"Return list of real interactions of given body.")
		.def("withBodyAll",&pyInteractionContainer::withBodyAll,"Return list of all (real as well as non-real) interactions of given body.")
		.def("eraseNonReal",&pyInteractionContainer::eraseNonReal,"Erase all interactions that are not :yref:`real <InteractionContainer.isReal>`.")
		.def("erase",&pyInteractionContainer::erase,"Erase one interaction, given by id1, id2 (internally, ``requestErase`` is called -- the interaction might still exist as potential, if the :yref:`Collider` decides so).")
		.def_property("serializeSorted",&pyInteractionContainer::serializeSorted_get,&pyInteractionContainer::serializeSorted_set)
		.def("clear",&pyInteractionContainer::clear,"Remove all interactions, and invalidate persistent collider data (if the collider supports it).");
	pybind11::class_<pyInteractionIterator>(m, "InteractionIterator").def(pybind11::init<const pyInteractionIterator&>())
		.def("__iter__",&pyInteractionIterator::pyIter)
		.def("__next__",&pyInteractionIterator::pyNext);

	pybind11::class_<pyForceContainer>(m, "ForceContainer").def(pybind11::init<const pyForceContainer&>())
		.def("f",&pyForceContainer::force_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Force applied on body. For clumps in openMP, synchronize the force container with sync=True, else the value will be wrong.")
		.def("t",&pyForceContainer::torque_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Torque applied on body. For clumps in openMP, synchronize the force container with sync=True, else the value will be wrong.")
		.def("m",&pyForceContainer::torque_get,pybind11::arg("id"),pybind11::arg("sync")=false,"Deprecated alias for t (torque).")
		.def("move",&pyForceContainer::move_get,(pybind11::arg("id")),"Displacement applied on body.")
		.def("rot",&pyForceContainer::rot_get,(pybind11::arg("id")),"Rotation applied on body.")
		.def("addF",&pyForceContainer::force_add,pybind11::arg("id"),pybind11::arg("f"),pybind11::arg("permanent")=false,"Apply force on body (accumulates).\n\n # If permanent=false (default), the force applies for one iteration, then it is reset by ForceResetter. \n # If permanent=true, it persists over iterations, until it is overwritten by another call to addF(id,f,True) or removed by reset(resetAll=True). The permanent force on a body can be checked with permF(id).")
		.def("addT",&pyForceContainer::torque_add,pybind11::arg("id"),pybind11::arg("t"),pybind11::arg("permanent")=false,"Apply torque on body (accumulates). \n\n # If permanent=false (default), the torque applies for one iteration, then it is reset by ForceResetter. \n # If permanent=true, it persists over iterations, until it is overwritten by another call to addT(id,f,True) or removed by reset(resetAll=True). The permanent torque on a body can be checked with permT(id).")
		.def("permF",&pyForceContainer::permForce_get,(pybind11::arg("id")),"read the value of permanent force on body (set with setPermF()).")
		.def("permT",&pyForceContainer::permTorque_get,(pybind11::arg("id")),"read the value of permanent torque on body (set with setPermT()).")
		.def("addMove",&pyForceContainer::move_add,pybind11::arg("id"),pybind11::arg("m"),"Apply displacement on body (accumulates).")
		.def("addRot",&pyForceContainer::rot_add,pybind11::arg("id"),pybind11::arg("r"),"Apply rotation on body (accumulates).")
		.def("reset",&pyForceContainer::reset,(pybind11::arg("resetAll")=true),"Reset the force container, including user defined permanent forces/torques. resetAll=False will keep permanent forces/torques unchanged.")
		.def("getPermForceUsed",&pyForceContainer::getPermForceUsed,"Check wether permanent forces are present.")
		.def_property("syncCount",&pyForceContainer::syncCount_get,&pyForceContainer::syncCount_set,"Number of synchronizations  of ForceContainer (cummulative); if significantly higher than number of steps, there might be unnecessary syncs hurting performance.")
		;
	pybind11::class_<pyMaterialContainer>(m, "MaterialContainer", "Container for :yref:`Materials<Material>`. A material can be accessed using \n\n #. numerical index in range(0,len(cont)), like cont[2]; \n #. textual label that was given to the material, like cont['steel']. This etails traversing all materials and should not be used frequently.").def(pybind11::init<const pyMaterialContainer&>())
		.def("append",&pyMaterialContainer::append,"Add new shared :yref:`Material`; changes its id and return it.")
		.def("append",&pyMaterialContainer::appendList,"Append list of :yref:`Material` instances, return list of ids.")
		.def("index",&pyMaterialContainer::index,"Return id of material, given its label.")
		.def("__getitem__",&pyMaterialContainer::getitem_id)
		.def("__getitem__",&pyMaterialContainer::getitem_label)
		.def("__len__",&pyMaterialContainer::len);

//	py::class_<STLImporter>("STLImporter").def("ymport",&STLImporter::import);

//////////////////////////////////////////////////////////////
///////////// proxyless wrappers
	
	::registerAllDirectly(m);
	
	pybind11::module_(m).attr("O")=pyOmega();

	// Initialize merged submodules
	auto _utils = m.def_submodule("_utils", "Utility functions for 3D SudoDEM.");
	pybind_init__utils(_utils);
	
	auto _customConverters = m.def_submodule("_customConverters", "Custom type converters for pybind11.");
	pybind_init__customConverters(_customConverters);
	
	auto _superquadrics_utils = m.def_submodule("_superquadrics_utils", "Superquadrics utility functions for 3D SudoDEM.");
	pybind_init__superquadrics_utils(_superquadrics_utils);
	
	auto _fem_utils = m.def_submodule("_fem_utils", "FEM utility functions for 3D SudoDEM.");
	pybind_init__fem_utils(_fem_utils);
	
	auto _gjkparticle_utils = m.def_submodule("_gjkparticle_utils", "GJK particle utility functions for 3D SudoDEM.");
	pybind_init__gjkparticle_utils(_gjkparticle_utils);
	
	// Expose submodules in main module namespace for direct import
	m.attr("_utils") = _utils;
	m.attr("_customConverters") = _customConverters;
	m.attr("_superquadrics_utils") = _superquadrics_utils;
	m.attr("_fem_utils") = _fem_utils;
	m.attr("_gjkparticle_utils") = _gjkparticle_utils;
}
