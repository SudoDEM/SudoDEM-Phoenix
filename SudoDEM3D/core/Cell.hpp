// 2009 © Václav Šmilauer <eudoxos@arcig.cz>

/*! Periodic cell parameters and routines. Usually instantiated as Scene::cell.

The Cell has current box configuration represented by the parallelepiped's base vectors (*hSize*). Lengths of the base vectors can be accessed via *size*.

* Matrix3r *trsf* is "deformation gradient tensor" F (http://en.wikipedia.org/wiki/Finite_strain_theory)
* Matrix3r *velGrad* is "velocity gradient tensor" (http://www.cs.otago.ac.nz/postgrads/alexis/FluidMech/node7.html)

The transformation is split between "normal" part and "rotation/shear" part for contact detection algorithms. The shearPt, unshearPt, getShearTrsf etc functions refer to both shear and rotation. This decomposition is frame-dependant and does not correspond to the rotation/stretch decomposition of mechanics (with stretch further decomposed into isotropic and deviatoric). Therefore, using the shearPt family in equations of mechanics is not recommended. Similarly, attributes assuming the existence of a "reference" state are considered deprecated (refSize, hSize0). It is better to not use them since there is no guarantee that the so-called "reference state" will be maintained consistently in the future.

*/

#pragma once
#include<sudodem/lib/base/Logging.hpp>
#include<sudodem/lib/base/Math.hpp>
#include<sudodem/lib/serialization/Serializable.hpp>
#include<pybind11/pybind11.h>
#include<pybind11/stl.h>

class Cell: public Serializable{
	public:
	//! Get/set sizes of cell base vectors
	const Vector3r& getSize() const { return _size; }
	void setSize(const Vector3r& s){for (int k=0;k<3;k++) hSize.col(k)*=s[k]/hSize.col(k).norm(); refHSize=hSize;  postLoad(*this);}
	//! Return copy of the current size (used only by the python wrapper)
	Vector3r getSize_copy() const { return _size; }
	//! return vector of consines of skew angle in xy plane between transformed base vectors
	const Real& getCos() const {return _cos;}
	//! transformation matrix applying pure shear&rotation (scaling removed)
	const Matrix3r& getShearTrsf() const { return _shearTrsf; }
	//! inverse of getShearTrsfMatrix().
	const Matrix3r& getUnshearTrsf() const {return _unshearTrsf;}

	const double* getGlShearTrsfMatrix() const { return _glShearTrsfMatrix; }
	//! Whether any shear (non-diagonal) component of the strain matrix is nonzero.
	bool hasShear() const {return _hasShear; }

	// caches; private
	private:
		Matrix3r _invTrsf;
		Matrix3r _trsfInc;
		Matrix3r _vGradTimesPrevH;
		Vector3r _size;
		Real _cos;
		Vector3r _refSize;
		bool _hasShear;
		Matrix3r _shearTrsf, _unshearTrsf;
		double _glShearTrsfMatrix[16];
		void fillGlShearTrsfMatrix(double m[16]);
	public:

	DECLARE_LOGGER;

	//! "integrate" velGrad, update cached values used by public getter.
	void integrateAndUpdate(Real dt);
	/*! Return point inside periodic cell, even if shear is applied */
	Vector3r wrapShearedPt(const Vector3r& pt) const { return shearPt(wrapPt(unshearPt(pt))); }
	/*! Return point inside periodic cell, even if shear is applied; store cell coordinates in period. */
	Vector3r wrapShearedPt(const Vector3r& pt, Vector3i& period) const { return shearPt(wrapPt(unshearPt(pt),period)); }
	/*! Apply inverse shear on point; to put it inside (unsheared) periodic cell, apply wrapPt on the returned value. */
	Vector3r unshearPt(const Vector3r& pt) const { return _unshearTrsf*pt; }
	//! Apply shear on point.
	Vector3r shearPt(const Vector3r& pt) const { return _shearTrsf*pt; }
	/*! Wrap point to inside the periodic cell; don't compute number of periods wrapped */
	Vector3r wrapPt(const Vector3r& pt) const {
		Vector3r ret; for(int i=0;i<3;i++) ret[i]=wrapNum(pt[i],_size[i]); return ret;}
	/*! Wrap point to inside the periodic cell; period will contain by how many cells it was wrapped. */
	Vector3r wrapPt(const Vector3r& pt, Vector3i& period) const {
		Vector3r ret; for(int i=0; i<3; i++){ ret[i]=wrapNum(pt[i],_size[i],period[i]); } return ret;}
	/*! Wrap number to interval 0…sz */
	static Real wrapNum(const Real& x, const Real& sz) {
		Real norm=x/sz; return (norm-floor(norm))*sz;}
	/*! Wrap number to interval 0…sz; store how many intervals were wrapped in period */
	static Real wrapNum(const Real& x, const Real& sz, int& period) {
		Real norm=x/sz; period=(int)floor(norm); return (norm-period)*sz;}

