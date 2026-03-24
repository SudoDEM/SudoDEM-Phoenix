#pragma once
#include<sudodem/lib/base/openmp-accu.hpp>
#include<sudodem/lib/serialization/Serializable.hpp>
#include<pybind11/pybind11.h>

namespace py=pybind11;

class EnergyTracker: public Serializable{
	public:
	~EnergyTracker();
	void findId(const std::string& name, int& id, bool reset=false, bool newIfNotFound=true){
		if(id>0) return; // the caller should have checked this already
		if(names.count(name)) id=names[name];
		else if(newIfNotFound) {
			#ifdef SUDODEM_OPENMP
				#pragma omp critical
			#endif
				{ energies.resize(energies.size()+1); id=energies.size()-1; resetStep.resize(id+1); resetStep[id]=reset; names[name]=id; assert(id<(int)energies.size()); assert(id>=0); }
		}
	}
	// set value of the accumulator; note: must NOT be called from parallel sections!
	void set(const Real& val, const std::string& name, int &id){
		if(id<0) findId(name,id,/* do not reset value that is set directly */ false);
		energies.set(id,val);
	}
	// add value to the accumulator; safely called from parallel sections
	void add(const Real& val, const std::string& name, int &id, bool reset=false){
		if(id<0) findId(name,id,reset);
		energies.add(id,val);
	}
	Real getItem_py(const std::string& name){
		int id=-1; findId(name,id,false,false);
					if (id<0) {PyErr_SetString(PyExc_KeyError,("Unknown energy name '"+name+"'.").c_str());  throw py::error_already_set(); }		return energies.get(id);
	}
	void setItem_py(const std::string& name, Real val){
		int id=-1; set(val,name,id);
	}
	void clear(){ energies.clear(); names.clear(); resetStep.clear();}
	void resetResettables(){ size_t sz=energies.size(); for(size_t id=0; id<sz; id++){ if(resetStep[id]) energies.reset(id); } }

	Real total() const { Real ret=0; size_t sz=energies.size(); for(size_t id=0; id<sz; id++) ret+=energies.get(id); return ret; };
	py::list keys_py() const { py::list ret; for (const auto& p : names) ret.append(p.first); return ret; };
	py::list items_py() const { py::list ret; for (const auto& p : names) ret.append(py::make_tuple(p.first,energies.get(p.second))); return ret; };
	py::dict perThreadData() const {
		py::dict ret;
		std::vector<std::vector<Real> > dta=energies.getPerThreadData();
		for (const auto& p : names) ret[py::cast(p.first)] = py::cast(dta[p.second]);
		return ret;
  };

	typedef std::map<std::string,int> mapStringInt;
	typedef std::pair<std::string,int> pairStringInt;

	// Member variables for serialization
	OpenMPArrayAccumulator<Real> energies;
	mapStringInt names;
	vector<bool> resetStep;

	// Cereal serialization
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int version) {
		ar(cereal::base_class<Serializable>(this));
		ar(CEREAL_NVP(energies));
		ar(CEREAL_NVP(names));
		ar(CEREAL_NVP(resetStep));
	}

	public:
		SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module) override {
			checkPyClassRegistersItself("EnergyTracker");
			pybind11::class_<EnergyTracker, Serializable, std::shared_ptr<EnergyTracker>> _classObj(_module, "EnergyTracker", "Storage for tracing energies. Only to be used if :yref:`O.trackEnergy<Omega.trackEnergy>` is True.");
			_classObj.def(pybind11::init<>());
			_classObj.def_readwrite("energies", &EnergyTracker::energies, "Energy values, in linear array");
			_classObj.def_readwrite("names", &EnergyTracker::names, "Associate textual name to an index in the energies array.");
			_classObj.def_readwrite("resetStep", &EnergyTracker::resetStep, "Whether the respective energy value should be reset at every step.");
			_classObj.def("__getitem__", &EnergyTracker::getItem_py, "Get energy value for given name.");
			_classObj.def("__setitem__", &EnergyTracker::setItem_py, "Set energy value for given name (will create a non-resettable item, if it does not exist yet).");
			_classObj.def("clear", &EnergyTracker::clear, "Clear all stored values.");
			_classObj.def("keys", &EnergyTracker::keys_py, "Return defined energies.");
			_classObj.def("items", &EnergyTracker::items_py, "Return contents as list of (name,value) tuples.");
			_classObj.def("total", &EnergyTracker::total, "Return sum of all energies.");
			_classObj.def_property_readonly("_perThreadData", &EnergyTracker::perThreadData, "Contents as dictionary, where each value is tuple of individual threads' values (for debugging)");
		}
};
REGISTER_SERIALIZABLE(EnergyTracker);