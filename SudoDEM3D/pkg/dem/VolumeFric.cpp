#include<sudodem/core/Interaction.hpp>
#include<sudodem/core/Omega.hpp>
#include<sudodem/core/Scene.hpp>
#include<sudodem/core/State.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>

#include<sudodem/pkg/dem/VolumeFric.hpp>

REGISTER_CLASS_INDEX_CPP(VolumeFricMat,Material)
REGISTER_CLASS_INDEX_CPP(VolumeFricPhys,IPhys)

/* Material law, physics */

void Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys::go( const shared_ptr<Material>& b1
					, const shared_ptr<Material>& b2
					, const shared_ptr<Interaction>& interaction)
{
	if(interaction->phys) return;
	const shared_ptr<VolumeFricMat>& mat1 = SUDODEM_PTR_CAST<VolumeFricMat>(b1);
	const shared_ptr<VolumeFricMat>& mat2 = SUDODEM_PTR_CAST<VolumeFricMat>(b2);
	interaction->phys = shared_ptr<VolumeFricPhys>(new VolumeFricPhys());
	const shared_ptr<VolumeFricPhys>& contactPhysics = SUDODEM_PTR_CAST<VolumeFricPhys>(interaction->phys);
	Real Kna 	= mat1->Kn;
	Real Knb 	= mat2->Kn;
	Real Ksa 	= mat1->Ks;
	Real Ksb 	= mat2->Ks;
	Real frictionAngle = std::min(mat1->frictionAngle,mat2->frictionAngle);
        contactPhysics->tangensOfFrictionAngle = std::tan(frictionAngle);
	contactPhysics->kn = Kna*Knb/(Kna+Knb);
	contactPhysics->ks = Ksa*Ksb/(Ksa+Ksb);
};


//**************************************************************************************
// Apply forces on polyhedrons in collision based on geometric configuration
bool VolumetricLaw::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* contact){
	int id1 = contact->getId1(), id2 = contact->getId2();

	ScGeom*    geom= static_cast<ScGeom*>(ig.get());
	//FrictPhys* phys = static_cast<FrictPhys*>(ip.get());
	VolumeFricPhys* phys = dynamic_cast<VolumeFricPhys*>(contact->phys.get());
	if(geom->penetrationDepth <0){
		if (neverErase) {
			phys->shearForce = Vector3r::Zero();
			phys->normalForce = Vector3r::Zero();}
		else return false;
	}
	//Real& un=geom->penetrationDepth;
	//calculate normal force in terms of volumetric stiffness
	//calculate the volume of overlap
	//sphere and sphere

	Real ra = geom->radius1;
	Real rb = geom->radius2;
	Real dab = ra + rb - geom->penetrationDepth;
	Real ha = ra - 0.5*dab - (std::pow(ra,2)-std::pow(rb,2))/dab*0.5;
	Real hb = rb - 0.5*dab + (std::pow(ra,2)-std::pow(rb,2))/dab*0.5;
	Real Volume = Mathr::PI/3.*(std::pow(ha,2)*(3.*ra - ha) + std::pow(hb,2)*(3.*rb - hb));
	phys->normalForce=phys->kn*std::max(Volume,(Real) 0)*geom->normal;

	Vector3r& shearForce = geom->rotate(phys->shearForce);
	const Vector3r& shearDisp = geom->shearIncrement();
	shearForce -= phys->ks*shearDisp;
	Real maxFs = phys->normalForce.squaredNorm()*std::pow(phys->tangensOfFrictionAngle,2);
	// PFC3d SlipModel, is using friction angle. CoulombCriterion
	if( shearForce.squaredNorm() > maxFs ){
		Real ratio = sqrt(maxFs) / shearForce.norm();
		shearForce *= ratio;}


	//we need to use correct branches in the periodic case, the following apply for spheres only
	Vector3r force = -phys->normalForce-shearForce;
	scene->forces.addForce(id1,force);
	scene->forces.addForce(id2,-force);
	scene->forces.addTorque(id1,(geom->radius1-0.5*geom->penetrationDepth)* geom->normal.cross(force));
	scene->forces.addTorque(id2,(geom->radius2-0.5*geom->penetrationDepth)* geom->normal.cross(force));

	return true;

}