	// relative position and velocity for interaction accross multiple cells
	Vector3r intrShiftPos(const Vector3i& cellDist) const { return hSize*cellDist.cast<Real>(); }
	Vector3r intrShiftVel(const Vector3i& cellDist) const { return _vGradTimesPrevH*cellDist.cast<Real>(); }
	// return body velocity while taking away mean field velocity (coming from velGrad) if the mean field velocity is applied on velocity
	Vector3r bodyFluctuationVel(const Vector3r& pos, const Vector3r& vel, const Matrix3r& prevVelGrad) const { return (vel-prevVelGrad*pos); }

	// get/set current shape; setting resets trsf to identity
	Matrix3r getHSize() const { return hSize; }
	void setHSize(const Matrix3r& m){ hSize=refHSize=m; postLoad(*this); }
	// set current transformation; has no influence on current configuration (hSize); sets display refHSize as side-effect
	Matrix3r getTrsf() const { return trsf; }
	void setTrsf(const Matrix3r& m){ trsf=m; postLoad(*this); }
	Matrix3r getVelGrad() const { return velGrad; }
	void setVelGrad(const Matrix3r& m){ nextVelGrad=m; velGradChanged=true;}
	//BEGIN Deprecated (see refSize property)
	// get undeformed shape
	Matrix3r getHSize0() const { return _invTrsf*hSize; }
	// edge lengths of the undeformed shape
	Vector3r getRefSize() const { Matrix3r h=getHSize0(); return Vector3r(h.col(0).norm(),h.col(1).norm(),h.col(2).norm()); }
	// temporary, will be removed in favor of more descriptive setBox(...)
	void setRefSize(const Vector3r& s){
		// if refSize is set to the current size and the cell is a box (used in older scripts), say it is not necessary
		Matrix3r hSizeEigen3=hSize.diagonal().asDiagonal();
		if(s==_size && hSize==hSizeEigen3){ LOG_WARN("Setting O.cell.refSize=O.cell.size is useless, O.trsf=Matrix3.Identity is enough now."); }
		else {LOG_WARN("Setting Cell.refSize is deprecated, use Cell.setBox(...) instead.");}
		setBox(s); postLoad(*this);
	}
	//END Deprecated
	// set box shape of the cell
	void setBox(const Vector3r& size){ setHSize(size.asDiagonal()); trsf=Matrix3r::Identity(); postLoad(*this); }

	// return current cell volume
	Real getVolume() const {return hSize.determinant();}
	void postLoad(Cell&){ integrateAndUpdate(0); }

	// to resolve overloads
	Vector3r wrapShearedPt_py(const Vector3r& pt) const { return wrapShearedPt(pt);}
	Vector3r wrapPt_py(const Vector3r& pt) const { return wrapPt(pt);}

	// strain measures
	Matrix3r getDefGrad() { return trsf; }
	Matrix3r getSmallStrain() { return .5*(trsf+trsf.transpose()) - Matrix3r::Identity(); }
	Matrix3r getRCauchyGreenDef() { return trsf.transpose()*trsf; }
	Matrix3r getLCauchyGreenDef() { return trsf*trsf.transpose(); }
	Matrix3r getLagrangianStrain() { return .5*(getRCauchyGreenDef()-Matrix3r::Identity()); }
	Matrix3r getEulerianAlmansiStrain() { return .5*(Matrix3r::Identity()-getLCauchyGreenDef().inverse()); }
	void computePolarDecOfDefGrad(Matrix3r& R, Matrix3r& U) { Matrix_computeUnitaryPositive(trsf,&R,&U); }
	pybind11::tuple getPolarDecOfDefGrad(){ Matrix3r R,U; computePolarDecOfDefGrad(R,U); return pybind11::make_tuple(R,U); }
	Matrix3r getRotation() { Matrix3r R,U; computePolarDecOfDefGrad(R,U); return R; }
	Matrix3r getLeftStretch() { Matrix3r R,U; computePolarDecOfDefGrad(R,U); return U; }
	Matrix3r getRightStretch() { Matrix3r R,U; computePolarDecOfDefGrad(R,U); return trsf*R.transpose(); }

	enum { HOMO_NONE=0, HOMO_POS=1, HOMO_VEL=2, HOMO_VEL_2ND=3 };
	Matrix3r trsf = Matrix3r::Identity();
	Matrix3r refHSize = Matrix3r::Identity();
	Matrix3r hSize = Matrix3r::Identity();
	Matrix3r prevHSize = Matrix3r::Identity();
	Matrix3r velGrad = Matrix3r::Zero();
	Matrix3r nextVelGrad = Matrix3r::Zero();
	Matrix3r prevVelGrad = Matrix3r::Zero();
	bool homoDeform = true;
	bool velGradChanged = false;

	Cell() { _invTrsf = Matrix3r::Identity(); integrateAndUpdate(0); }

	// Cereal serialization
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int version) {
		ar(cereal::base_class<Serializable>(this));
		ar(CEREAL_NVP(trsf));
		ar(CEREAL_NVP(refHSize));
		ar(CEREAL_NVP(hSize));
		ar(CEREAL_NVP(prevHSize));
		ar(CEREAL_NVP(velGrad));
		ar(CEREAL_NVP(nextVelGrad));
		ar(CEREAL_NVP(prevVelGrad));
		ar(CEREAL_NVP(homoDeform));
		ar(CEREAL_NVP(velGradChanged));
	}

	SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE(Cell);
