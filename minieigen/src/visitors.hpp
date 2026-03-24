#pragma once
#include"common.hpp"

// Helper to convert Eigen expression to concrete type
template<typename T, typename Expr>
inline T to_concrete(const Expr& expr) {
	return T(expr);
}

// MatrixBaseVisitor - base functionality for vectors and matrices
template<typename MatrixBaseT>
class MatrixBaseVisitor {
	typedef typename MatrixBaseT::Scalar Scalar;
	typedef typename MatrixBaseT::RealScalar RealScalar;
public:
	static void expose(pybind11::class_<MatrixBaseT>& cls) {
		cls
			.def("__neg__", [](const MatrixBaseT& a){ return to_concrete<MatrixBaseT>(-a); })
			.def("__add__", [](const MatrixBaseT& a, const MatrixBaseT& b){ return to_concrete<MatrixBaseT>(a + b); })
			.def("__iadd__", [](MatrixBaseT& a, const MatrixBaseT& b){ a += b; return a; })
			.def("__sub__", [](const MatrixBaseT& a, const MatrixBaseT& b){ return to_concrete<MatrixBaseT>(a - b); })
			.def("__isub__", [](MatrixBaseT& a, const MatrixBaseT& b){ a -= b; return a; })
			.def("__eq__", [](const MatrixBaseT& a, const MatrixBaseT& b){
				if(a.rows()!=b.rows() || a.cols()!=b.cols()) return false;
				return a.cwiseEqual(b).all();
			})
			.def("__ne__", [](const MatrixBaseT& a, const MatrixBaseT& b){ return !(a == b); })
			.def("__mul__", [](const MatrixBaseT& a, long s){ return to_concrete<MatrixBaseT>(a * static_cast<Scalar>(s)); })
			.def("__imul__", [](MatrixBaseT& a, long s){ a *= static_cast<Scalar>(s); return a; })
			.def("__rmul__", [](const MatrixBaseT& a, long s){ return to_concrete<MatrixBaseT>(a * static_cast<Scalar>(s)); })
			.def("isApprox", [](const MatrixBaseT& a, const MatrixBaseT& b, RealScalar prec = Eigen::NumTraits<Scalar>::dummy_precision()){
				return a.isApprox(b, prec);
			}, pybind11::arg("other"), pybind11::arg("prec") = Eigen::NumTraits<Scalar>::dummy_precision(), "Approximate comparison with precision *prec*.")
			.def("rows", [](const MatrixBaseT& m){ return m.rows(); }, "Number of rows.")
			.def("cols", [](const MatrixBaseT& m){ return m.cols(); }, "Number of columns.")
			.def("sum", [](const MatrixBaseT& m){ return m.sum(); }, "Sum of all elements.")
			.def("prod", [](const MatrixBaseT& m){ return m.prod(); }, "Product of all elements.")
			.def("mean", [](const MatrixBaseT& m){ return m.mean(); }, "Mean value over all elements.")
			.def("maxAbsCoeff", [](const MatrixBaseT& m){ return m.array().abs().maxCoeff(); }, "Maximum absolute value over all elements.");

		// Add float-specific operations
		if constexpr (!std::is_integral<Scalar>::value) {
			cls
				.def("__mul__", [](const MatrixBaseT& a, Scalar s){ return to_concrete<MatrixBaseT>(a * s); })
				.def("__rmul__", [](const MatrixBaseT& a, Scalar s){ return to_concrete<MatrixBaseT>(a * s); })
				.def("__imul__", [](MatrixBaseT& a, Scalar s){ a *= s; return a; })
				.def("__truediv__", [](const MatrixBaseT& a, long s){ return to_concrete<MatrixBaseT>(a / static_cast<Scalar>(s)); })
				.def("__itruediv__", [](MatrixBaseT& a, long s){ a /= static_cast<Scalar>(s); return a; })
				.def("__truediv__", [](const MatrixBaseT& a, Scalar s){ return to_concrete<MatrixBaseT>(a / s); })
				.def("__itruediv__", [](MatrixBaseT& a, Scalar s){ a /= s; return a; })
				.def("norm", &MatrixBaseT::norm, "Euclidean norm.")
				.def("__abs__", &MatrixBaseT::norm)
				.def("squaredNorm", &MatrixBaseT::squaredNorm, "Square of the Euclidean norm.")
				.def("normalize", &MatrixBaseT::normalize, "Normalize this object in-place.")
				.def("normalized", &MatrixBaseT::normalized, "Return normalized copy of this object")
				.def("pruned", [](const MatrixBaseT& a, double absTol = 1e-6){
					MatrixBaseT ret = MatrixBaseT::Zero(a.rows(), a.cols());
					for(Index c = 0; c < a.cols(); c++){
						for(Index r = 0; r < a.rows(); r++){
							Scalar val = a(c, r);
							if constexpr (!Eigen::NumTraits<Scalar>::IsComplex) {
								if(std::abs(val) > absTol || val == -0) ret(c, r) = val;
							} else {
								if(std::abs(val) > absTol) ret(c, r) = val;
							}
						}
					}
					return ret;
				}, pybind11::arg("absTol") = 1e-6, "Zero all elements which are greater than *absTol*. Negative zeros are not pruned.");
		}
		
		// Add maxCoeff/minCoeff for non-complex types
		if constexpr (!Eigen::NumTraits<Scalar>::IsComplex) {
			cls
				.def("maxCoeff", [](const MatrixBaseT& m){ return m.array().maxCoeff(); }, "Maximum value over all elements.")
				.def("minCoeff", [](const MatrixBaseT& m){ return m.array().minCoeff(); }, "Minimum value over all elements.");
		}
		
		// Add static properties for fixed-size types
		if constexpr (MatrixBaseT::RowsAtCompileTime != Eigen::Dynamic) {
			cls.def_property_readonly_static("Ones", [](pybind11::object){ return to_concrete<MatrixBaseT>(MatrixBaseT::Ones()); });
			cls.def_property_readonly_static("Zero", [](pybind11::object){ return to_concrete<MatrixBaseT>(MatrixBaseT::Zero()); });
			cls.def_static("Random", [](){ return to_concrete<MatrixBaseT>(MatrixBaseT::Random()); }, "Return an object where all elements are randomly set to values between 0 and 1.");
			if constexpr (MatrixBaseT::RowsAtCompileTime == MatrixBaseT::ColsAtCompileTime) {
				cls.def_property_readonly_static("Identity", [](pybind11::object){ return to_concrete<MatrixBaseT>(MatrixBaseT::Identity()); });
			}
		}
	}
};

