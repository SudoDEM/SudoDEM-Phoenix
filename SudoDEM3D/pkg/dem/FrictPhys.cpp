#include <sudodem/pkg/dem/FrictPhys.hpp>
#include <sudodem/pkg/dem/ScGeom.hpp>

REGISTER_CLASS_INDEX_CPP(FrictPhys,NormShearPhys)
REGISTER_CLASS_INDEX_CPP(ViscoFrictPhys,FrictPhys)

// The following code was moved from Ip2_FrictMat_FrictMat_FrictPhys.hpp

void Ip2_FrictMat_FrictMat_FrictPhys::go( const shared_ptr<Material>& b1
					, const shared_ptr<Material>& b2
					, const shared_ptr<Interaction>& interaction)
{
	if(interaction->phys) return;

	const shared_ptr<FrictMat>& mat1 = SUDODEM_PTR_CAST<FrictMat>(b1);
	const shared_ptr<FrictMat>& mat2 = SUDODEM_PTR_CAST<FrictMat>(b2);

	Real Ra,Rb;//Vector3r normal;
	assert(dynamic_cast<GenericSpheresContact*>(interaction->geom.get()));//only in debug mode
	GenericSpheresContact* sphCont=SUDODEM_CAST<GenericSpheresContact*>(interaction->geom.get());
	Ra=sphCont->refR1>0?sphCont->refR1:sphCont->refR2;
	Rb=sphCont->refR2>0?sphCont->refR2:sphCont->refR1;

	interaction->phys = shared_ptr<FrictPhys>(new FrictPhys());
	const shared_ptr<FrictPhys>& contactPhysics = SUDODEM_PTR_CAST<FrictPhys>(interaction->phys);
	// In SudoDEM3D, FrictMat uses direct Kn and Ks values instead of young and poisson
	Real Kn = (mat1->Kn + mat2->Kn) / 2.0;  // Average of the two stiffnesses
	Real Ks = (mat1->Ks + mat2->Ks) / 2.0;  // Average of the two stiffnesses

	Real frictionAngle = (!frictAngle) ? std::min(mat1->frictionAngle,mat2->frictionAngle) : (*frictAngle)(mat1->id,mat2->id,mat1->frictionAngle,mat2->frictionAngle);
	contactPhysics->tangensOfFrictionAngle = std::tan(frictionAngle);
	contactPhysics->kn = Kn;
	contactPhysics->ks = Ks;
};

void Ip2_FrictMat_FrictMat_ViscoFrictPhys::go( const shared_ptr<Material>& b1
					, const shared_ptr<Material>& b2
					, const shared_ptr<Interaction>& interaction)
{
	if(interaction->phys) return;
	const shared_ptr<FrictMat>& mat1 = SUDODEM_PTR_CAST<FrictMat>(b1);
	const shared_ptr<FrictMat>& mat2 = SUDODEM_PTR_CAST<FrictMat>(b2);
	interaction->phys = shared_ptr<ViscoFrictPhys>(new ViscoFrictPhys());
	const shared_ptr<ViscoFrictPhys>& contactPhysics = SUDODEM_PTR_CAST<ViscoFrictPhys>(interaction->phys);
	// In SudoDEM3D, FrictMat uses direct Kn and Ks values instead of young and poisson
	Real Kn = (mat1->Kn + mat2->Kn) / 2.0;  // Average of the two stiffnesses
	Real Ks = (mat1->Ks + mat2->Ks) / 2.0;  // Average of the two stiffnesses

	Real frictionAngle = (!frictAngle) ? std::min(mat1->frictionAngle,mat2->frictionAngle) : (*frictAngle)(mat1->id,mat2->id,mat1->frictionAngle,mat2->frictionAngle);
	contactPhysics->tangensOfFrictionAngle = std::tan(frictionAngle);
	contactPhysics->kn = Kn;
	contactPhysics->ks = Ks;
};

