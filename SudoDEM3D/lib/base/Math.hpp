// © 2010 Václav Šmilauer <eudoxos@arcig.cz>
#pragma once

#ifdef QUAD_PRECISION
	typedef long double quad;
	typedef quad Real;
#else
	typedef double Real;
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using std::endl;
using std::cout;
using std::cerr;
using std::vector;
using std::string;
using std::list;
using std::pair;
using std::min;
using std::max;
using std::set;
using std::map;
using std::type_info;
using std::ifstream;
using std::ofstream;
using std::runtime_error;
using std::logic_error;
using std::invalid_argument;
using std::ios;
using std::ios_base;
using std::fstream;
using std::ostream;
using std::ostringstream;
using std::istringstream;
using std::swap;
using std::make_pair;

// Modern C++ standard library includes
#include <type_traits>
#include <any>
#include <memory>
#include <tuple>
#include <filesystem>

using std::shared_ptr;


#ifndef SUDODEM_PTR_CAST
#define SUDODEM_PTR_CAST std::static_pointer_cast
#endif
#ifndef SUDODEM_CAST
	#define SUDODEM_CAST static_cast
#endif

#ifndef SUDODEM_PTR_DYN_CAST
#define SUDODEM_PTR_DYN_CAST std::dynamic_pointer_cast
#endif
#define EIGEN_DONT_PARALLELIZE

#ifdef SUDODEM_MASK_ARBITRARY
	#include <bitset>
#endif

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <float.h>

// templates of those types with single parameter are not possible, use macros for now
#define VECTOR2_TEMPLATE(Scalar) Eigen::Matrix<Scalar,2,1>
#define VECTOR3_TEMPLATE(Scalar) Eigen::Matrix<Scalar,3,1>
#define VECTOR6_TEMPLATE(Scalar) Eigen::Matrix<Scalar,6,1>
#define MATRIX3_TEMPLATE(Scalar) Eigen::Matrix<Scalar,3,3>
#define MATRIX2_TEMPLATE(Scalar) Eigen::Matrix<Scalar,2,2>
#define MATRIX6_TEMPLATE(Scalar) Eigen::Matrix<Scalar,6,6>

// this would be the proper way, but only works in c++-0x (not yet supported by gcc (4.5))
#if 0
	template<typename Scalar> using Vector2=Eigen::Matrix<Scalar,2,1>;
	template<typename Scalar> using Vector3=Eigen::Matrix<Scalar,3,1>;
	template<typename Scalar> using Matrix3=Eigen::Matrix<Scalar,3,3>;
	typedef Vector2<int> Vector2i;
	typedef Vector2<Real> Vector2r;
	// etc
#endif

typedef VECTOR2_TEMPLATE(int) Vector2i;
typedef VECTOR2_TEMPLATE(Real) Vector2r;
typedef VECTOR3_TEMPLATE(int) Vector3i;
typedef VECTOR3_TEMPLATE(Real) Vector3r;
typedef VECTOR6_TEMPLATE(Real) Vector6r;
typedef VECTOR6_TEMPLATE(int) Vector6i;
typedef MATRIX3_TEMPLATE(Real) Matrix3r;
typedef MATRIX2_TEMPLATE(Real) Matrix2r;
typedef MATRIX6_TEMPLATE(Real) Matrix6r;

typedef Eigen::Matrix<Real,Eigen::Dynamic,Eigen::Dynamic> MatrixXr;
typedef Eigen::Matrix<Real,Eigen::Dynamic,1> VectorXr;

typedef Eigen::Quaternion<Real> Quaternionr;
typedef Eigen::Rotation2D<Real> Rotationr;
typedef Eigen::AngleAxis<Real> AngleAxisr;
typedef Eigen::AlignedBox<Real,2> AlignedBox2r;
typedef Eigen::AlignedBox<Real,3> AlignedBox3r;
using Eigen::AngleAxis; using Eigen::Quaternion;using Eigen::Rotation2D;

// in some cases, we want to initialize types that have no default constructor (OpenMPAccumulator, for instance)
// template specialization will help us here
template<typename EigenMatrix> EigenMatrix ZeroInitializer(){ return EigenMatrix::Zero(); };
template<> int ZeroInitializer<int>();
template<> Real ZeroInitializer<Real>();


