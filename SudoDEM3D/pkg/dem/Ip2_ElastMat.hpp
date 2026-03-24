#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/common/MatchMaker.hpp>
#include<sudodem/pkg/common/ElastMat.hpp>

class Ip2_ElastMat_ElastMat_NormPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction);
		
		// Use REGISTER_ATTRIBUTES for serialization
		
	FUNCTOR2D(ElastMat,ElastMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_ElastMat_ElastMat_NormPhys, IPhysFunctor);


class Ip2_ElastMat_ElastMat_NormShearPhys: public IPhysFunctor{
	public:
		virtual void go(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction);
		
		// Use REGISTER_ATTRIBUTES for serialization
		
	FUNCTOR2D(ElastMat,ElastMat);

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(Ip2_ElastMat_ElastMat_NormShearPhys, IPhysFunctor);