// VectorVisitor
template<typename VectorT>
class VectorVisitor {
	typedef typename VectorT::Scalar Scalar;
	typedef Eigen::Matrix<Scalar, VectorT::RowsAtCompileTime, VectorT::RowsAtCompileTime> CompatMatrixT;
	typedef Eigen::Matrix<Scalar, 2, 1> CompatVec2;
	typedef Eigen::Matrix<Scalar, 3, 1> CompatVec3;
	typedef Eigen::Matrix<Scalar, 6, 1> CompatVec6;
	typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> CompatVecX;
	enum { Dim = VectorT::RowsAtCompileTime };
public:
	static void expose(pybind11::module_& m, const char* name, const char* doc) {
		auto cls = pybind11::class_<VectorT>(m, name, doc)
			.def(pybind11::init<>())
			.def(pybind11::pickle(
				[](const VectorT& x) -> pybind11::tuple {
					if constexpr (Dim == Eigen::Dynamic) {
						std::vector<Scalar> v(x.size());
						for(Index i = 0; i < (Index)x.size(); i++) v[i] = x[i];
						return pybind11::make_tuple(v);
					} else if constexpr (Dim == 2) {
						return pybind11::make_tuple(x[0], x[1]);
					} else if constexpr (Dim == 3) {
						return pybind11::make_tuple(x[0], x[1], x[2]);
					} else if constexpr (Dim == 4) {
						return pybind11::make_tuple(x[0], x[1], x[2], x[3]);
					} else if constexpr (Dim == 6) {
						return pybind11::make_tuple(x[0], x[1], x[2], x[3], x[4], x[5]);
					}
				},
				[](pybind11::tuple t) -> VectorT {
					if constexpr (Dim == Eigen::Dynamic) {
						std::vector<Scalar> v = t[0].cast<std::vector<Scalar>>();
						VectorT vec(v.size());
						for(size_t i = 0; i < v.size(); i++) vec[i] = v[i];
						return vec;
					} else if constexpr (Dim == 2) {
						return VectorT(t[0].cast<Scalar>(), t[1].cast<Scalar>());
					} else if constexpr (Dim == 3) {
						return VectorT(t[0].cast<Scalar>(), t[1].cast<Scalar>(), t[2].cast<Scalar>());
					} else if constexpr (Dim == 4) {
						return VectorT(t[0].cast<Scalar>(), t[1].cast<Scalar>(), t[2].cast<Scalar>(), t[3].cast<Scalar>());
					} else if constexpr (Dim == 6) {
						VectorT v;
						v << t[0].cast<Scalar>(), t[1].cast<Scalar>(), t[2].cast<Scalar>(),
						     t[3].cast<Scalar>(), t[4].cast<Scalar>(), t[5].cast<Scalar>();
						return v;
					}
				}
			))
			.def("__setitem__", [](VectorT& self, Index ix, pybind11::object value_obj){
				if(ix < 0 || ix >= (Index)(Dim == Eigen::Dynamic ? self.size() : Dim)) throw pybind11::index_error("Index out of bounds");
				Scalar value = pybind11::cast<Scalar>(value_obj);
				self(ix) = value;
			})
			.def("__getitem__", [](const VectorT& self, Index ix){
				if(ix < 0 || ix >= (Index)(Dim == Eigen::Dynamic ? self.size() : Dim)) throw pybind11::index_error("Index out of bounds");
				return typename VectorT::Scalar(self(ix));
			})
			.def("__str__", [name](const pybind11::object& obj){
				VectorT self = obj.cast<VectorT>();
				std::ostringstream oss;
				oss << name << (Dim == Eigen::Dynamic && self.size() > 0 ? "([" : "(");
				for(Index i = 0; i < self.size(); i++) {
					if(i > 0) oss << ((i % 3 != 0) ? ", " : ", ");
					oss << num_to_string(self[i]);
				}
				oss << (Dim == Eigen::Dynamic && self.size() > 0 ? "])" : ")");
				return oss.str();
			})
			.def("__repr__", [](const pybind11::object& obj){ return obj.attr("__str__")(); })
			.def("dot", [](const VectorT& self, const VectorT& other){ return self.dot(other); }, pybind11::arg("other"), "Dot product with *other*.")
			.def("outer", [](const VectorT& self, const VectorT& other){ return to_concrete<CompatMatrixT>(self * other.transpose()); }, pybind11::arg("other"), "Outer product with *other*.")
			.def("asDiagonal", [](const VectorT& self){ return to_concrete<CompatMatrixT>(self.asDiagonal()); }, "Return diagonal matrix with this vector on the diagonal.");
		
		// Apply MatrixBaseVisitor functionality
		MatrixBaseVisitor<VectorT>::expose(cls);
		
		// Dynamic vector specific
		if constexpr (Dim == Eigen::Dynamic) {
			cls
				.def("__len__", [](const VectorT& self){ return static_cast<int>(self.size()); })
				.def("resize", [](VectorT& self, Index size){ self.resize(size); })
				.def_static("Unit", [](Index size, Index ix){
					if(ix < 0 || ix >= size) throw pybind11::index_error("Index out of bounds");
					return to_concrete<VectorT>(VectorT::Unit(size, ix));
				})
				.def_static("Ones", [](Index size){ return to_concrete<VectorT>(VectorT::Ones(size)); })
				.def_static("Zero", [](Index size){ return to_concrete<VectorT>(VectorT::Zero(size)); })
				.def_static("Random", [](Index len){
					return to_concrete<VectorT>(VectorT::Random(len));
				}, pybind11::arg("len"), "Return vector of given length with all elements set to values between 0 and 1 randomly.")
				.def(pybind11::init([](const std::vector<Scalar>& vv){
					VectorT v(vv.size());
					for(size_t i = 0; i < vv.size(); i++) v[i] = vv[i];
					return v;
				}))
				.def(pybind11::init([](Index size){
					VectorT v(size);
					v.setZero();
					return v;
				}), pybind11::arg("size"), "Construct vector of given size, initialized to zero.");
		} else {
			// Fixed-size vector
			cls.def("__len__", [](const VectorT&){ return static_cast<int>(Dim); })
			    .def_static("Unit", [](Index ix){
					if(ix < 0 || ix >= (Index)Dim) throw pybind11::index_error("Index out of bounds");
					return to_concrete<VectorT>(VectorT::Unit(ix));
				});
		}

		// Size-specific features
		if constexpr (Dim == 2) {
			cls.def(pybind11::init<Scalar, Scalar>(), pybind11::arg("x"), pybind11::arg("y"))
			    .def(pybind11::init([](const std::vector<Scalar>& v){
					if(v.size() != 2) throw std::runtime_error("Vector2 requires exactly 2 elements");
					VectorT vec; vec << v[0], v[1]; return vec;
				}), pybind11::arg("seq"), "Construct from sequence (list/tuple) of 2 floats")
			    .def_property_readonly_static("UnitX", [](pybind11::object){ return to_concrete<VectorT>(CompatVec2::UnitX()); })
			    .def_property_readonly_static("UnitY", [](pybind11::object){ return to_concrete<VectorT>(CompatVec2::UnitY()); });
		} else if constexpr (Dim == 3) {
			cls.def(pybind11::init<Scalar, Scalar, Scalar>(), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"))
			    .def(pybind11::init([](const std::vector<Scalar>& v){
					if(v.size() != 3) throw std::runtime_error("Vector3 requires exactly 3 elements");
					VectorT vec; vec << v[0], v[1], v[2]; return vec;
				}), pybind11::arg("seq"), "Construct from sequence (list/tuple) of 3 floats")
			    .def("cross", [](const CompatVec3& self, const CompatVec3& other){ return to_concrete<VectorT>(self.cross(other)); }, "Cross product")
			    .def_property_readonly_static("UnitX", [](pybind11::object){ return to_concrete<VectorT>(CompatVec3::UnitX()); })
			    .def_property_readonly_static("UnitY", [](pybind11::object){ return to_concrete<VectorT>(CompatVec3::UnitY()); })
			    .def_property_readonly_static("UnitZ", [](pybind11::object){ return to_concrete<VectorT>(CompatVec3::UnitZ()); })
			    .def("xy", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[0], v[1])); })
			    .def("yx", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[1], v[0])); })
			    .def("xz", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[0], v[2])); })
			    .def("zx", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[2], v[0])); })
			    .def("yz", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[1], v[2])); })
			    .def("zy", [](const CompatVec3& v){ return to_concrete<CompatVec2>(CompatVec2(v[2], v[1])); });
		} else if constexpr (Dim == 4) {
			cls.def(pybind11::init<Scalar, Scalar, Scalar, Scalar>(), pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"));
		} else if constexpr (Dim == 6) {
			cls.def(pybind11::init([](const Scalar& v0, const Scalar& v1, const Scalar& v2, const Scalar& v3, const Scalar& v4, const Scalar& v5){
				VectorT v; v << v0, v1, v2, v3, v4, v5; return v;
			}), pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"), pybind11::arg("v4"), pybind11::arg("v5"))
			.def(pybind11::init([](const CompatVec3& head, const CompatVec3& tail){
				VectorT v; v << head, tail; return v;
			}), pybind11::arg("head"), pybind11::arg("tail"))
			.def("head", [](const CompatVec6& v){ return to_concrete<CompatVec3>(v.template head<3>()); }, "Return first 3 elements")
			.def("tail", [](const CompatVec6& v){ return to_concrete<CompatVec3>(v.template tail<3>()); }, "Return last 3 elements");
		}
	}
};