// pyRegisterClass implementations moved from FrictPhys.hpp
void FrictPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("FrictPhys");
	pybind11::class_<FrictPhys, NormShearPhys, std::shared_ptr<FrictPhys>> _classObj(_module, "FrictPhys", "The simple linear elastic-plastic interaction with friction angle, like in the traditional [CundallStrack1979]_");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("tangensOfFrictionAngle", &FrictPhys::tangensOfFrictionAngle, "tan of angle of friction");
}

void ViscoFrictPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("ViscoFrictPhys");
	pybind11::class_<ViscoFrictPhys, FrictPhys, std::shared_ptr<ViscoFrictPhys>> _classObj(_module, "ViscoFrictPhys", "Temporary version of :yref:`FrictPhys` for compatibility with e.g. :yref:`Law2_ScGeom6D_NormalInelasticityPhys_NormalInelasticity`");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("creepedShear", &ViscoFrictPhys::creepedShear, "Creeped force (parallel)");
}

void Ip2_FrictMat_FrictMat_FrictPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_FrictMat_FrictMat_FrictPhys");
	pybind11::class_<Ip2_FrictMat_FrictMat_FrictPhys, IPhysFunctor, std::shared_ptr<Ip2_FrictMat_FrictMat_FrictPhys>> _classObj(_module, "Ip2_FrictMat_FrictMat_FrictPhys", "Create a :yref:`FrictPhys` from two :yref:`FrictMats<FrictMat>`. The compliance of one disk under point load is defined here as $1/(E.D)$, with $E$ the stiffness of the disk and $D$ its diameter. The compliance of the contact itself will be the sum of compliances from each disk, i.e. $1/(E_1.D_1)+1/(E_2.D_2)$ in the general case, or $2/(E.D)$ in the special case of equal sizes and equal stiffness. Note that summing compliances corresponds to an harmonic average of stiffnesss (as in e.g. [Scholtes2009a]_), which is how kn is actually computed in the :yref:`Ip2_FrictMat_FrictMat_FrictPhys` functor:\n\n $k_n = \\frac{E_1D_1*E_2D_2}{E_1D_1+E_2D_2}=\\frac{k_1*k_2}{k_1+k_2}$, with $k_i=E_iD_i$.\n\n The shear stiffness ks of one disk is defined via the material parameter :yref:`ElastMat::poisson`, as ks=poisson*kn, and the resulting shear stiffness of the interaction will be also an harmonic average. In the case of a contact between a :yref:`ViscElMat` and a :yref:`FrictMat`, be sure to set :yref:`FrictMat::young` and :yref:`FrictMat::poisson`, otherwise the default value will be used.");
	_classObj.def(pybind11::init<>());
	_classObj.def_readwrite("frictAngle", &Ip2_FrictMat_FrictMat_FrictPhys::frictAngle, "Instance of :yref:`MatchMaker` determining how to compute interaction's friction angle. If ``None``, minimum value is used.");
}

void Ip2_FrictMat_FrictMat_ViscoFrictPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_FrictMat_FrictMat_ViscoFrictPhys");
	pybind11::class_<Ip2_FrictMat_FrictMat_ViscoFrictPhys, Ip2_FrictMat_FrictMat_FrictPhys, std::shared_ptr<Ip2_FrictMat_FrictMat_ViscoFrictPhys>> _classObj(_module, "Ip2_FrictMat_FrictMat_ViscoFrictPhys", "Create a :yref:`FrictPhys` from two :yref:`FrictMats<FrictMat>`. The compliance of one disk under symetric point loads is defined here as 1/(E.r), with E the stiffness of the disk and r its radius, and corresponds to a compliance 1/(2.E.r)=1/(E.D) from each contact point. The compliance of the contact itself will be the sum of compliances from each disk, i.e. 1/(E.D1)+1/(E.D2) in the general case, or 1/(E.r) in the special case of equal sizes. Note that summing compliances corresponds to an harmonic average of stiffnesss, which is how kn is actually computed in the :yref:`Ip2_FrictMat_FrictMat_FrictPhys` functor.\n\nThe shear stiffness ks of one disk is defined via the material parameter :yref:`ElastMat::poisson`, as ks=poisson*kn, and the resulting shear stiffness of the interaction will be also an harmonic average.");
	_classObj.def(pybind11::init<>());
}