// io
template<class Scalar> std::ostream & operator<<(std::ostream &os, const VECTOR2_TEMPLATE(Scalar)& v){ os << v.x()<<" "<<v.y(); return os; };
template<class Scalar> std::ostream & operator<<(std::ostream &os, const VECTOR3_TEMPLATE(Scalar)& v){ os << v.x()<<" "<<v.y()<<" "<<v.z(); return os; };
template<class Scalar> std::ostream & operator<<(std::ostream &os, const VECTOR6_TEMPLATE(Scalar)& v){ os << v[0]<<" "<<v[1]<<" "<<v[2]<<" "<<v[3]<<" "<<v[4]<<" "<<v[5]; return os; };
template<class Scalar> std::ostream & operator<<(std::ostream &os, const Eigen::Quaternion<Scalar>& q){ os<<q.w()<<" "<<q.x()<<" "<<q.y()<<" "<<q.z(); return os; };
template<class Scalar> std::ostream & operator<<(std::ostream &os, const Eigen::Rotation2D<Scalar>& r){ os<<r.angle(); return os; };
// operators
//template<class Scalar> VECTOR3_TEMPLATE(Scalar) operator*(Scalar s, const VECTOR3_TEMPLATE(Scalar)& v) {return v*s;}
//template<class Scalar> MATRIX3_TEMPLATE(Scalar) operator*(Scalar s, const MATRIX3_TEMPLATE(Scalar)& m) { return m*s; }
//template<class Scalar> Quaternion<Scalar> operator*(Scalar s, const Quaternion<Scalar>& q) { return q*s; }
//template<typename Scalar> void matrixEigenDecomposition(const MATRIX3_TEMPLATE(Scalar) m, MATRIX3_TEMPLATE(Scalar)& mRot, MATRIX3_TEMPLATE(Scalar)& mDiag){ Eigen::SelfAdjointEigenSolver<MATRIX3_TEMPLATE(Scalar)> a(m); mRot=a.eigenvectors(); mDiag=a.eigenvalues().asDiagonal(); }
// http://eigen.tuxfamily.org/dox/TutorialGeometry.html
template<typename Scalar> MATRIX3_TEMPLATE(Scalar) matrixFromEulerAnglesXYZ(Scalar x, Scalar y, Scalar z){ MATRIX3_TEMPLATE(Scalar) m; m=AngleAxis<Scalar>(x,VECTOR3_TEMPLATE(Scalar)::UnitX())*AngleAxis<Scalar>(y,VECTOR3_TEMPLATE(Scalar)::UnitY())*AngleAxis<Scalar>(z,VECTOR3_TEMPLATE(Scalar)::UnitZ()); return m;}
template<typename Scalar> bool operator==(const Quaternion<Scalar>& u, const Quaternion<Scalar>& v){ return u.x()==v.x() && u.y()==v.y() && u.z()==v.z() && u.w()==v.w(); }
template<typename Scalar> bool operator!=(const Quaternion<Scalar>& u, const Quaternion<Scalar>& v){ return !(u==v); }
template<typename Scalar> bool operator==(const MATRIX3_TEMPLATE(Scalar)& m, const MATRIX3_TEMPLATE(Scalar)& n){ for(int i=0;i<3;i++)for(int j=0;j<3;j++)if(m(i,j)!=n(i,j)) return false; return true; }
template<typename Scalar> bool operator!=(const MATRIX3_TEMPLATE(Scalar)& m, const MATRIX3_TEMPLATE(Scalar)& n){ return !(m==n); }
template<typename Scalar> bool operator==(const MATRIX6_TEMPLATE(Scalar)& m, const MATRIX6_TEMPLATE(Scalar)& n){ for(int i=0;i<6;i++)for(int j=0;j<6;j++)if(m(i,j)!=n(i,j)) return false; return true; }
template<typename Scalar> bool operator!=(const MATRIX6_TEMPLATE(Scalar)& m, const MATRIX6_TEMPLATE(Scalar)& n){ return !(m==n); }
template<typename Scalar> bool operator==(const VECTOR6_TEMPLATE(Scalar)& u, const VECTOR6_TEMPLATE(Scalar)& v){ return u[0]==v[0] && u[1]==v[1] && u[2]==v[2] && u[3]==v[3] && u[4]==v[4] && u[5]==v[5]; }
template<typename Scalar> bool operator!=(const VECTOR6_TEMPLATE(Scalar)& u, const VECTOR6_TEMPLATE(Scalar)& v){ return !(u==v); }
template<typename Scalar> bool operator==(const VECTOR3_TEMPLATE(Scalar)& u, const VECTOR3_TEMPLATE(Scalar)& v){ return u.x()==v.x() && u.y()==v.y() && u.z()==v.z(); }
template<typename Scalar> bool operator!=(const VECTOR3_TEMPLATE(Scalar)& u, const VECTOR3_TEMPLATE(Scalar)& v){ return !(u==v); }
template<typename Scalar> bool operator==(const VECTOR2_TEMPLATE(Scalar)& u, const VECTOR2_TEMPLATE(Scalar)& v){ return u.x()==v.x() && u.y()==v.y(); }
template<typename Scalar> bool operator!=(const VECTOR2_TEMPLATE(Scalar)& u, const VECTOR2_TEMPLATE(Scalar)& v){ return !(u==v); }
template<typename Scalar> Quaternion<Scalar> operator*(Scalar f, const Quaternion<Scalar>& q){ return Quaternion<Scalar>(q.coeffs()*f); }
template<typename Scalar> Quaternion<Scalar> operator+(Quaternion<Scalar> q1, const Quaternion<Scalar>& q2){ return Quaternion<Scalar>(q1.coeffs()+q2.coeffs()); }	/* replace all those by standard math functions
	this is a non-templated version, to avoid compilation because of static constants;
*/
template<typename Scalar>
struct Math{
	inline static constexpr Scalar PI = Scalar(3.1415926535897932384626433832795L);
    inline static constexpr Scalar HALF_PI = Scalar(0.5) * PI;
    inline static constexpr Scalar TWO_PI = Scalar(2.0) * PI;
    inline static constexpr Scalar MAX_REAL = std::numeric_limits<Scalar>::max();
    inline static constexpr Scalar DEG_TO_RAD = PI / Scalar(180.0);
    inline static constexpr Scalar RAD_TO_DEG = Scalar(180.0) / PI;
    inline static constexpr Scalar EPSILON = std::numeric_limits<Scalar>::epsilon();
    inline static constexpr Scalar ZERO_TOLERANCE = Scalar(1e-20);