// MatrixVisitor
template<typename MatrixT>
class MatrixVisitor {
	typedef typename MatrixT::Scalar Scalar;
	typedef Eigen::Matrix<Scalar, MatrixT::RowsAtCompileTime, 1> CompatVectorT;
	typedef Eigen::Matrix<Scalar, 2, 2> CompatMat2;
	typedef Eigen::Matrix<Scalar, 3, 3> CompatMat3;
	typedef Eigen::Matrix<Scalar, 2, 1> CompatVec2;
	typedef Eigen::Matrix<Scalar, 3, 1> CompatVec3;
	typedef Eigen::Matrix<Scalar, 6, 6> CompatMat6;
	typedef Eigen::Matrix<Scalar, 6, 1> CompatVec6;
	typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> CompatMatX;
	typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> CompatVecX;
	enum { Dim = MatrixT::RowsAtCompileTime };
public:
	static void expose(pybind11::class_<MatrixT>& cls, const char* name) {
		cls
			.def(pybind11::pickle(
				[](const MatrixT& x) -> pybind11::tuple {
					if constexpr (Dim == Eigen::Dynamic) {
						std::vector<CompatVecX> rows;
						for(Index i = 0; i < x.rows(); i++) {
							CompatVecX row(x.cols());
							for(Index j = 0; j < x.cols(); j++) row[j] = x(i, j);
							rows.push_back(row);
						}
						return pybind11::make_tuple(rows);
					} else if constexpr (Dim == 2) {
						return pybind11::make_tuple(x(0,0), x(0,1), x(1,0), x(1,1));
					} else if constexpr (Dim == 3) {
						return pybind11::make_tuple(x(0,0), x(0,1), x(0,2), x(1,0), x(1,1), x(1,2), x(2,0), x(2,1), x(2,2));
					} else if constexpr (Dim == 6) {
						// Return all elements as flat vector
						std::vector<Scalar> flat;
						for(Index i = 0; i < 6; i++) {
							for(Index j = 0; j < 6; j++) {
								flat.push_back(x(i, j));
							}
						}
						return pybind11::make_tuple(flat);
					}
				},
				[](pybind11::tuple t) -> MatrixT {
					if constexpr (Dim == Eigen::Dynamic) {
						std::vector<CompatVecX> rows = t[0].cast<std::vector<CompatVecX>>();
						int r = rows.size(), c = rows.size() > 0 ? rows[0].size() : 0;
						MatrixT m(r, c);
						for(int i = 0; i < r; i++) m.row(i) = rows[i];
						return m;
					} else if constexpr (Dim == 2) {
						MatrixT m; m << t[0].cast<Scalar>(), t[1].cast<Scalar>(), t[2].cast<Scalar>(), t[3].cast<Scalar>(); return m;
					} else if constexpr (Dim == 3) {
						MatrixT m; m << t[0].cast<Scalar>(), t[1].cast<Scalar>(), t[2].cast<Scalar>(),
						        t[3].cast<Scalar>(), t[4].cast<Scalar>(), t[5].cast<Scalar>(),
						        t[6].cast<Scalar>(), t[7].cast<Scalar>(), t[8].cast<Scalar>(); return m;
					} else if constexpr (Dim == 6) {
						std::vector<Scalar> flat = t[0].cast<std::vector<Scalar>>();
						MatrixT m; Index idx = 0;
						for(Index i = 0; i < 6; i++) { for(Index j = 0; j < 6; j++) { m(i, j) = flat[idx++]; } } return m;
					}
				}
			))
			.def(pybind11::init([](const CompatVectorT& diag){ MatrixT m = diag.asDiagonal(); return m; }), pybind11::arg("diag"))
			.def("determinant", &MatrixT::determinant, "Return matrix determinant.")
			.def("trace", &MatrixT::trace, "Return sum of diagonal elements.")
			.def("transpose", [](const MatrixT& m){ return to_concrete<MatrixT>(m.transpose()); }, "Return transposed matrix.")
			.def("diagonal", [](const MatrixT& m){ return to_concrete<CompatVectorT>(m.diagonal()); }, "Return diagonal as vector.")
			.def("row", [](const MatrixT& a, Index ix){
				if(ix < 0 || ix >= a.rows()) throw pybind11::index_error("Index out of bounds");
				return to_concrete<CompatVectorT>(a.row(ix));
			}, pybind11::arg("row"), "Return row as vector.")
			.def("col", [](const MatrixT& a, Index ix){
				if(ix < 0 || ix >= a.cols()) throw pybind11::index_error("Index out of bounds");
				return to_concrete<CompatVectorT>(a.col(ix));
			}, pybind11::arg("col"), "Return column as vector.")
			.def("__setitem__", [](MatrixT& a, Index ix, const CompatVectorT& val){
				if(ix < 0 || ix >= a.rows()) throw pybind11::index_error("Index out of bounds");
				a.row(ix) = val;
			})
			.def("__getitem__", [](const MatrixT& a, Index ix){
				if(ix < 0 || ix >= a.rows()) throw pybind11::index_error("Index out of bounds");
				return to_concrete<CompatVectorT>(a.row(ix));
			})
			.def("__setitem__", [](MatrixT& a, pybind11::tuple idx, Scalar val){
				Index i = idx[0].cast<Index>(), j = idx[1].cast<Index>();
				if(i < 0 || i >= a.rows() || j < 0 || j >= a.cols()) throw pybind11::index_error("Index out of bounds");
				a(i, j) = val;
			})
			.def("__getitem__", [](const MatrixT& a, pybind11::tuple idx){
				Index i = idx[0].cast<Index>(), j = idx[1].cast<Index>();
				if(i < 0 || i >= a.rows() || j < 0 || j >= a.cols()) throw pybind11::index_error("Index out of bounds");
				return a(i, j);
			})
			.def("__str__", [name](const MatrixT& mat){
				std::ostringstream oss; oss << name << "(";
				for(Index i = 0; i < mat.rows(); i++){
					if(i > 0) oss << ", ";
					oss << "[";
					for(Index j = 0; j < mat.cols(); j++){
						if(j > 0) oss << ", ";
						oss << num_to_string(mat(i, j));
					}
					oss << "]";
				}
				oss << ")";
				return oss.str();
			})
			.def("__repr__", [name](const MatrixT& mat){
				std::ostringstream oss; oss << name << "(";
				for(Index i = 0; i < mat.rows(); i++){
					if(i > 0) oss << ", ";
					oss << "[";
					for(Index j = 0; j < mat.cols(); j++){
						if(j > 0) oss << ", ";
						oss << num_to_string(mat(i, j));
					}
					oss << "]";
				}
				oss << ")";
				return oss.str();
			})
			.def("__mul__", [](const MatrixT& a, const MatrixT& b){ return to_concrete<MatrixT>(a * b); })
			.def("__mul__", [](const MatrixT& a, const CompatVectorT& v){ return to_concrete<CompatVectorT>(a * v); })
			.def("__rmul__", [](const MatrixT& a, const CompatVectorT& v){ return to_concrete<CompatVectorT>(a * v); });

		// Apply MatrixBaseVisitor functionality
		MatrixBaseVisitor<MatrixT>::expose(cls);

		// Add float-specific operations
		if constexpr (!std::is_integral<Scalar>::value) {
			cls.def("inverse", [](const MatrixT& m){ return to_concrete<MatrixT>(m.inverse()); }, "Return inverted matrix.");

			// Add decompositions for non-complex types
			if constexpr (!Eigen::NumTraits<Scalar>::IsComplex) {
				cls
					.def("jacobiSVD", [](const MatrixT& m){
						auto svd = m.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
						return pybind11::make_tuple(to_concrete<MatrixT>(svd.matrixU()), to_concrete<CompatVectorT>(svd.singularValues()), to_concrete<MatrixT>(svd.matrixV()));
					}, "Compute SVD decomposition, returns (U, S, V) such that self = U*S*V.transpose()")
					.def("svd", [](const MatrixT& m){
						auto svd = m.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
						return pybind11::make_tuple(to_concrete<MatrixT>(svd.matrixU()), to_concrete<CompatVectorT>(svd.singularValues()), to_concrete<MatrixT>(svd.matrixV()));
					}, "Alias for jacobiSVD.")
					.def("computeUnitaryPositive", [](const MatrixT& m){
						auto svd = m.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
						return pybind11::make_tuple(to_concrete<MatrixT>(svd.matrixU()), to_concrete<CompatVectorT>(svd.singularValues()), to_concrete<MatrixT>(svd.matrixV()));
					}, "Compute polar decomposition (unitary matrix U and positive semi-definite symmetric matrix P such that self = U*P). Returns (U, S, V).")
					.def("polarDecomposition", [](const MatrixT& m){
						auto svd = m.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
						return pybind11::make_tuple(to_concrete<MatrixT>(svd.matrixU()), to_concrete<CompatVectorT>(svd.singularValues()), to_concrete<MatrixT>(svd.matrixV()));
					}, "Alias for computeUnitaryPositive. Returns (U, S, V).")
					.def("selfAdjointEigenDecomposition", [](const MatrixT& m){
						Eigen::SelfAdjointEigenSolver<MatrixT> es(m);
						return pybind11::make_tuple(to_concrete<MatrixT>(es.eigenvectors()), to_concrete<CompatVectorT>(es.eigenvalues()));
					}, "Compute eigen (spectral) decomposition of symmetric matrix, returns (eigVecs, eigVals).")
					.def("spectralDecomposition", [](const MatrixT& m){
						Eigen::SelfAdjointEigenSolver<MatrixT> es(m);
						return pybind11::make_tuple(to_concrete<MatrixT>(es.eigenvectors()), to_concrete<CompatVectorT>(es.eigenvalues()));
					}, "Alias for selfAdjointEigenDecomposition.");
			}
		}
		
		// Dynamic matrix specific
		if constexpr (Dim == Eigen::Dynamic) {
			cls
				.def("__len__", [](const MatrixT& a){ return static_cast<int>(a.rows()); })
				.def("resize", [](MatrixT& self, Index rows, Index cols){ self.resize(rows, cols); },
					"Change size of the matrix, keep values of elements which exist in the new matrix", pybind11::arg("rows"), pybind11::arg("cols"))
				.def_static("Ones", [](Index rows, Index cols){ return to_concrete<MatrixT>(MatrixT::Ones(rows, cols)); },
					"Create matrix of given dimensions where all elements are set to 1.", pybind11::arg("rows"), pybind11::arg("cols"))
				.def_static("Zero", [](Index rows, Index cols){ return to_concrete<MatrixT>(MatrixT::Zero(rows, cols)); },
					"Create zero matrix of given dimensions", pybind11::arg("rows"), pybind11::arg("cols"))
				.def_static("Random", [](Index rows, Index cols){ return to_concrete<MatrixT>(MatrixT::Random(rows, cols)); },
					"Create matrix with given dimensions where all elements are set to number between 0 and 1 (uniformly-distributed).", pybind11::arg("rows"), pybind11::arg("cols"))
				.def_static("Identity", [](Index rows, Index cols){ return to_concrete<MatrixT>(MatrixT::Identity(rows, cols)); },
					"Create identity matrix with given dimensions (square if rows==cols).", pybind11::arg("rows"), pybind11::arg("cols"))
				.def_static("Identity", [](Index rank){ return to_concrete<MatrixT>(MatrixT::Identity(rank, rank)); },
					"Create identity matrix with given rank (square).", pybind11::arg("rank"));
		} else {
			// Fixed-size matrix
			cls.def("__len__", [](const MatrixT&){ return static_cast<int>(Dim); });
		}

		// Size-specific features
		if constexpr (Dim == 2) {
			// Eigen doesn't support scalar matrix constructors, use lambda instead
			cls.def(pybind11::init([](Scalar m00, Scalar m01, Scalar m10, Scalar m11){
				MatrixT m; m << m00, m01, m10, m11; return m;
			}), pybind11::arg("m00"), pybind11::arg("m01"), pybind11::arg("m10"), pybind11::arg("m11"))
			    .def(pybind11::init([](const CompatVec2& r0, const CompatVec2& r1, bool cols = false){
				MatrixT m; if(cols) { m.col(0) = r0; m.col(1) = r1; } else { m.row(0) = r0; m.row(1) = r1; } return m;
			}), pybind11::arg("r0"), pybind11::arg("r1"), pybind11::arg("cols") = false);
		} else if constexpr (Dim == 3) {
			// Eigen doesn't support scalar matrix constructors, use lambda instead
			cls.def(pybind11::init([](Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22){
				MatrixT m; m << m00, m01, m02, m10, m11, m12, m20, m21, m22; return m;
			}), pybind11::arg("m00"), pybind11::arg("m01"), pybind11::arg("m02"), pybind11::arg("m10"), pybind11::arg("m11"), pybind11::arg("m12"), pybind11::arg("m20"), pybind11::arg("m21"), pybind11::arg("m22"))
			    .def(pybind11::init([](const CompatVec3& r0, const CompatVec3& r1, const CompatVec3& r2, bool cols = false){
				MatrixT m; if(cols) { m.col(0) = r0; m.col(1) = r1; m.col(2) = r2; } else { m.row(0) = r0; m.row(1) = r1; m.row(2) = r2; } return m;
			}), pybind11::arg("r0"), pybind11::arg("r1"), pybind11::arg("r2"), pybind11::arg("cols") = false);
		} else if constexpr (Dim == 6) {
			cls.def(pybind11::init([](const CompatMat3& ul, const CompatMat3& ur, const CompatMat3& ll, const CompatMat3& lr){
				MatrixT m; m << ul, ur, ll, lr; return m;
			}), pybind11::arg("ul"), pybind11::arg("ur"), pybind11::arg("ll"), pybind11::arg("lr"))
			.def(pybind11::init([](const CompatVec6& r0, const CompatVec6& r1, const CompatVec6& r2, const CompatVec6& r3, const CompatVec6& r4, const CompatVec6& r5, bool cols = false){
				MatrixT m; if(cols) { m.col(0) = r0; m.col(1) = r1; m.col(2) = r2; m.col(3) = r3; m.col(4) = r4; m.col(5) = r5; } else { m.row(0) = r0; m.row(1) = r1; m.row(2) = r2; m.row(3) = r3; m.row(4) = r4; m.row(5) = r5; } return m;
			}), pybind11::arg("r0"), pybind11::arg("r1"), pybind11::arg("r2"), pybind11::arg("r3"), pybind11::arg("r4"), pybind11::arg("r5"), pybind11::arg("cols") = false)
			.def("ul", [](const CompatMat6& m){ return to_concrete<CompatMat3>(m.template topLeftCorner<3,3>()); }, "Return upper-left 3x3 block")
			.def("ur", [](const CompatMat6& m){ return to_concrete<CompatMat3>(m.template topRightCorner<3,3>()); }, "Return upper-right 3x3 block")
			.def("ll", [](const CompatMat6& m){ return to_concrete<CompatMat3>(m.template bottomLeftCorner<3,3>()); }, "Return lower-left 3x3 block")
			.def("lr", [](const CompatMat6& m){ return to_concrete<CompatMat3>(m.template bottomRightCorner<3,3>()); }, "Return lower-right 3x3 block");
		}
	}
};

