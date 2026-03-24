// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/core/GlobalEngine.hpp>
#include<sudodem/core/Interaction.hpp>

class FacetTopologyAnalyzer: public GlobalEngine{
	struct VertexData{
		VertexData(Body::id_t _id, int _vertexNo, Vector3r _pos, Real _coord): id(_id), vertexNo(_vertexNo), coord(_coord), pos(_pos){index=3*id+vertexNo; isLowestIndex=true; vertexId=-1;}
		Body::id_t id;
		int vertexNo;
		Real coord;
		Vector3r pos;
		size_t index;
		bool isLowestIndex;
		vector<shared_ptr<VertexData> > nextIdentical;
		long vertexId;
	};
	struct VertexComparator{
		bool operator()(const shared_ptr<VertexData>& v1, const shared_ptr<VertexData>& v2){return v1->coord<v2->coord;}
	};
	struct VertexIndexComparator{
		bool operator()(const shared_ptr<VertexData>& v1, const shared_ptr<VertexData>& v2){return v1->index<v2->index;}
	};
	struct FacetTopology{
		FacetTopology(Body::id_t _id): id(_id){vertices[0]=vertices[1]=vertices[2]=-1;}
		long vertices[3];
		Body::id_t id;
		long minVertex(){return min(vertices[0],min(vertices[1],vertices[2]));}
		long maxVertex(){return max(vertices[0],max(vertices[1],vertices[2]));}
		struct MinVertexComparator{
			bool operator()(const shared_ptr<FacetTopology>& t1, const shared_ptr<FacetTopology>& t2){ return t1->minVertex()<t2->minVertex();}
		};
	};
	public:
		Vector3r projectionAxis;
		Real relTolerance;
		long commonEdgesFound;
		long commonVerticesFound;
		
		void action();
		
		// Explicit constructor with initial values
		FacetTopologyAnalyzer(): projectionAxis(Vector3r::UnitX()), relTolerance(1e-4), commonEdgesFound(0), commonVerticesFound(0) {}
		
		// Use REGISTER_ATTRIBUTES for serialization
		REGISTER_ATTRIBUTES(GlobalEngine, projectionAxis, relTolerance, commonEdgesFound, commonVerticesFound);
		
	DECLARE_LOGGER;

		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};

REGISTER_SERIALIZABLE_BASE(FacetTopologyAnalyzer, GlobalEngine);