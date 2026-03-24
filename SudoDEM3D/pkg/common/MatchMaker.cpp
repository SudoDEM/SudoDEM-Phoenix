// 2010 © Vaclav Smilauer <eudoxos@arcig.cz>

#include <sudodem/pkg/common/MatchMaker.hpp>


void MatchMaker::pyRegisterClass(pybind11::module_ _module) {
	pybind11::class_<MatchMaker, Serializable, std::shared_ptr<MatchMaker>> _classObj(_module, "MatchMaker", "Class matching pair of ids to return pre-defined (for a pair of ids defined in :yref:`matches<MatchMaker.matches>`) or derived value (computed using :yref:`algo<MatchMaker.algo>`) of a scalar parameter. It can be called (``id1``, ``id2``, ``val1=NaN``, ``val2=NaN``) in both python and c++. \n\n.. note:: There is a :ref:`converter <customconverters>` from python number defined for this class, which creates a new :yref:`MatchMaker` returning the value of that number; instead of giving the object instance therefore, you can only pass the number value and it will be converted automatically.");
	_classObj.def(pybind11::init<>());
	_classObj.def(pybind11::init<std::string>());
	_classObj.def(pybind11::init<Real>());
	_classObj.def_readwrite("matches", &MatchMaker::matches, "Array of ``(id1,id2,value)`` items; queries matching ``id1`` + ``id2`` or ``id2`` + ``id1`` will return ``value``");
	_classObj.def_readwrite("algo", &MatchMaker::algo, "Alogorithm used to compute value when no match for ids is found. Possible values are\n\n* 'avg' (arithmetic average)\n* 'min' (minimum value)\n* 'max' (maximum value)\n* 'harmAvg' (harmonic average)\n\nThe following algo algorithms do *not* require meaningful input values in order to work:\n\n* 'val' (return value specified by :yref:`val<MatchMaker.val>`)\n* 'zero' (always return 0.)\n\n");
	_classObj.def_readwrite("val", &MatchMaker::val, "Constant value returned if there is no match and :yref:`algo<MatchMaker::algo>` is ``val``");
	_classObj.def("__call__", &MatchMaker::operator(), pybind11::arg("id1"), pybind11::arg("id2"), pybind11::arg("val1")=NaN, pybind11::arg("val2")=NaN, "Ask the instance for scalar value for given pair *id1*,*id2* (the order is irrelevant). Optionally, *val1*, *val2* can be given so that if there is no :yref:`match<MatchMaker.matches>`, return value can be computed using given :yref:`algo<MatchMaker.algo>`. If there is no match and *val1*, *val2* are not given, an exception is raised.");
	_classObj.def("computeFallback", &MatchMaker::computeFallback, pybind11::arg("val1"), pybind11::arg("val2"), "Compute algo value for *val1* and *val2*, using algorithm specified by :yref:`algo<MatchMaker.algo>`.");
}

Real MatchMaker::operator()(int id1, int id2, Real val1, Real val2) const {
	for(const Vector3r& m : matches){
		if(((int)m[0]==id1 && (int)m[1]==id2) || ((int)m[0]==id2 && (int)m[1]==id1)) return m[2];
	}
	// no match
	if(fbNeedsValues && (isnan(val1) || isnan(val2))) throw std::invalid_argument("MatchMaker: no match for ("+std::to_string(id1)+","+std::to_string(id2)+"), and values required for algo computation '"+algo+"' not specified.");
	return computeFallback(val1,val2);
}

void MatchMaker::postLoad(MatchMaker&){
	if(algo=="val")      { fbPtr=&MatchMaker::fbVal; fbNeedsValues=false; }
	else if(algo=="zero"){ fbPtr=&MatchMaker::fbZero;fbNeedsValues=false; }
	else if(algo=="avg") { fbPtr=&MatchMaker::fbAvg; fbNeedsValues=true;  }
	else if(algo=="min") { fbPtr=&MatchMaker::fbMin; fbNeedsValues=true;  }
	else if(algo=="max") { fbPtr=&MatchMaker::fbMax; fbNeedsValues=true;  }
	else if(algo=="harmAvg") { fbPtr=&MatchMaker::fbHarmAvg; fbNeedsValues=true; }
	else throw std::invalid_argument("MatchMaker:: algo '"+algo+"' not recognized (possible values: val, avg, min, max, harmAvg).");
}

Real MatchMaker::computeFallback(Real v1, Real v2) const { return (this->*MatchMaker::fbPtr)(v1,v2); }