// AlignedBox visitor
template<typename AlignedBoxT>
class AabbVisitor {
	typedef typename AlignedBoxT::VectorType VectorT;
public:
	static void expose(pybind11::class_<AlignedBoxT>& cls, const char* doc) {
		cls
			.def(pybind11::init<>())
			.def(pybind11::init<const VectorT&, const VectorT&>())
			.def_property_readonly("min", [](const AlignedBoxT& box){ return box.min(); }, "Minimum corner")
			.def_property_readonly("max", [](const AlignedBoxT& box){ return box.max(); }, "Maximum corner")
			.def_property_readonly("center", [](const AlignedBoxT& box){ return box.center(); }, "Center of the box")
			.def_property_readonly("sizes", [](const AlignedBoxT& box){ return box.sizes(); }, "Sizes of the box")
			.def_property_readonly("volume", &AlignedBoxT::volume, "Volume of the box")
			.def("extend", [](AlignedBoxT& box, const VectorT& point){ box.extend(point); }, "Extend the box to include another point")
			.def("extend", [](AlignedBoxT& box, const AlignedBoxT& other){ box.extend(other); }, "Extend the box to include another box")
			.def("contains", [](const AlignedBoxT& box, const VectorT& point){ return box.contains(point); }, "Check if contains a point")
			.def("intersects", &AlignedBoxT::intersects, "Check if intersects with another box")
			.def("merged", &AlignedBoxT::merged, "Return merged box")
			.def("isEmpty", &AlignedBoxT::isEmpty, "Check if empty")
			.def("isApprox", &AlignedBoxT::isApprox, "Approximate comparison");
	}
};

