// © 2008 Václav Šmilauer <eudoxos@arcig.cz>

#pragma once

#include<sudodem/lib/base/Math.hpp>
#include<pybind11/pybind11.h>
#include<cmath>

using std::vector;
using std::string;

// Simple normal distribution helper
struct NormalDistribution {
	Real mean;
	Real stddev;
	
	NormalDistribution(Real m, Real s) : mean(m), stddev(s) {}
	
	Real pdf(Real x) const {
		Real coeff = 1.0 / (stddev * std::sqrt(2 * M_PI));
		Real exponent = -0.5 * std::pow((x - mean) / stddev, 2);
		return coeff * std::exp(exponent);
	}
	
	Real cdf(Real x) const {
		Real z = (x - mean) / stddev;
		return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
	}
};

template<typename T>
struct GridContainer{
	private:
		Vector2r lo, hi;
		Vector2r cellSizes;
		Vector2i nCells;
	public:
		Vector2r getLo(){return lo;}
		Vector2r getHi(){return hi;}
		Vector2r getCellSize(){ return cellSizes; }
		vector<vector<vector<T> > > grid;
	/* construct grid from lower left corner, upper right corner and number of cells in each direction */
	GridContainer(Vector2r _lo, Vector2r _hi, Vector2i _nCells): lo(_lo), hi(_hi), nCells(_nCells){
		cellSizes=Vector2r((hi[0]-lo[0])/nCells[0],(hi[1]-lo[1])/nCells[1]);
		grid.resize(nCells[0]); for(int i=0; i<nCells[0]; i++) grid[i].resize(nCells[1]);
	}
	/* turn spatial coordinates into cell coordinates; if inGrid is not given, out-of-grid throws; otherwise, it sets inGrid=false. */
	Vector2i xy2cell(Vector2r xy, bool* inGrid=NULL) const {
		Vector2i ret((int)(floor((xy[0]-lo[0])/cellSizes[0])),(int)(floor((xy[1]-lo[1])/cellSizes[1])));
		if(ret[0]<0 || ret[0]>=nCells[0] || ret[1]<0 || ret[1]>=nCells[1]){
			if(inGrid) *inGrid=false; else throw std::invalid_argument("Cell coordinates outside grid (xy="+std::to_string(xy[0])+","+std::to_string(xy[1])+", computed cell coordinates "+std::to_string(ret[0])+","+std::to_string(ret[1])+").");
		} else {if(inGrid) *inGrid=true;}
		return ret;
	}
	Vector2r cell2xyMid(Vector2i cxy) const { return Vector2r(lo[0]+cellSizes[0]*(.5+cxy[0]),lo[1]+cellSizes[1]*(.5+cxy[1])); }
	const Vector2i& getSize() const{ return nCells;}

	// add new element to the right cell
	void add(const T& t, Vector2r xy){bool inGrid; Vector2i cxy=xy2cell(xy,&inGrid); if(!inGrid){ if(cxy[0]<0) cxy[0]=0; if(cxy[0]>=nCells[0]) cxy[0]=nCells[0]-1; if(cxy[1]<0) cxy[1]=0; if(cxy[1]>=nCells[1]) cxy[1]=nCells[1]-1; } grid[cxy[0]][cxy[1]].push_back(t);}

	/* Filters: return list of cells, based on some spatial criterion: rectangle, ellipse, circle (add more if you need that) */
	vector<Vector2i> rectangleFilter(Vector2r bbLo, Vector2r bbHi) const {
		vector<Vector2i> ret; bool dummy; Vector2i cxymin=xy2cell(bbLo,&dummy), cxymax=xy2cell(bbHi,&dummy);
		for(int cx=cxymin[0]; cx<=cxymax[0]; cx++){ for(int cy=cxymin[1]; cy<=cxymax[1]; cy++){ if(cx>=0 && cx<nCells[0] && cy>=0 && cy<nCells[1]) ret.push_back(Vector2i(cx,cy));	} }
		return ret;
	}
	vector<Vector2i> circleFilter(Vector2r xy, Real radius) const { return ellipseFilter(xy,Vector2r(radius,radius));}
	vector<Vector2i> ellipseFilter(Vector2r xy0, Vector2r radii) const {
		vector<Vector2i> rectangle=rectangleFilter(Vector2r(xy0[0]-radii[0],xy0[1]-radii[1]),Vector2r(xy0[0]+radii[0],xy0[1]+radii[1]));
		vector<Vector2i> ret; bool inGrid;
		Vector2i cxy=xy2cell(xy0,&inGrid);
		for (Vector2i mid : rectangle){
			// if we are in the cell where the middle is also, this cell passes the filter
			if(inGrid && mid[0]==cxy[0] && mid[1]==cxy[1]){ret.push_back(mid); continue;}
			Vector2r xyMid=cell2xyMid(mid);
			Vector2r xy(xyMid[0]-xy0[0],xyMid[1]-xy0[1]); // relative mid-cell coords
			// tricky: move the mid-cell to the corner (or aligned pt on edge) according to position WRT center
			if(mid[0]!=cxy[0]) xy.x()+=(mid[0]<cxy[0]?1:-1)*.5*cellSizes[0]; else xy.x()=0;
			if(mid[1]!=cxy[1]) xy.x()+=(mid[1]<cxy[1]?1:-1)*.5*cellSizes[1]; else xy.y()=0;
			// are we inside the ellipse? pass the filter, then
			if((pow(xy[0],2)/pow(radii[0],2)+pow(xy[1],2)/pow(radii[1],2))<=1) ret.push_back(mid);
		}
		return ret;
	}