	static Scalar Sign(Scalar f){ if(f<0) return -1; if(f>0) return 1; return 0; }

	static Scalar UnitRandom(){ return ((double)rand()/((double)(RAND_MAX))); }
	static Scalar SymmetricRandom(){ return 2.*(((double)rand())/((double)(RAND_MAX)))-1.; }
	static Scalar FastInvCos0(Scalar fValue){ Scalar fRoot = sqrt(((Scalar)1.0)-fValue); Scalar fResult = -(Scalar)0.0187293; fResult *= fValue; fResult += (Scalar)0.0742610; fResult *= fValue; fResult -= (Scalar)0.2121144; fResult *= fValue; fResult += (Scalar)1.5707288; fResult *= fRoot; return fResult; }
	//math functions
	/////////////////////////gamma func----referring to NR3.0
	static double gammln(const double xx) {
		int j;
		double x,tmp,y,ser;
		static const double cof[14]={57.1562356658629235,-59.5979603554754912,
		14.1360979747417471,-0.491913816097620199,.339946499848118887e-4,
		.465236289270485756e-4,-.983744753048795646e-4,.158088703224912494e-3,
		-.210264441724104883e-3,.217439618115212643e-3,-.164318106536763890e-3,
		.844182239838527433e-4,-.261908384015814087e-4,.368991826595316234e-5};
		if (xx <= 0) throw("bad arg in gammln");
		y=x=xx;
		tmp = x+5.24218750000000000;
		tmp = (x+0.5)*log(tmp)-tmp;
		ser = 0.999999999999997092;
		for (j=0;j<14;j++) ser += cof[j]/++y;
		return tmp+log(2.5066282746310005*ser/x);
	}
	///////////////////////beta func
	static double beta(const double z, const double w) {
		return exp(gammln(z)+gammln(w)-gammln(z+w));
	}

};
typedef Math<Real> Mathr;