// Rotation2D visitor
class Rotation2DVisitor {
	typedef Eigen::Matrix<Real,2,1> CompatVec2;
	typedef Eigen::Matrix<Real,2,2> CompatMat2;
public:
	static void expose(pybind11::class_<Rotationr>& cls) {
		cls
			.def(pybind11::init<>(), "Default constructor")
			.def(pybind11::init<Real>(), "Initialize from a rotation angle.")
			.def(pybind11::init<CompatMat2>(), "Initialize from given rotation matrix.")
			.def(pybind11::init<const Rotationr&>())
			.def_property_readonly_static("Identity", [](pybind11::object){ return Rotationr::Identity(); })
			.def_property("angle", 
				[](const Rotationr& r){ return r.angle(); },
				[](Rotationr& r, Real angle){ r.angle() = angle; },
				"The rotation angle.")
			.def("inverse", &Rotationr::inverse)
			.def("toRotationMatrix", &Rotationr::toRotationMatrix, "Constructs and returns an equivalent 2x2 rotation matrix.")
			.def("smallestAngle", &Rotationr::smallestAngle, "The rotation angle in [-pi,pi]")
			.def("smallestPositiveAngle", &Rotationr::smallestPositiveAngle, "The rotation angle in [0,2pi]")
			.def("slerp", [](const Rotationr& self, Real t, const Rotationr& other){ return self.slerp(t, other); }, 
				pybind11::arg("t"), pybind11::arg("other"))
			.def("__mul__", [](const Rotationr& a, const Rotationr& b){ return a * b; })
			.def("__imul__", [](Rotationr& a, const Rotationr& b){ a *= b; return a; })
			.def("__mul__", [](const Rotationr& r, const CompatVec2& v){ return r * v; })
			.def("__eq__", [](const Rotationr& a, const Rotationr& b){ return a.angle() == b.angle(); })
			.def("__ne__", [](const Rotationr& a, const Rotationr& b){ return a.angle() != b.angle(); })
			.def("__repr__", [](const Rotationr& r){
				std::ostringstream oss;
				oss << "Rotation2d(" << num_to_string(r.angle()) << ")";
				return oss.str();
			})
			.def(pybind11::pickle(
				[](const Rotationr& r){ return pybind11::make_tuple(r.angle()); },
				[](pybind11::tuple t){ return Rotationr(t[0].cast<Real>()); }
			));
	}
};

