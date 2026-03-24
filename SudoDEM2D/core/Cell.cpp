
#include<sudodem/core/Cell.hpp>

CREATE_LOGGER(Cell);

void Cell::integrateAndUpdate(Real dt){
	//incremental displacement gradient
	_trsfInc=dt*velGrad;
	// total transformation; M = (Id+G).M = F.M
	trsf+=_trsfInc*trsf;
	_invTrsf=trsf.inverse();
	// hSize contains colums with updated base vectors
	prevHSize=hSize;
	_vGradTimesPrevH = velGrad*prevHSize;
	hSize+=_trsfInc*hSize;
	if(hSize.determinant()==0){ throw runtime_error("Cell is degenerate (zero volume)."); }
	// lengths of transformed cell vectors, skew cosines
	Matrix2r Hnorm; // normalized transformed base vectors
	for(int i=0; i<2; i++){
		Vector2r base(hSize.col(i));
		_size[i]=base.norm(); base/=_size[i]; //base is normalized now
		Hnorm(0,i)=base[0]; Hnorm(1,i)=base[1];};
	// skew cosines
		// sin between axes is cos of skew
		//_cos=(Hnorm.col(0).cross(Hnorm.col(1))).squaredNorm();
		//_cos=1 - pow(Hnorm.col(0).dot(Hnorm.col(1)),2);
		_cos=sqrt(1.0 - pow(Hnorm.col(0).dot(Hnorm.col(1)),2));//sin of the angle between the two basises
	// pure shear trsf: ones on diagonal
	_shearTrsf=Hnorm;
	// pure unshear transformation
	_unshearTrsf=_shearTrsf.inverse();
	// some parts can branch depending on presence/absence of shear
	_hasShear=(hSize(0,1)!=0 || hSize(1,0)!=0);
	// OpenGL shear matrix (used frequently)
	fillGlShearTrsfMatrix(_glShearTrsfMatrix);
}

void Cell::fillGlShearTrsfMatrix(double m[16]){
	m[0]=_shearTrsf(0,0); m[4]=_shearTrsf(0,1); m[8]=0; m[12]=0;
	m[1]=_shearTrsf(1,0); m[5]=_shearTrsf(1,1); m[9]=0; m[13]=0;
	m[2]=0;               m[6]=0;               m[10]=0;m[14]=0;
	m[3]=0;               m[7]=0;               m[11]=0;m[15]=1;
}