/* this was removed in eigen3, see http://forum.kde.org/viewtopic.php?f=74&t=90914 */
template<typename MatrixT>
void Matrix_computeUnitaryPositive(const MatrixT& in, MatrixT* unitary, MatrixT* positive){
	assert(unitary); assert(positive);
	Eigen::JacobiSVD<MatrixT> svd(in, Eigen::ComputeThinU | Eigen::ComputeThinV);
	MatrixT mU, mV, mS;
	mU = svd.matrixU();
		mV = svd.matrixV();
		mS = svd.singularValues().asDiagonal();

	*unitary=mU * mV.adjoint();
	*positive=mV * mS * mV.adjoint();
}

template<typename MatrixT>
void Matrix_SVD(const MatrixT& in, MatrixT* mU, MatrixT* mS, MatrixT* mV){
	assert(mU); assert(mS);  assert(mV);
	Eigen::JacobiSVD<MatrixT> svd(in, Eigen::ComputeThinU | Eigen::ComputeThinV);
	*mU = svd.matrixU();
	*mV = svd.matrixV();
	*mS = svd.singularValues().asDiagonal();
}

template<typename MatrixT>
void matrixEigenDecomposition(const MatrixT& m, MatrixT& mRot, MatrixT& mDiag){
	//assert(mRot); assert(mDiag);
	Eigen::SelfAdjointEigenSolver<MatrixT> a(m); mRot=a.eigenvectors(); mDiag=a.eigenvalues().asDiagonal();
}


bool MatrixXr_pseudoInverse(const MatrixXr &a, MatrixXr &a_pinv, double epsilon=std::numeric_limits<MatrixXr::Scalar>::epsilon());

/*
 * Extra sudodem math functions and classes
 */


/* convert Vector6r in the Voigt notation to corresponding 2nd order symmetric tensor (stored as Matrix3r)
	if strain is true, then multiply non-diagonal parts by .5
*/
template<typename Scalar>
MATRIX3_TEMPLATE(Scalar) voigt_toSymmTensor(const VECTOR6_TEMPLATE(Scalar)& v, bool strain=false){
	Real k=(strain?.5:1.);
	MATRIX3_TEMPLATE(Scalar) ret; ret<<v[0],k*v[5],k*v[4], k*v[5],v[1],k*v[3], k*v[4],k*v[3],v[2]; return ret;
}
/* convert 2nd order tensor to 6-vector (Voigt notation), symmetrizing the tensor;
	if strain is true, multiply non-diagonal compoennts by 2.
*/
template<typename Scalar>
VECTOR6_TEMPLATE(Scalar) tensor_toVoigt(const MATRIX3_TEMPLATE(Scalar)& m, bool strain=false){
	int k=(strain?2:1);
	VECTOR6_TEMPLATE(Scalar) ret; ret<<m(0,0),m(1,1),m(2,2),k*.5*(m(1,2)+m(2,1)),k*.5*(m(2,0)+m(0,2)),k*.5*(m(0,1)+m(1,0)); return ret;
}



[[maybe_unused]]
const Real NaN(std::numeric_limits<Real>::signaling_NaN());

// void quaternionToEulerAngles (const Quaternionr& q, Vector3r& eulerAngles,Real threshold=1e-6f);
template<typename Scalar> void quaterniontoGLMatrix(const Quaternion<Scalar>& q, Scalar m[16]){
	Scalar w2=2.*q.w(), x2=2.*q.x(), y2=2.*q.y(), z2=2.*q.z();
	Scalar x2w=w2*q.w(), y2w=y2*q.w(), z2w=z2*q.w();
	Scalar x2x=x2*q.x(), y2x=y2*q.x(), z2x=z2*q.x();
	Scalar x2y=y2*q.y(), y2y=y2*q.y(), z2y=z2*q.y();
	Scalar x2z=z2*q.z(), y2z=y2*q.z(), z2z=z2*q.z();
	m[0]=1.-(y2y+z2z); m[4]=y2x-z2w;      m[8]=z2x+y2w;       m[12]=0;
	m[1]=y2x+z2w;      m[5]=1.-(x2x+z2z); m[9]=z2y-x2w;       m[13]=0;
	m[2]=z2x-y2w;      m[6]=z2y+x2w;      m[10]=1.-(x2x+y2y); m[14]=0;
	m[3]=0.;           m[7]=0.;           m[11]=0.;           m[15]=1.;
}