// Quaternion visitor
class QuaternionVisitor {
public:
	static void expose(pybind11::class_<Quaternionr>& cls) {
		cls
			.def(pybind11::init<>())
			.def(pybind11::init<Real, Real, Real, Real>())
			// Constructor from (axis, angle) - old API: supports both Quaternion(Vector3, angle) and Quaternion((tuple), angle)
			.def(pybind11::init([](pybind11::object arg1, Real angle){
				Vector3r axis;
				// Check if first arg is a tuple (old API: Quaternion((0,1,0), 0.0))
				if (pybind11::isinstance<pybind11::tuple>(arg1)) {
					pybind11::tuple t = arg1.cast<pybind11::tuple>();
					if (pybind11::len(t) == 3) {
						axis << t[0].cast<Real>(), t[1].cast<Real>(), t[2].cast<Real>();
					} else {
						throw std::runtime_error("Quaternion axis tuple must have 3 elements");
					}
				} else {
					// Assume it's already a Vector3
					axis = arg1.cast<Vector3r>();
				}
				Eigen::AngleAxisd aa(angle, axis.normalized());
				return Quaternionr(aa);
			}), pybind11::arg("axis"), pybind11::arg("angle"))
			.def_property("x", [](const Quaternionr& q){ return q.x(); }, [](Quaternionr& q, Real val){ q.x() = val; })
			.def_property("y", [](const Quaternionr& q){ return q.y(); }, [](Quaternionr& q, Real val){ q.y() = val; })
			.def_property("z", [](const Quaternionr& q){ return q.z(); }, [](Quaternionr& q, Real val){ q.z() = val; })
			.def_property("w", [](const Quaternionr& q){ return q.w(); }, [](Quaternionr& q, Real val){ q.w() = val; })
			.def("angularDistance", [](const Quaternionr& a, const Quaternionr& b){ return a.angularDistance(b); })
			.def("conjugate", &Quaternionr::conjugate)
			.def("inverse", &Quaternionr::inverse)
			.def("norm", &Quaternionr::norm)
			.def("normalize", &Quaternionr::normalize)
			.def("normalized", &Quaternionr::normalized)
			.def("slerp", [](const Quaternionr& a, const Quaternionr& b, Real t){ return a.slerp(t, b); })
			.def("toRotationMatrix", &Quaternionr::toRotationMatrix)
			.def("vec", [](const Quaternionr& q){ return q.vec(); })
			.def("__repr__", [](const Quaternionr& q){
				std::ostringstream oss;
				oss << "Quaternion(" << q.w() << ", " << q.x() << ", " << q.y() << ", " << q.z() << ")";
				return oss.str();
			})
			.def("__mul__", [](const Quaternionr& q, const Quaternionr& other){ return q * other; })
			.def("__mul__", [](const Quaternionr& q, const Vector3r& v){ return q * v; })
			.def("__eq__", [](const Quaternionr& a, const Quaternionr& b){ return a.isApprox(b); })
			.def("__ne__", [](const Quaternionr& a, const Quaternionr& b){ return !a.isApprox(b); })
			.def_property_readonly_static("Identity", [](pybind11::object){ return Quaternionr::Identity(); });
	}
};

