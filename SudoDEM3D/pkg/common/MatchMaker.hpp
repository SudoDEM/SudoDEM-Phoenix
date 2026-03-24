// 2010 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once
#include<sudodem/lib/serialization/Serializable.hpp>

/* Future optimizations, in postLoad:

1. Use matches to update lookup table/hash for faster matching, instead of traversing matches every time
2. Use algo to update fbPtr, instead of string-comparison of algo every time

*/
class MatchMaker: public Serializable {
	#if 1
		Real fbZero(Real v1, Real v2) const{ return 0.; }
		Real fbAvg(Real v1, Real v2) const{ return (v1+v2)/2.; }
		Real fbMin(Real v1, Real v2) const{ return min(v1,v2); }
		Real fbMax(Real v1, Real v2) const{ return max(v1,v2); }
		Real fbHarmAvg(Real v1, Real v2) const { return 2*v1*v2/(v1+v2); }
		Real fbVal(Real v1, Real v2) const { return val; }
		Real (MatchMaker::*fbPtr)(Real,Real) const;
		// whether the current fbPtr function needs valid input values in order to give meaningful result.
		// must be kept in sync with fbPtr, which is done in postLoad
		bool fbNeedsValues;
	#endif
	public:
		std::vector<Vector3r> matches;
		std::string algo;
		Real val;
		
		virtual ~MatchMaker() {};
		MatchMaker(): algo("avg"), val(NaN) { postLoad(*this); }
		MatchMaker(std::string _algo): algo(_algo){ postLoad(*this); }
		MatchMaker(Real _val): algo("val"), val(_val){ postLoad(*this); }
		Real computeFallback(Real val1, Real val2) const ;
		void postLoad(MatchMaker&);
		// return value looking up matches for id1+id2 (the order is arbitrary)
		// if no match is found, use val1,val2 and algo strategy to compute new value.
		// if no match is found and val1 or val2 are not given, throw exception
		Real operator()(const int id1, const int id2, const Real val1=NaN, const Real val2=NaN) const;
		
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override;
};
REGISTER_SERIALIZABLE(MatchMaker);
