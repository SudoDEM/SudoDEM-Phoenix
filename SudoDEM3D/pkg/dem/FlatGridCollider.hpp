// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#include<sudodem/pkg/common/Collider.hpp>
class NewtonIntegrator;

class FlatGridCollider: public Collider{
	struct Grid{
		typedef std::vector<Body::id_t> idVector;
		Vector3i size;
		Vector3r mn, mx;
		Real step;
		// convert point into its integral coordinates (can be outside grid, use fitGrid to coords inside)
		Vector3i pt2int(const Vector3r& pt){ Vector3i ret; for(int i=0;i<3;i++)ret[i]=floor((pt[i]-mn[1])/step); return ret; }
		std::vector<idVector> data;
		// force integral coordinate inside (0,sz-1)
		int fit(int i, int sz) const { return max(0,min(i,sz-1)); }
		Vector3i fitGrid(const Vector3i& v){ return Vector3i(fit(v[0],size[0]),fit(v[1],size[1]),fit(v[2],size[2])); }
		// linearize x,y,z → n in data vector
		size_t lin(int x, int y, int z) const { return fit(x,size[0])+size[0]*fit(y,size[1])+size[0]*size[1]*fit(z,size[2]); }
		// return vector of ids at (x,y,z)
		idVector& operator()(int x, int y, int z){ return data[lin(x,y,z)];}
		idVector& operator()(const Vector3i& v){ return data[lin(v[0],v[1],v[2])];}
		const idVector& operator()(int x, int y, int z) const { return data[lin(x,y,z)];}
	};
	
	Grid grid;
	int sphereIdx;
	int facetIdx;
	int wallIdx;
	int boxIdx;
	shared_ptr<NewtonIntegrator> newton;
	Real fastestBodyMaxDist;
	Real verletDist;
	Vector3r aabbMin;
	Vector3r aabbMax;
	Real step;
	
	void initIndices();
	void updateGrid();
	void updateBodyCells(const shared_ptr<Body>& b);
	void updateCollisions();
	virtual void action();
	virtual bool isActivated();
	DECLARE_LOGGER;

public:
	// Explicit constructor with initial values
	FlatGridCollider(): sphereIdx(0), facetIdx(0), wallIdx(0), boxIdx(0), fastestBodyMaxDist(0.), verletDist(0.), step(0.) {
		initIndices();
	}
	
	// Use REGISTER_ATTRIBUTES for serialization
	REGISTER_ATTRIBUTES(Collider, verletDist, aabbMin, aabbMax, step);
	
	DECLARE_LOGGER;

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE_BASE(FlatGridCollider, Collider);