// se3
template <class Scalar>
class Se3
{
	public :
		VECTOR3_TEMPLATE(Scalar)	position;
		Quaternion<Scalar>	orientation;
		Se3(){};
		Se3(VECTOR3_TEMPLATE(Scalar) rkP, Quaternion<Scalar> qR){ position = rkP; orientation = qR; }
		Se3(const Se3<Scalar>& s){position = s.position;orientation = s.orientation;}
		Se3(Se3<Scalar>& a,Se3<Scalar>& b){
			position  = b.orientation.inverse()*(a.position - b.position);
			orientation = b.orientation.inverse()*a.orientation;
		}
		Se3<Scalar> inverse(){ return Se3(-(orientation.inverse()*position), orientation.inverse());}
		void toGLMatrix(float m[16]){ orientation.toGLMatrix(m); m[12] = position[0]; m[13] = position[1]; m[14] = position[2];}
		VECTOR3_TEMPLATE(Scalar) operator * (const VECTOR3_TEMPLATE(Scalar)& b ){return orientation*b+position;}
		Se3<Scalar> operator * (const Quaternion<Scalar>& b ){return Se3<Scalar>(position , orientation*b);}
		Se3<Scalar> operator * (const Se3<Scalar>& b ){return Se3<Scalar>(orientation*b.position+position,orientation*b.orientation);}
};

// se2
template <class Scalar>
class Se2
{
	public :
		VECTOR2_TEMPLATE(Scalar)	position;
		Rotation2D<Scalar>	rotation;
		Se2(){};
		Se2(VECTOR2_TEMPLATE(Scalar) rkP, Rotation2D<Scalar> qR){ position = rkP; rotation = qR; }
		Se2(const Se2<Scalar>& s){position = s.position;rotation = s.rotation;}
		/*Se2(Se2<Scalar>& a,Se2<Scalar>& b){
			position  = b.orientation.inverse()*(a.position - b.position);
			orientation = b.orientation.inverse()*a.orientation;
		}*/
		//Se3<Scalar> inverse(){ return Se3(-(orientation.inverse()*position), orientation.inverse());}
		//void toGLMatrix(float m[16]){ orientation.toGLMatrix(m); m[12] = position[0]; m[13] = position[1]; m[14] = position[2];}
		//VECTOR3_TEMPLATE(Scalar) operator * (const VECTOR3_TEMPLATE(Scalar)& b ){return orientation*b+position;}
		//Se3<Scalar> operator * (const Quaternion<Scalar>& b ){return Se3<Scalar>(position , orientation*b);}
		//Se3<Scalar> operator * (const Se3<Scalar>& b ){return Se3<Scalar>(orientation*b.position+position,orientation*b.orientation);}
};

// functions
template<typename Scalar> Scalar unitVectorsAngle(const VECTOR3_TEMPLATE(Scalar)& a, const VECTOR3_TEMPLATE(Scalar)& b){ return acos(a.dot(b)); }
// operators


/*
 * Mask
 */
#ifdef SUDODEM_MASK_ARBITRARY
typedef std::bitset<SUDODEM_MASK_ARBITRARY_SIZE> mask_t;
bool operator==(const mask_t& g, int i);
bool operator==(int i, const mask_t& g);
bool operator!=(const mask_t& g, int i);
bool operator!=(int i, const mask_t& g);
mask_t operator&(const mask_t& g, int i);
mask_t operator&(int i, const mask_t& g);
mask_t operator|(const mask_t& g, int i);
mask_t operator|(int i, const mask_t& g);
bool operator||(const mask_t& g, bool b);
bool operator||(bool b, const mask_t& g);
bool operator&&(const mask_t& g, bool b);
bool operator&&(bool b, const mask_t& g);
#else
typedef int mask_t;
#endif

//some functions

typedef float          DT_Scalar;


#define USE_DOUBLES
#ifdef USE_DOUBLES
typedef double   SD_Scalar;
#else
typedef float    SD_Scalar;
#endif

template <class T>
inline const T& Math_min(const T& a, const T& b)
{
  return b < a ? b : a;
}

template <class T>
inline const T& Math_max(const T& a, const T& b)
{
  return  a < b ? b : a;
}

template <class T>
inline void Math_set_min(T& a, const T& b)
{
    if (b < a) {a = b;}
}

template <class T>
inline void Math_set_max(T& a, const T& b)
{
    if (a < b) {a = b;}
}