	// graphical representation of a set of filtered cells
	string dumpGrid(vector<Vector2i> v){
		vector<vector<bool> > vvb; string ret; vvb.resize(nCells[0]); for(size_t i=0; i<(size_t)nCells[0]; i++) vvb[i].resize(nCells[1],false); for (Vector2i& vv : v) vvb[vv[0]][vv[1]]=true;
		for(int cy=nCells[1]-1; cy>=0; cy--){ ret+="|"; for(int cx=0; cx<nCells[0]; cx++){ ret+=vvb[cx][cy]?"@":"."; }	ret+="|\n";	}
		return ret;
	}
};
/* Abstract class for averages templated by elements stored in the grid and by the type of average value.
 *
 * To define your own class, you need to override getPosition (returns exact position of an element),
 * getWeight (return weight associated with given point and element -- may be 0, but should not be negative),
 * getValue (for an element),and filterCells (returns vector of cells of which elements will be used in computation).
 *
 * computeAverage computes weighted average using the overridden methods.
 */
template<typename T, typename Tvalue>
struct WeightedAverage{
	const shared_ptr<GridContainer<T> > grid;
	Real weightedSupportArea; // must be computed by derived class
	WeightedAverage(const shared_ptr<GridContainer<T> >& _grid):grid(_grid){};
	virtual Vector2r getPosition(const T&)=0;
	virtual Real getWeight(const Vector2r& refPt, const T&)=0;
	virtual Tvalue getValue(const T&)=0;
	virtual vector<Vector2i> filterCells(const Vector2r& refPt)=0;
	Tvalue computeAverage(const Vector2r& refPt){
		Real sumValues, sumWeights; sumValuesWeights(refPt,sumValues,sumWeights);
		return sumValues/sumWeights;
	}
	Tvalue computeAvgPerUnitArea(const Vector2r& refPt){
		Real sumValues, sumWeights; sumValuesWeights(refPt,sumValues,sumWeights);
		return sumValues/weightedSupportArea;
	}
	void sumValuesWeights(const Vector2r& refPt, Real& sumValues, Real& sumWeights){
		vector<Vector2i> filtered=filterCells(refPt);
		sumValues=sumWeights=0;
		for (Vector2i cell : filtered){
			for (const T& element : grid->grid[cell[0]][cell[1]]){
				Real weight=getWeight(refPt,element);
				sumValues+=weight*getValue(element); sumWeights+=weight;
			}
		}
	}
};

/* Class for doing template specialization of gaussian kernel average on SGDA_Scalar2d and for testing */
struct Scalar2d{
	Vector2r pos;
	Real val;
};

/* Symmetric Gaussian Distribution Average with scalar values
 */
struct SGDA_Scalar2d: public WeightedAverage<Scalar2d,Real> {
	Real stDev, relThreshold; 
	NormalDistribution distrib;
	SGDA_Scalar2d(const shared_ptr<GridContainer<Scalar2d> >& _grid, Real _stDev, Real _relThreshold=3): WeightedAverage<Scalar2d,Real>(_grid), stDev(_stDev), relThreshold(_relThreshold), distrib(0,stDev) {
		// approximate probability density function between -stDev*relThreshold,stDev*relThreshold
		// it is enough to get Φ(-stDev*relThreshold) and subtract twice from 1 (symmetry)
		// http://en.wikipedia.org/wiki/Normal_distribution#Numerical_approximations_for_the_normal_CDF
		// using Abramowitz & Stegun approximation, which has error less that 7.5e-8 (fine for us)

		// FIXME: algorithm not correct, as it takes 1d quantile, while we would need PDF for symmetric 2d gaussian!
		Real clippedQuantile=distrib.cdf(-stDev*relThreshold);
		Real area=M_PI*pow(relThreshold*stDev,2); // area of the support
		weightedSupportArea=(1-2*clippedQuantile)*area;

	}
	virtual Real getWeight(const Vector2r& meanPt, const Scalar2d& e){
		Vector2r pos=getPosition(e);
		Real rSq=(meanPt-pos).squaredNorm(); //pow(meanPt[0]-pos[0],2)+pow(meanPt[1]-pos[1],2);
		if(rSq>pow(relThreshold*stDev,2)) return 0.; // discard points further than relThreshold*stDev, by default 3*stDev
		//return (1./(stDev*sqrt(2*M_PI)))*exp(-rSq/(2*stDev*stDev));
		return distrib.pdf(sqrt(rSq));
	}
	vector<Vector2i> filterCells(const Vector2r& refPt){return WeightedAverage<Scalar2d,Real>::grid->circleFilter(refPt,stDev*relThreshold);}
	Real getValue(const Scalar2d& dp){return (Real)dp.val;}
	Vector2r getPosition(const Scalar2d& dp){return dp.pos;}
};