// Helper functions
inline void expose_vectors(pybind11::module_& m) {
	VectorVisitor<VectorXr>::expose(m, "VectorX", "Dynamic-sized float vector.\n\nSupported operations (``f`` if a float/int, ``v`` is a VectorX): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*f``, ``f*v``, ``v*=f``, ``v/f``, ``v/=f``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence (list, tuple, ...) of X floats.");
	VectorVisitor<Vector6r>::expose(m, "Vector6", "6-dimensional float vector.\n\nSupported operations (``f`` if a float/int, ``v`` is a Vector6): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*f``, ``f*v``, ``v*=f``, ``v/f``, ``v/=f``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence (list, tuple, ...) of 6 floats.\n\nStatic attributes: ``Zero``, ``Ones``.");
	VectorVisitor<Vector6i>::expose(m, "Vector6i", "6-dimensional integer vector.\n\nSupported operations (``i`` if an int, ``v`` is a Vector6): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*i``, ``i*v``, ``v*=i``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence  (list, tuple, ...) of 6 integers.\n\nStatic attributes: ``Zero``, ``Ones``.");
	VectorVisitor<Vector3r>::expose(m, "Vector3", "3-dimensional float vector.\n\nSupported operations (``f`` if a float/int, ``v`` is a Vector3): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*f``, ``f*v``, ``v*=f``, ``v/f``, ``v/=f``, ``v==v``, ``v!=v``, plus operations with ``Matrix3`` and ``Quaternion``.\n\nImplicit conversion from sequence (list, tuple, ...) of 3 floats.\n\nStatic attributes: ``Zero``, ``Ones``, ``UnitX``, ``UnitY``, ``UnitZ``.");
	// Vector3ra (AlignedVector3) is not supported by pybind11's eigen matrix caster
	// Not used in Python code, so not exposed
	VectorVisitor<Vector3i>::expose(m, "Vector3i", "3-dimensional integer vector.\n\nSupported operations (``i`` if an int, ``v`` is a Vector3i): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*i``, ``i*v``, ``v*=i``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence  (list, tuple, ...) of 3 integers.\n\nStatic attributes: ``Zero``, ``Ones``, ``UnitX``, ``UnitY``, ``UnitZ``.");
	VectorVisitor<Vector2r>::expose(m, "Vector2", "2-dimensional float vector.\n\nSupported operations (``f`` if a float/int, ``v`` is a Vector2): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*f``, ``f*v``, ``v*=f``, ``v/f``, ``v/=f``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence (list, tuple, ...) of 2 floats.\n\nStatic attributes: ``Zero``, ``Ones``, ``UnitX``, ``UnitY``.");
	VectorVisitor<Vector2i>::expose(m, "Vector2i", "2-dimensional integer vector.\n\nSupported operations (``i`` if an int, ``v`` is a Vector2i): ``-v``, ``v+v``, ``v+=v``, ``v-v``, ``v-=v``, ``v*i``, ``i*v``, ``v*=i``, ``v==v``, ``v!=v``.\n\nImplicit conversion from sequence (list, tuple, ...) of 2 integers.\n\nStatic attributes: ``Zero``, ``Ones``, ``UnitX``, ``UnitY``.");
}