template <typename Data, typename Size = size_t>
class DT_Array {
public:
	DT_Array()
      :	m_count(0),
		m_data(0)
	{}

	explicit DT_Array(Size count)
	  :	m_count(count),
		m_data(new Data[count])
	{
		assert(m_data);
	}

	DT_Array(Size count, const Data *data)
	  :	m_count(count),
		m_data(new Data[count])
	{
		assert(m_data);
		std::copy(&data[0], &data[count], m_data);
	}

	~DT_Array()
	{
		delete [] m_data;
	}

	const Data& operator[](int i) const { return m_data[i]; }
	Data&       operator[](int i)       { return m_data[i]; }

	Size size() const { return m_count; }

private:
	DT_Array(const DT_Array&);
	DT_Array& operator=(const DT_Array&);

	Size  m_count;
	Data *m_data;
};

namespace MT {

	template <typename Scalar>
	class Transform {
		enum {
			TRANSLATION = 0x01,
			ROTATION    = 0x02,
			RIGID       = TRANSLATION | ROTATION,
			SCALING     = 0x04,
			LINEAR      = ROTATION | SCALING,
			AFFINE      = TRANSLATION | LINEAR
		};

	public:
		Transform() {}

		template <typename Scalar2>
		explicit Transform(const Scalar2 *m) { setValue(m); }

		explicit Transform(const Quaternionr& q,
						   const Vector3r& c = Vector3r::Zero())
			: m_basis(q.toRotationMatrix()),
			  m_origin(c),
			  m_type(RIGID)
		{}

		explicit Transform(const Matrix3r& b,
						   const Vector3r& c = Vector3r::Zero(),
						   unsigned int type = AFFINE)
			: m_basis(b),
			  m_origin(c),
			  m_type(type)
		{}

		Vector3r operator()(const Vector3r& x) const
		{
			return Vector3r(m_basis.row(0).dot(x) + m_origin[0],
								   m_basis.row(1).dot(x) + m_origin[1],
								   m_basis.row(2).dot(x) + m_origin[2]);
		}

		Vector3r operator*(const Vector3r& x) const
		{
			return (*this)(x);
		}

		Matrix3r&       getBasis()          { return m_basis; }
		const Matrix3r& getBasis()    const { return m_basis; }

		Vector3r&         getOrigin()         { return m_origin; }
		const Vector3r&   getOrigin()   const { return m_origin; }

		Quaternionr getRotation() const { return Quaternionr(m_basis); }
		template <typename Scalar2>
		void setValue(const Scalar2 *m)
		{
			m_basis <<
				m[0], m[4], m[8],
				m[1], m[5], m[9],
				m[2], m[6], m[10];

			m_origin = Vector3r(m[12],m[13],m[14]);
			m_type = AFFINE;
		}

		template <typename Scalar2>
		void getValue(Scalar2 *m) const
		{
			m[0]  = Scalar2(m_basis(0,0));
			m[1]  = Scalar2(m_basis(1,0));
			m[2]  = Scalar2(m_basis(2,0));
			m[3]  = Scalar2(0.0);
			m[4]  = Scalar2(m_basis(0,1));
			m[5]  = Scalar2(m_basis(1,1));
			m[6]  = Scalar2(m_basis(2,1));
			m[7]  = Scalar2(0.0);
			m[8]  = Scalar2(m_basis(0,2));
			m[9]  = Scalar2(m_basis(1,2));
			m[10] = Scalar2(m_basis(2,2));
			m[11] = Scalar2(0.0);
			//
			m[12] = Scalar2(m_origin[0]);
			m[13] = Scalar2(m_origin[1]);
			m[14] = Scalar2(m_origin[2]);
			//
			m[15] = Scalar2(1.0);
		}
		/*template <typename Scalar2>
		void setValue(const Scalar2 *m)
		{
			m_basis.setValue(m);
			m_origin.setValue(&m[12]);
			m_type = AFFINE;
		}*/
		void setOrigin(const Vector3r& origin)
		{
			m_origin = origin;
			m_type |= TRANSLATION;
		}

		void setBasis(const Matrix3r& basis)
		{
			m_basis = basis;
			m_type |= LINEAR;
		}

		void setRotation(const Quaternionr& q)
		{
			m_basis = q.toRotationMatrix();
			m_type = (m_type & ~LINEAR) | ROTATION;
		}