/* simplified interface for python:
 *
 * ga=GaussAverage((0,0),(100,100),(10,10),5)
 * ga.add(53.444,(15.6,43.0))
 * ...
 * ga.avg((10,10))
 * ...
 *
 * */
class pyGaussAverage{
	//struct Scalar2d{Vector2r pos; Real val;};
	Vector2r tuple2vec2r(const pybind11::tuple& t){return Vector2r(t[0].cast<Real>(),t[1].cast<Real>());}
	Vector2i tuple2vec2i(const pybind11::tuple& t){return Vector2i(t[0].cast<int>(),t[1].cast<int>());}
	shared_ptr<SGDA_Scalar2d> sgda;
	struct Poly2d{vector<Vector2r> vertices; bool inclusive;};
	vector<Poly2d> clips;
	public:
	pyGaussAverage(pybind11::tuple lo, pybind11::tuple hi, pybind11::tuple nCells, Real stDev, Real relThreshold=3.){
		shared_ptr<GridContainer<Scalar2d> > g(new GridContainer<Scalar2d>(tuple2vec2r(lo),tuple2vec2r(hi),tuple2vec2i(nCells)));
		sgda=shared_ptr<SGDA_Scalar2d>(new SGDA_Scalar2d(g,stDev));
		sgda->relThreshold=relThreshold;
	}
	bool pointInsidePolygon(const Vector2r&,const vector<Vector2r>&);
	bool ptIsClipped(const Vector2r& pt){
		for (const Poly2d& poly : clips){
			bool inside=pointInsidePolygon(pt,poly.vertices);
			if((inside && !poly.inclusive) || (!inside && poly.inclusive)) return true;
		}
		return false;
	}
	bool addPt(Real val, pybind11::tuple pos){Scalar2d d; d.pos=tuple2vec2r(pos); if(ptIsClipped(d.pos)) return false; d.val=val; sgda->grid->add(d,d.pos); return true; }
	Real avg(Vector2r pt){ if(ptIsClipped(pt)) return std::numeric_limits<Real>::quiet_NaN(); return sgda->computeAverage(pt);}
	Real avgPerUnitArea(Vector2r pt){ if(ptIsClipped(pt)) return std::numeric_limits<Real>::quiet_NaN(); return sgda->computeAvgPerUnitArea(pt); }
	Real stDev_get(){return sgda->stDev;} void stDev_set(Real s){sgda->stDev=s;}
	Real relThreshold_get(){return sgda->relThreshold;} void relThreshold_set(Real rt){sgda->relThreshold=rt;}
	pybind11::tuple aabb_get(){return pybind11::make_tuple(sgda->grid->getLo(),sgda->grid->getHi());}
	pybind11::list clips_get(){
		pybind11::list ret;
		for (const Poly2d& poly : clips){
			pybind11::list vertices;
			for (const Vector2r& v : poly.vertices) vertices.append(pybind11::make_tuple(v[0],v[1]));
			ret.append(pybind11::make_tuple(vertices,poly.inclusive));
		}
		return ret;
	}
	void clips_set(pybind11::list l){
		/* [ ( [(x1,y1),(x2,y2),…], true), … ] */
		clips.clear();
		for(int i=0; i<pybind11::len(l); i++){
			pybind11::tuple polyDesc=l[i].cast<pybind11::tuple>();
			pybind11::list coords=polyDesc[0].cast<pybind11::list>();
			Poly2d poly; poly.inclusive=polyDesc[1].cast<bool>();
			for(int j=0; j<pybind11::len(coords); j++){
				poly.vertices.push_back(tuple2vec2r(coords[j].cast<pybind11::tuple>()));
			}
			clips.push_back(poly);
		}
	}
	pybind11::tuple data_get(){
		pybind11::list x,y,val;
		const Vector2i& dim=sgda->grid->getSize();
		for(int i=0; i<dim[0]; i++){
			for(int j=0; j<dim[1]; j++){
				for (const Scalar2d& element : sgda->grid->grid[i][j]){
					x.append(element.pos[0]); y.append(element.pos[1]); val.append(element.val);
				}
			}
		}
		return pybind11::make_tuple(x,y,val);
	}
	Vector2i nCells_get(){ return sgda->grid->getSize(); }
	int cellNum(const Vector2i& cell){ return sgda->grid->grid[cell[0]][cell[1]].size(); }
	Real cellSum(const Vector2i& cell){ Real sum=0; for (const Scalar2d& v : sgda->grid->grid[cell[0]][cell[1]]) sum+=v.val; return sum; }
	Real cellAvg(const Vector2i& cell){ return cellSum(cell)/cellNum(cell); }
	Real cellArea(){ Vector2r sz=sgda->grid->getCellSize(); return sz[0]*sz[1]; }
	Vector2r cellDim(){ return sgda->grid->getCellSize(); }
};