//**************************************************************************************
// Compute elastic energy
Real VolumetricLaw::elasticEnergy() {
	// Calculate total elastic energy from all interactions
	Real energy = 0;
	for (const auto& intr : *scene->interactions) {
		if (!intr->isReal()) continue;
		if (intr->phys) {
			const VolumeFricPhys* phys = dynamic_cast<VolumeFricPhys*>(intr->phys.get());
			if (phys) {
				// Elastic energy from normal and shear forces
				energy += 0.5 * (phys->normalForce.squaredNorm() / phys->kn + phys->shearForce.squaredNorm() / phys->ks);
			}
		}
	}
	return energy;
}

//**************************************************************************************
// Get plastic dissipation
Real VolumetricLaw::getPlasticDissipation() {
	return plasticDissipation.get();
}

//**************************************************************************************
// Initialize plastic dissipation
void VolumetricLaw::initPlasticDissipation(Real initVal) {
	plasticDissipation.reset();
	plasticDissipation.set(initVal);
}

void VolumeFricMat::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("VolumeFricMat");
	pybind11::class_<VolumeFricMat, Material, std::shared_ptr<VolumeFricMat>> _classObj(_module, "VolumeFricMat", "Elastic material with Coulomb friction");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("Kn", &VolumeFricMat::Kn, "Normal volumetric stiffness (N/m3)");
	_classObj.def_readwrite("Ks", &VolumeFricMat::Ks, "Shear stiffness (N/m)");
	_classObj.def_readwrite("frictionAngle", &VolumeFricMat::frictionAngle, "Contact friction angle (in radians)");
	_classObj.def_readwrite("IsSplitable", &VolumeFricMat::IsSplitable, "To be splitted");
	_classObj.def_readwrite("strength", &VolumeFricMat::strength, "Stress at which polyhedra breaks");
}

void VolumeFricPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("VolumeFricPhys");
	pybind11::class_<VolumeFricPhys, IPhys, std::shared_ptr<VolumeFricPhys>> _classObj(_module, "VolumeFricPhys", "Simple elastic material with friction for volumetric constitutive laws");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("kn", &VolumeFricPhys::kn, "Normal stiffness");
	_classObj.def_readwrite("normalForce", &VolumeFricPhys::normalForce, "Normal force after previous step");
	_classObj.def_readwrite("ks", &VolumeFricPhys::ks, "Shear stiffness");
	_classObj.def_readwrite("shearForce", &VolumeFricPhys::shearForce, "Shear force after previous step");
	_classObj.def_readwrite("tangensOfFrictionAngle", &VolumeFricPhys::tangensOfFrictionAngle, "tangens of angle of internal friction");
}

void Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys");
	pybind11::class_<Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys, IPhysFunctor, std::shared_ptr<Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys>> _classObj(_module, "Ip2_VolumeFricMat_VolumeFricMat_VolumeFricPhys", "Material to physics functor");
	_classObj.def(pybind11::init<>());
}

void VolumetricLaw::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("VolumetricLaw");
	pybind11::class_<VolumetricLaw, LawFunctor, std::shared_ptr<VolumetricLaw>> _classObj(_module, "VolumetricLaw", "Calculate physical response of two polyhedra in interaction");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("shearForce", &VolumetricLaw::shearForce, "Shear force from last step");
	_classObj.def_readwrite("neverErase", &VolumetricLaw::neverErase, "Keep interactions even if particles go away from each other");
	_classObj.def_readwrite("traceEnergy", &VolumetricLaw::traceEnergy, "Define the total energy dissipated in plastic slips");
	_classObj.def("elasticEnergy", &VolumetricLaw::elasticEnergy, "Compute and return the total elastic energy");
	_classObj.def("getPlasticDissipation", &VolumetricLaw::getPlasticDissipation, "Total energy dissipated in plastic slips");
	_classObj.def("initPlasticDissipation", &VolumetricLaw::initPlasticDissipation, "Initialize cummulated plastic dissipation to a value");
}