    	void scale(const Vector3r& scaling)
		{
			m_basis.col(0) *= scaling[0];
			m_basis.col(1) *= scaling[1];
			m_basis.col(2) *= scaling[2];
			m_type |= SCALING;
		}

		void setIdentity()
		{
			m_basis = Matrix3r::Identity();
			m_origin = Vector3r::Zero();
			m_type = 0x0;
		}

		bool isIdentity() const { return m_type == 0x0; }

		Transform<Scalar>& operator*=(const Transform<Scalar>& t)
		{
			m_origin += m_basis * t.m_origin;
			m_basis *= t.m_basis;
			m_type |= t.m_type;
			return *this;
		}

		Transform<Scalar> inverse() const
		{
			Matrix3r inv;
			if(m_type & SCALING){
                inv = getRotation().conjugate().toRotationMatrix();
			}else{
                inv = m_basis.transpose();
			}
			return Transform<Scalar>(inv, inv * -m_origin, m_type);
		}

		Transform<Scalar> inverseTimes(const Transform<Scalar>& t) const;

		Transform<Scalar> operator*(const Transform<Scalar>& t) const;

	private:

		Matrix3r m_basis;
		Vector3r  m_origin;
		unsigned int      m_type;
	};


	template <typename Scalar>
	inline Transform<Scalar>
	Transform<Scalar>::inverseTimes(const Transform<Scalar>& t) const
	{
		Vector3r v = t.getOrigin() - m_origin;
		if (m_type & SCALING)
		{
			Matrix3r inv = m_basis.inverse();
			return Transform<Scalar>(inv * t.getBasis(), inv * v,
									 m_type | t.m_type);
		}
		else
		{
			return Transform<Scalar>(m_basis.transpose()*t.m_basis,
									 v * m_basis, m_type | t.m_type);
		}
	}

	template <typename Scalar>
	inline Transform<Scalar>
	Transform<Scalar>::operator*(const Transform<Scalar>& t) const
	{
		return Transform<Scalar>(m_basis * t.m_basis,
								 (*this)(t.m_origin),
								 m_type | t.m_type);
	}
}

class DT_Accuracy {
public:
	static SD_Scalar rel_error2; // squared relative error in the computed distance
	static SD_Scalar depth_tolerance; // terminate EPA if upper_bound <= depth_tolerance * dist2
	static SD_Scalar tol_error; // error tolerance if the distance is almost zero

	static void setAccuracy(SD_Scalar rel_error)
	{
		rel_error2 = rel_error * rel_error;
		depth_tolerance = SD_Scalar(1.0) + SD_Scalar(2.0) * rel_error;
	}

	static void setTolerance(SD_Scalar epsilon)
	{
		tol_error = epsilon;
	}
};

/*
 * typedefs
 */
typedef Se3<Real> Se3r;
typedef Se2<Real> Se2r;

/*
 * Serialization of math classes
 */


#include<cereal/cereal.hpp>
#include<cereal/types/array.hpp>
// Not using multi_array, using Cereal instead

// fast serialization (no version info and no tracking) for basic math types
// Using Cereal for serialization
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector2r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector2i);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector3r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector3i);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector6r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Vector6i);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Quaternionr);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Rotationr);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Se2r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Se3r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Matrix2r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Matrix3r);
// CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Matrix6r);

