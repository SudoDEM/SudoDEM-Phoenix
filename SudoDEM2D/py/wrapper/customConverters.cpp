// 2009 © Václav Šmilauer <eudoxos@arcig.cz>


#include<sudodem/lib/base/Math.hpp>
#include<sudodem/lib/base/openmp-accu.hpp>

#include<sudodem/core/Engine.hpp>

#include<sudodem/pkg/common/Dispatching.hpp>
#include<sudodem/pkg/common/Callbacks.hpp>
//#include<sudodem/pkg/dem/DiskPack.hpp>
//#include<sudodem/pkg/common/KinematicEngines.hpp>
#ifdef SUDODEM_OPENGL
	#include<sudodem/pkg/common/GLDrawFunctors.hpp>
	#include<sudodem/pkg/common/OpenGLRenderer.hpp>
#endif
#include<sudodem/pkg/common/MatchMaker.hpp>

#include<pybind11/pybind11.h>
#include<pybind11/stl.h>

namespace py = pybind11;

// OpenMPAccumulator to/from Python (converts to/from Real)
namespace pybind11 { namespace detail {
    template<typename T> struct type_caster<OpenMPAccumulator<T>> {
    public:
        PYBIND11_TYPE_CASTER(OpenMPAccumulator<T>, const_name("OpenMPAccumulator"));
        
        bool load(handle src, bool) {
            if (!PyNumber_Check(src.ptr())) return false;
            T val;
            if constexpr (std::is_integral_v<T>) {
                val = PyLong_AsLong(src.ptr());
            } else {
                val = PyFloat_AsDouble(src.ptr());
            }
            value = OpenMPAccumulator<T>(val);
            return true;
        }
        
        static handle cast(const OpenMPAccumulator<T>& src, return_value_policy /* policy */, handle /* parent */) {
            T val = src.get();
            if constexpr (std::is_integral_v<T>) {
                return PyLong_FromLong(static_cast<long>(val));
            } else {
                return PyFloat_FromDouble(static_cast<double>(val));
            }
        }
    };
}}

void pybind_init__customConverters(pybind11::module& m){
    // Modern pybind11 automatically handles vectors and other containers,
    // so many of the old custom converters are no longer needed.
    // The pybind11::stl.h header already provides conversions for:
    // - std::vector<T>
    // - std::list<T>
    // - std::map<K,V>
    // - std::set<T>
    // etc.

    // Note: Se2r and other math types are provided by miniEigen library
    // which handles its own type conversions.
}