void Cell::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Cell");
	pybind11::class_<Cell, Serializable, std::shared_ptr<Cell>> _classObj(_module, "Cell", "Periodic boundary conditions parameters");
	_classObj.def(pybind11::init<>());
	// basic attributes
	_classObj.def_readonly("trsf", &Cell::trsf, "[overridden]");
	_classObj.def_readwrite("refHSize", &Cell::refHSize, "Reference cell configuration, only used with OpenGLRenderer.dispScale. Updated automatically when hSize or trsf is assigned directly; also modified by sudodem.utils.setRefPos.");
	_classObj.def_property("hSize", &Cell::getHSize, &Cell::setHSize, "Base cell vectors (columns of the matrix), updated at every step from velGrad (trsf accumulates applied velGrad transformations). Setting hSize during a simulation is not supported by most contact laws, it is only meant to be used at iteration 0 before any interactions have been created.");
	_classObj.def_readonly("prevHSize", &Cell::prevHSize, "hSize from the previous step, used in the definition of relative velocity across periods.");
	_classObj.def_property("velGrad", &Cell::getVelGrad, &Cell::setVelGrad, "Velocity gradient of the transformation; used in NewtonIntegrator. Values of velGrad accumulate in trsf at every step. NOTE: changing velGrad at the beginning of a simulation loop would lead to inaccurate integration for one step, as it should normally be changed after the contact laws (but before Newton). To avoid this problem, assignment is deferred automatically. The target value typed in terminal is actually stored in nextVelGrad and will be applied right in time by Newton integrator.");
	_classObj.def_readonly("nextVelGrad", &Cell::nextVelGrad, "see Cell.velGrad.");
	_classObj.def_readonly("prevVelGrad", &Cell::prevVelGrad, "Velocity gradient in the previous step.");
	_classObj.def_readwrite("homoDeform", &Cell::homoDeform, "Deform (velGrad) the cell homothetically, by adjusting positions and velocities of bodies. The velocity change is obtained by deriving the expression v=∇v.x, where ∇v is the macroscopic velocity gradient, giving in an incremental form: Δv=Δ ∇v x + ∇v Δx. As a result, velocities are modified as soon as velGrad changes, according to the first term: Δv(t)=Δ ∇v x(t), while the 2nd term reflects a convective term: Δv'= ∇v v(t-dt/2).");
	_classObj.def_readonly("velGradChanged", &Cell::velGradChanged, "true when velGrad has been changed manually (see also Cell.nextVelGrad)");
	// override some attributes above
	_classObj.def_property("size", &Cell::getSize_copy, &Cell::setSize, "Current size of the cell, i.e. lengths of the cell lateral vectors contained in Cell.hSize columns. Updated automatically at every step. Assigning a value will change the lengths of base vectors (see Cell.hSize), keeping their orientations unchanged.");
	_classObj.def_property("refSize", &Cell::getRefSize, &Cell::setRefSize, "Reference size of the cell (lengths of initial cell vectors, i.e. column norms of hSize).\\n\\n.. note::\\n\\t Modifying this value is deprecated, use Cell.setBox instead.");
	// useful properties
	_classObj.def_property_readonly("size", &Cell::getSize_copy, "Current size of the cell, i.e. lengths of the cell lateral vectors contained in Cell.hSize columns. Updated automatically at every step.");
	_classObj.def_property_readonly("volume", &Cell::getVolume, "Current volume of the cell.");
	// functions
	_classObj.def("setBox", &Cell::setBox, "Set Cell shape to be rectangular, with dimensions along axes specified by given argument. Shorthand for assigning diagonal matrix with respective entries to hSize.");
	// debugging only
	_classObj.def("wrap", &Cell::wrapShearedPt_py, "Transform an arbitrary point into a point in the reference cell");
	_classObj.def("unshearPt", &Cell::unshearPt, "Apply inverse shear on the point (removes skew+rot of the cell)");
	_classObj.def("shearPt", &Cell::shearPt, "Apply shear (cell skew+rot) on the point");
	_classObj.def("wrapPt", &Cell::wrapPt_py, "Wrap point inside the reference cell, assuming the cell has no skew+rot.");
	_classObj.def("getDefGrad", &Cell::getDefGrad, "Returns deformation gradient tensor F of the cell deformation (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getSmallStrain", &Cell::getSmallStrain, "Returns small strain tensor ε=1/2(F+F^T)-I of the cell (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getRCauchyGreenDef", &Cell::getRCauchyGreenDef, "Returns right Cauchy-Green deformation tensor C=F^T F of the cell (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getLCauchyGreenDef", &Cell::getLCauchyGreenDef, "Returns left Cauchy-Green deformation tensor b=F F^T of the cell (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getLagrangianStrain", &Cell::getLagrangianStrain, "Returns Lagrangian strain tensor E=1/2(C-I)=1/2(F^T F-I)=1/2(U^2-I) of the cell (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getEulerianAlmansiStrain", &Cell::getEulerianAlmansiStrain, "Returns Eulerian-Almansi strain tensor e=1/2(I-b^{-1})=1/2(I-(F F^T)^{-1}) of the cell (http://en.wikipedia.org/wiki/Finite_strain_theory)");
	_classObj.def("getPolarDecOfDefGrad", &Cell::getPolarDecOfDefGrad, "Returns orthogonal matrix R and symmetric positive semi-definite matrix U as polar decomposition of deformation gradient F of the cell ( F=RU )");
	_classObj.def("getRotation", &Cell::getRotation, "Returns rotation of the cell (orthogonal matrix R from polar decomposition F=RU )");
	_classObj.def("getLeftStretch", &Cell::getLeftStretch, "Returns left (spatial) stretch tensor of the cell (matrix U from polar decomposition F=RU )");
	_classObj.def("getRightStretch", &Cell::getRightStretch, "Returns right (material) stretch tensor of the cell (matrix V from polar decomposition F=RU=VR -> V=FR^T )");
	_classObj.def_property_readonly("shearTrsf", [](const Cell& c) -> const Matrix2r& { return c._shearTrsf; }, "Current skew+rot transformation (no resize)");
	_classObj.def_property_readonly("unshearTrsf", [](const Cell& c) -> const Matrix2r& { return c._unshearTrsf; }, "Inverse of the current skew+rot transformation (no resize)");
	_classObj.def_property_readonly("hSize0", [](const Cell& c) -> Matrix2r { return c.getHSize0(); }, "Value of untransformed hSize, with respect to current trsf (computed as trsf^{-1} × hSize.");
}
