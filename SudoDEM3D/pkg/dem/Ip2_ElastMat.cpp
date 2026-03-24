#include <sudodem/pkg/dem/Ip2_ElastMat.hpp>

#include <sudodem/pkg/common/NormShearPhys.hpp>
#include <sudodem/pkg/dem/DemXDofGeom.hpp>



void Ip2_ElastMat_ElastMat_NormPhys::go( const shared_ptr<Material>& b1
					, const shared_ptr<Material>& b2
					, const shared_ptr<Interaction>& interaction)
{
	if(interaction->phys) return;
	const shared_ptr<ElastMat>& mat1 = SUDODEM_PTR_CAST<ElastMat>(b1);
	const shared_ptr<ElastMat>& mat2 = SUDODEM_PTR_CAST<ElastMat>(b2);
	interaction->phys = shared_ptr<NormPhys>(new NormPhys());
	const shared_ptr<NormPhys>& phys = SUDODEM_PTR_CAST<NormPhys>(interaction->phys);
	Real Kn;
	const GenericSpheresContact* geom=dynamic_cast<GenericSpheresContact*>(interaction->geom.get());
	if (geom) {
		Real Ra,Rb;//Vector3r normal;
		Ra=geom->refR1>0?geom->refR1:geom->refR2;
		Rb=geom->refR2>0?geom->refR2:geom->refR1;
		//harmonic average of the two stiffnesses when (Ri.Ei/2) is the stiffness of a contact point on sphere "i"
		Kn =  (mat1->Kn + mat2->Kn) / 2.0;
	} else {
		Kn = (mat1->Kn + mat2->Kn) / 2.0;
	}
	phys->kn = Kn;
};


void Ip2_ElastMat_ElastMat_NormShearPhys::go( const shared_ptr<Material>& b1
					, const shared_ptr<Material>& b2
					, const shared_ptr<Interaction>& interaction)
{
	if(interaction->phys) return;
	const shared_ptr<ElastMat>& mat1 = SUDODEM_PTR_CAST<ElastMat>(b1);
	const shared_ptr<ElastMat>& mat2 = SUDODEM_PTR_CAST<ElastMat>(b2);
	interaction->phys = shared_ptr<NormShearPhys>(new NormShearPhys());
	const shared_ptr<NormShearPhys>& phys = SUDODEM_PTR_CAST<NormShearPhys>(interaction->phys);
	Real Kn=0.0, Ks=0.0;
	GenericSpheresContact* geom=dynamic_cast<GenericSpheresContact*>(interaction->geom.get());
	if (geom) {
		Real Ra,Rb;//Vector3r normal;
		Ra=geom->refR1>0?geom->refR1:geom->refR2;
		Rb=geom->refR2>0?geom->refR2:geom->refR1;
		//harmonic average of the two stiffnesses when (Ri.Ei/2) is the stiffness of a contact point on sphere "i"
		Kn =  (mat1->Kn + mat2->Kn) / 2.0;
		Ks = (mat1->Ks + mat2->Ks) / 2.0;
	} else {
		Kn = (mat1->Kn + mat2->Kn) / 2.0;
		Ks = (mat1->Ks + mat2->Ks) / 2.0;
	}
	phys->kn = Kn;
	phys->ks = Ks;
};

// pyRegisterClass implementations moved from Ip2_ElastMat.hpp
void Ip2_ElastMat_ElastMat_NormPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_ElastMat_ElastMat_NormPhys");
	pybind11::class_<Ip2_ElastMat_ElastMat_NormPhys, IPhysFunctor, std::shared_ptr<Ip2_ElastMat_ElastMat_NormPhys>> _classObj(_module, "Ip2_ElastMat_ElastMat_NormPhys", "Create a NormPhys from two ElastMats.");
	_classObj.def(pybind11::init<>());
}

void Ip2_ElastMat_ElastMat_NormShearPhys::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Ip2_ElastMat_ElastMat_NormShearPhys");
	pybind11::class_<Ip2_ElastMat_ElastMat_NormShearPhys, IPhysFunctor, std::shared_ptr<Ip2_ElastMat_ElastMat_NormShearPhys>> _classObj(_module, "Ip2_ElastMat_ElastMat_NormShearPhys", "Create a NormShearPhys from two ElastMats.");
	_classObj.def(pybind11::init<>());
}