// Cereal serialization for Eigen types in Eigen namespace (for ADL)
namespace Eigen {

// Rotation2D serialization
template<class Archive, typename _Scalar>
void serialize(Archive & ar, Rotation2D<_Scalar> & g, const unsigned int version)
{
	_Scalar angle = g.angle();
	ar(cereal::make_nvp("angle", angle));
	if(Archive::is_loading::value) {
		g = Rotation2D<_Scalar>(angle);
	}
}

// Quaternion serialization
template<class Archive, typename _Scalar>
void serialize(Archive & ar, Quaternion<_Scalar> & q, const unsigned int version)
{
	_Scalar w = q.w(), x = q.x(), y = q.y(), z = q.z();
	ar(cereal::make_nvp("w", w));
	ar(cereal::make_nvp("x", x));
	ar(cereal::make_nvp("y", y));
	ar(cereal::make_nvp("z", z));
	if(Archive::is_loading::value) {
		q = Quaternion<_Scalar>(w, x, y, z);
	}
}

// Matrix serialization helpers
namespace internal {

template<typename Archive, typename Derived>
void serialize_matrix(Archive & ar, MatrixBase<Derived> & m, const unsigned int version)
{
	typedef typename Derived::Scalar Scalar;
	typedef typename Derived::Index Index;
	Index rows = m.rows(), cols = m.cols();
	ar(cereal::make_nvp("rows", rows));
	ar(cereal::make_nvp("cols", cols));
	if(Archive::is_loading::value) {
		m.derived().resize(rows, cols);
	}
	for(Index i = 0; i < rows; ++i) {
		for(Index j = 0; j < cols; ++j) {
			Scalar val = m(i, j);
			ar(cereal::make_nvp(std::string("m") + std::to_string(i) + "_" + std::to_string(j), val));
			if(Archive::is_loading::value) {
				m(i, j) = val;
			}
		}
	}
}

} // namespace internal

// Fixed-size matrix serializations
template<class Archive, typename _Scalar, int _Rows, int _Cols>
void serialize(Archive & ar, Matrix<_Scalar, _Rows, _Cols> & m, const unsigned int version)
{
	for(int i = 0; i < _Rows; ++i) {
		for(int j = 0; j < _Cols; ++j) {
			_Scalar val = m(i, j);
			ar(cereal::make_nvp(std::string("m") + std::to_string(i) + "_" + std::to_string(j), val));
			if(Archive::is_loading::value) {
				m(i, j) = val;
			}
		}
	}
}

// Vector serialization (fixed-size)
template<class Archive, typename _Scalar, int _Rows>
void serialize(Archive & ar, Matrix<_Scalar, _Rows, 1> & v, const unsigned int version)
{
	for(int i = 0; i < _Rows; ++i) {
		_Scalar val = v(i);
		ar(cereal::make_nvp(std::string("v") + std::to_string(i), val));
		if(Archive::is_loading::value) {
			v(i) = val;
		}
	}
}

// Dynamic-size matrix serialization
template<class Archive, typename _Scalar>
void serialize(Archive & ar, Matrix<_Scalar, Dynamic, Dynamic> & m, const unsigned int version)
{
	typedef typename Matrix<_Scalar, Dynamic, Dynamic>::Index Index;
	Index rows = m.rows(), cols = m.cols();
	ar(cereal::make_nvp("rows", rows));
	ar(cereal::make_nvp("cols", cols));
	if(Archive::is_loading::value) {
		m.resize(rows, cols);
	}
	for(Index i = 0; i < rows; ++i) {
		for(Index j = 0; j < cols; ++j) {
			_Scalar val = m(i, j);
			ar(cereal::make_nvp(std::string("m") + std::to_string(i) + "_" + std::to_string(j), val));
			if(Archive::is_loading::value) {
				m(i, j) = val;
			}
		}
	}
}

// Dynamic-size vector serialization
template<class Archive, typename _Scalar>
void serialize(Archive & ar, Matrix<_Scalar, Dynamic, 1> & v, const unsigned int version)
{
	typedef typename Matrix<_Scalar, Dynamic, 1>::Index Index;
	Index size = v.size();
	ar(cereal::make_nvp("size", size));
	if(Archive::is_loading::value) {
		v.resize(size);
	}
	for(Index i = 0; i < size; ++i) {
		_Scalar val = v(i);
		ar(cereal::make_nvp(std::string("v") + std::to_string(i), val));
		if(Archive::is_loading::value) {
			v(i) = val;
		}
	}
}

} // namespace Eigen

// Cereal serialization for Se2 and Se3 in global namespace (for ADL)
template<class Archive, typename RealT>
void serialize(Archive & ar, Se2<RealT> & g, const unsigned int version)
{
	ar(cereal::make_nvp("position", g.position));
	ar(cereal::make_nvp("rotation", g.rotation));
}

template<class Archive, typename RealT>
void serialize(Archive & ar, Se3<RealT> & g, const unsigned int version)
{
	ar(cereal::make_nvp("position", g.position));
	ar(cereal::make_nvp("orientation", g.orientation));
}

#if 0
// revert optimization options back
#if defined(__GNUG__) && __GNUC__ >= 4 && __GNUC_MINOR__ >=4
	#pragma GCC pop_options
#endif
#endif
