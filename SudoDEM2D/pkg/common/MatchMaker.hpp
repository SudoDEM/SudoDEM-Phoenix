// 2010 © Václav Šmilauer <eudoxos@arcig.cz>
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
		
		virtual void pyRegisterClass(pybind11::module_ _module) override {
			pybind11::class_<MatchMaker, Serializable, std::shared_ptr<MatchMaker>> _classObj(_module, "MatchMaker", "Class matching pair of ids to return pre-defined (for a pair of ids defined in :yref:`matches<MatchMaker.matches>`) or derived value (computed using :yref:`algo<MatchMaker.algo>`) of a scalar parameter. It can be called (``id1``, ``id2``, ``val1=NaN``, ``val2=NaN``) in both python and c++. \n\n.. note:: There is a :ref:`converter <customconverters>` from python number defined for this class, which creates a new :yref:`MatchMaker` returning the value of that number; instead of giving the object instance therefore, you can only pass the number value and it will be converted automatically.");
			_classObj.def(pybind11::init<>());
			_classObj.def(pybind11::init<std::string>());
			_classObj.def(pybind11::init<Real>());
			_classObj.def_readwrite("matches", &MatchMaker::matches, "Array of ``(id1,id2,value)`` items; queries matching ``id1`` + ``id2`` or ``id2`` + ``id1`` will return ``value``");
			_classObj.def_readwrite("algo", &MatchMaker::algo, "Alogorithm used to compute value when no match for ids is found. Possible values are\n\n* 'avg' (arithmetic average)\n* 'min' (minimum value)\n* 'max' (maximum value)\n* 'harmAvg' (harmonic average)\n\nThe following algo algorithms do *not* require meaningful input values in order to work:\n\n* 'val' (return value specified by :yref:`val<MatchMaker.val>`)\n* 'zero' (always return 0.)\n\n");
			_classObj.def_readwrite("val", &MatchMaker::val, "Constant value returned if there is no match and :yref:`algo<MatchMaker::algo>` is ``val``");
			_classObj.def("__call__", &MatchMaker::operator(), pybind11::arg("id1"), pybind11::arg("id2"), pybind11::arg("val1")=NaN, pybind11::arg("val2")=NaN, "Ask the instance for scalar value for given pair *id1*,*id2* (the order is irrelevant). Optionally, *val1*, *val2* can be given so that if there is no :yref:`match<MatchMaker.matches>`, return value can be computed using given :yref:`algo<MatchMaker.algo>`. If there is no match and *val1*, *val2* are not given, an exception is raised.");
			_classObj.def("computeFallback", &MatchMaker::computeFallback, pybind11::arg("val1"), pybind11::arg("val2"), "Compute algo value for *val1* and *val2*, using algorithm specified by :yref:`algo<MatchMaker.algo>`.");

			// Register implicit conversion from Real to shared_ptr<MatchMaker>
			pybind11::implicitly_convertible<Real, std::shared_ptr<MatchMaker>>();
		}
};
REGISTER_SERIALIZABLE(MatchMaker);