inline void expose_matrices(pybind11::module_& m) {
	auto matX = pybind11::class_<MatrixXr>(m, "MatrixX", "Dynamic-sized float matrix");
	MatrixVisitor<MatrixXr>::expose(matX, "MatrixX");
	
	auto mat6 = pybind11::class_<Matrix6r>(m, "Matrix6", "6x6 float matrix");
	MatrixVisitor<Matrix6r>::expose(mat6, "Matrix6");
	
	auto mat3 = pybind11::class_<Matrix3r>(m, "Matrix3", "3x3 float matrix");
	mat3.def(pybind11::init<const Quaternionr&>(), pybind11::arg("q"), "Initialize from quaternion.");
	MatrixVisitor<Matrix3r>::expose(mat3, "Matrix3");
	
	auto mat2 = pybind11::class_<Matrix2r>(m, "Matrix2", "2x2 float matrix");
	MatrixVisitor<Matrix2r>::expose(mat2, "Matrix2");
}

inline void expose_boxes(pybind11::module_& m) {
	auto box3 = pybind11::class_<AlignedBox3r>(m, "AlignedBox3", "Axis-aligned box object, defined by its minimum and maximum corners");
	AabbVisitor<AlignedBox3r>::expose(box3, "AlignedBox3");
	
	// TODO: AlignedBox2 is not working with pybind11 due to Eigen::AlignedBox<Real,2> not being implicitly convertible
	// It's not used in Python code, so commented out for now
	// auto box2 = pybind11::class_<AlignedBox2r>(m, "AlignedBox2", "Axis-aligned box object in 2d, defined by its minimum and maximum corners");
	// AabbVisitor<AlignedBox2r>::expose(box2, "AlignedBox2");
}

inline void expose_quaternion(pybind11::module_& m) {
	auto quat = pybind11::class_<Quaternionr>(m, "Quaternion", "Quaternion representing rotation");
	QuaternionVisitor::expose(quat);
	
	auto rot2d = pybind11::class_<Rotationr>(m, "Rotation2d", "Rotation2D");
	Rotation2DVisitor::expose(rot2d);
}

#ifdef _COMPLEX_SUPPORT
inline void expose_complex(pybind11::module_& m) {
	VectorVisitor<Vector2cr>::expose(m, "Vector2c", "2-dimensional complex vector.");
	VectorVisitor<Vector3cr>::expose(m, "Vector3c", "3-dimensional complex vector.");
	VectorVisitor<Vector6cr>::expose(m, "Vector6c", "6-dimensional complex vector.");
	VectorVisitor<VectorXcr>::expose(m, "VectorXc", "Dynamic-sized complex vector.");

	auto mat3c = pybind11::class_<Matrix3cr>(m, "Matrix3c", "3x3 complex matrix");
	MatrixVisitor<Matrix3cr>::expose(mat3c, "Matrix3c");
	
	auto mat6c = pybind11::class_<Matrix6cr>(m, "Matrix6c", "6x6 complex matrix");
	MatrixVisitor<Matrix6cr>::expose(mat6c, "Matrix6c");
	
	auto matXc = pybind11::class_<MatrixXcr>(m, "MatrixXc", "Dynamic-sized complex matrix");
	MatrixVisitor<MatrixXcr>::expose(matXc, "MatrixXc");
}
#else
inline void expose_complex(pybind11::module_& m) {}
#endif