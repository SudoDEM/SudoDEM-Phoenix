/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  Copyright (C) 2024 - SudoDEM Project
*
*  Simplified Serializable class for pybind11 compatibility
*  Uses Cereal for serialization
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*************************************************************************/

#pragma once
#include <sudodem/lib/base/Math.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <sudodem/lib/factory/Factorable.hpp>
#include <sudodem/lib/factory/ClassRegistry.hpp>
#include <sudodem/lib/serialization/RegistrationMacros.hpp>
#include <sudodem/lib/pyutil/doc_opts.hpp>
// Include cereal archives before polymorphic to ensure proper type registration
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/polymorphic.hpp>

// Attribute flags namespace
namespace sudodem {
    namespace Attr {
        // Keep in sync with py/wrapper/sudodemWrapper.cpp
        enum flags { noSave=1, readonly=2, triggerPostLoad=4, hidden=8, noResize=16 };
    };
}

using namespace sudodem;

// ADL functions for serialization
template<typename T>
void preLoad(T&){}

template<typename T>
void postLoad(T& obj){}

template<typename T>
void preSave(T&){}

template<typename T>
void postSave(T&){}

// Type traits for reference handling
namespace sudodem {
    // By default, do not return reference; return value instead
    template<typename T>
    struct py_wrap_ref : public std::false_type{};

    // Specialize for types that should be returned as references
    template<> struct py_wrap_ref<Vector3r> : public std::true_type{};
    template<> struct py_wrap_ref<Vector3i> : public std::true_type{};
    template<> struct py_wrap_ref<Vector2r> : public std::true_type{};
    template<> struct py_wrap_ref<Vector2i> : public std::true_type{};
    template<> struct py_wrap_ref<Quaternionr> : public std::true_type{};
    template<> struct py_wrap_ref<Matrix3r> : public std::true_type{};

    // Noncopyable class
    class noncopyable {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };
}

// ADL helper for postLoad calls
template<class C, typename T, T C::*A>
void make_setter_postLoad(C& instance, const T& val){
    instance.*A = val;
    instance.callPostLoad();
}

/**
 * Simplified Serializable base class.
 * All classes that need serialization and Python binding should inherit from this.
 */
class Serializable: public Factorable {
public:
    /**
     * Serialization method for Cereal.
     * Override this in derived classes to serialize custom attributes.
     */
    template <class ArchiveT>
    void serialize(ArchiveT & ar, unsigned int version){ }

    /**
     * Cast methods for type-safe downcasting.
     */
    template <class DerivedT>
    const DerivedT& cast() const {
        return *static_cast<DerivedT*>(this);
    }

    template <class DerivedT>
    DerivedT& cast(){
        return *static_cast<DerivedT*>(this);
    }

    /**
     * Constructors and destructor.
     */
    Serializable() {}
    virtual ~Serializable() {}

    /**
     * Comparison operators (compare by address).
     */
    bool operator==(const Serializable& other){ return this==&other; }
    bool operator!=(const Serializable& other){ return this!=&other; }

    /**
     * Python attribute update method.
     * Updates attributes from a Python dictionary.
     */
    void pyUpdateAttrs(const pybind11::dict& d);

    /**
     * Set a single attribute by name.
     * Override in derived classes to handle custom attributes.
     */
    virtual void pySetAttr(const std::string& key, const pybind11::object& value){
        PyErr_SetString(PyExc_AttributeError,
                       (std::string("No such attribute: ") + key + ".").c_str());
        throw pybind11::error_already_set();
    }

    /**
     * Get all attributes as a Python dictionary.
     * Override in derived classes to include custom attributes.
     */
    virtual pybind11::dict pyDict() const {
        return pybind11::dict();
    }

    /**
     * Call postLoad hook.
     * This should be called after updating attributes from Python.
     */
    virtual void callPostLoad(void){
        postLoad(*this);
    }

    /**
     * Check if class registers itself properly.
     * This is used for debugging and validation.
     */
    virtual void checkPyClassRegistersItself(const std::string& thisClassName) const;

    /**
     * Python class registration method.
     * All derived classes must implement this to register themselves with pybind11.
     */
    SUDODEM_PYREGISTER_CLASS_API virtual void pyRegisterClass(pybind11::module_ _module);

    /**
     * Handle custom constructor arguments from Python.
     * Override in derived classes to manipulate constructor arguments.
     */
    virtual void pyHandleCustomCtorArgs(pybind11::tuple& args, pybind11::dict& kw){
        return;
    }

    /**
     * String representation of this object.
     */
    std::string pyStr() {
        return "<" + getClassName() + " instance at " +
               std::to_string(reinterpret_cast<unsigned long long>(this)) + ">";
    }

    REGISTER_CLASS_NAME_DERIVED(Serializable);
    REGISTER_BASE_CLASS_NAME_DERIVED(Factorable);
};

/**
 * Helper function for creating Serializable instances from Python.
 * Handles keyword arguments and attribute initialization.
 */
template <typename T>
shared_ptr<T> Serializable_ctor_kwAttrs(pybind11::tuple& t, pybind11::dict& d){
    shared_ptr<T> instance;
    instance = shared_ptr<T>(new T);

    // Allow derived classes to manipulate constructor arguments
    instance->pyHandleCustomCtorArgs(t, d);

    // Check that no positional arguments remain
    if(pybind11::len(t) > 0){
        throw runtime_error("Zero (not " + std::to_string(pybind11::len(t)) +
                           ") non-keyword constructor arguments required "
                           "[in Serializable_ctor_kwAttrs; Serializable::pyHandleCustomCtorArgs "
                           "might have changed it after your call].");
    }

    // Update attributes from keyword arguments
    if(pybind11::len(d) > 0){
        instance->pyUpdateAttrs(d);
        instance->callPostLoad();
    }

    return instance;
}

/**
 * Helper function for updating attributes from a Python dictionary.
 * This is used by the pyUpdateAttrs method.
 */
template<typename T>
void updateAttrsFromDict(shared_ptr<T> instance, const pybind11::dict& d){
    for(auto item : d){
        std::string key = pybind11::str(item.first).cast<std::string>();
        pybind11::object value = pybind11::reinterpret_borrow<pybind11::object>(item.second);
        instance->pySetAttr(key, value);
    }
}

/**
 * Simplified attribute registration helper for pybind11.
 * This is a convenience function to register properties with proper flags.
 */
template<typename ClassType, typename AttrType>
void registerAttribute(
    pybind11::class_<ClassType, std::shared_ptr<ClassType>>& cls,
    AttrType ClassType::*attr,
    const std::string& name,
    const std::string& doc,
    int flags = 0
){
    bool readonly = (flags & Attr::readonly);
    bool triggerPostLoad = (flags & Attr::triggerPostLoad);
    bool hidden = (flags & Attr::hidden);

    if(hidden) return; // Don't register hidden attributes

    if(readonly){
        cls.def_property_readonly(name.c_str(),
            [attr](const std::shared_ptr<ClassType>& c) -> const AttrType& {
                return c->*attr;
            },
            doc.c_str());
    } else if(triggerPostLoad){
        cls.def_property(name.c_str(),
            [attr](const std::shared_ptr<ClassType>& c) -> const AttrType& {
                return c->*attr;
            },
            [attr](std::shared_ptr<ClassType>& c, const AttrType& value) {
                c->*attr = value;
                c->callPostLoad();
            },
            doc.c_str());
    } else {
        // Check if we should return reference (for vector/matrix types)
        if(sudodem::py_wrap_ref<AttrType>::value){
            cls.def_property(name.c_str(),
                [attr](std::shared_ptr<ClassType>& c) -> AttrType& {
                    return c->*attr;
                },
                [attr](std::shared_ptr<ClassType>& c, const AttrType& value) {
                    c->*attr = value;
                },
                doc.c_str());
        } else {
            cls.def_property(name.c_str(),
                [attr](const std::shared_ptr<ClassType>& c) -> AttrType {
                    return c->*attr;
                },
                [attr](std::shared_ptr<ClassType>& c, const AttrType& value) {
                    c->*attr = value;
                },
                doc.c_str());
        }
    }
}

/**
 * Simplified method registration helper.
 */
template<typename ClassType, typename ReturnType, typename... Args>
void registerMethod(
    pybind11::class_<ClassType, std::shared_ptr<ClassType>>& cls,
    ReturnType (ClassType::*method)(Args...),
    const std::string& name,
    const std::string& doc
){
    cls.def(name.c_str(), method, doc.c_str());
}

/**
 * Simplified static method registration helper.
 */
template<typename ClassType, typename ReturnType, typename... Args>
void registerStaticMethod(
    pybind11::class_<ClassType, std::shared_ptr<ClassType>>& cls,
    ReturnType (*method)(Args...),
    const std::string& name,
    const std::string& doc
){
    cls.def_static(name.c_str(), method, doc.c_str());
}

// Register Serializable with Cereal for serialization
CEREAL_REGISTER_TYPE(Serializable)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Factorable, Serializable)

// REGISTER_SERIALIZABLE: Register with Cereal for serialization
// Note: This assumes the class derives from Serializable directly
// Use CEREAL_REGISTER_TYPE_WITH_NAME to ensure string-based polymorphic serialization works across shared libraries
// Runtime registration is handled by ClassRegistry via pyRegisterClass()
#define REGISTER_SERIALIZABLE(name) CEREAL_REGISTER_TYPE_WITH_NAME(name, #name); CEREAL_REGISTER_POLYMORPHIC_RELATION(Serializable, name)

// REGISTER_SERIALIZABLE_BASE: Register with Cereal for serialization with explicit base class
// Use this when the class derives from something other than Serializable directly
// Runtime registration is handled by ClassRegistry via pyRegisterClass()
#define REGISTER_SERIALIZABLE_BASE(name, base) CEREAL_REGISTER_TYPE_WITH_NAME(name, #name); CEREAL_REGISTER_POLYMORPHIC_RELATION(base, name)

// SUDODEM_CLASS_BASE_DOC: No-op for in-class usage
// Registration happens through pyRegisterClass() method
// This macro is kept for backward compatibility with existing code
#define SUDODEM_CLASS_BASE_DOC(klass,base,doc)

// SUDODEM_CLASS_BASE_DOC_ATTRS_CTOR_PY and variants: No-op for in-class usage
// These macros consumed attributes, constructor, and pybind11 extras in the old system
// In the new system, registration happens through pyRegisterClass() method
#define SUDODEM_CLASS_BASE_DOC_ATTRS_CTOR_PY(klass,base,doc,attrs,ctor,py)
#define SUDODEM_CLASS_BASE_DOC_ATTRS_CTOR(klass,base,doc,attrs,ctor)
#define SUDODEM_CLASS_BASE_DOC_ATTRS(klass,base,doc,attrs)

// REGISTER_ATTRIBUTES: Generate serialize method for cereal
// Usage: REGISTER_ATTRIBUTES(baseClass, attr1, attr2, attr3)
// This creates a serialize method that serializes all listed attributes
#define _SUDODEM_SERIALIZE_ONE_ATTR(ar, attr) ar(CEREAL_NVP_(#attr, attr));

// Helper macros to iterate over attributes (supports up to 20 attributes)
#define _SUDODEM_SERIALIZE_1(ar, a1) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a1)
#define _SUDODEM_SERIALIZE_2(ar, a1, a2) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a1) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a2)
#define _SUDODEM_SERIALIZE_3(ar, a1, a2, a3) _SUDODEM_SERIALIZE_2(ar, a1, a2) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a3)
#define _SUDODEM_SERIALIZE_4(ar, a1, a2, a3, a4) _SUDODEM_SERIALIZE_3(ar, a1, a2, a3) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a4)
#define _SUDODEM_SERIALIZE_5(ar, a1, a2, a3, a4, a5) _SUDODEM_SERIALIZE_4(ar, a1, a2, a3, a4) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a5)
#define _SUDODEM_SERIALIZE_6(ar, a1, a2, a3, a4, a5, a6) _SUDODEM_SERIALIZE_5(ar, a1, a2, a3, a4, a5) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a6)
#define _SUDODEM_SERIALIZE_7(ar, a1, a2, a3, a4, a5, a6, a7) _SUDODEM_SERIALIZE_6(ar, a1, a2, a3, a4, a5, a6) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a7)
#define _SUDODEM_SERIALIZE_8(ar, a1, a2, a3, a4, a5, a6, a7, a8) _SUDODEM_SERIALIZE_7(ar, a1, a2, a3, a4, a5, a6, a7) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a8)
#define _SUDODEM_SERIALIZE_9(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9) _SUDODEM_SERIALIZE_8(ar, a1, a2, a3, a4, a5, a6, a7, a8) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a9)
#define _SUDODEM_SERIALIZE_10(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) _SUDODEM_SERIALIZE_9(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a10)
#define _SUDODEM_SERIALIZE_11(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) _SUDODEM_SERIALIZE_10(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a11)
#define _SUDODEM_SERIALIZE_12(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) _SUDODEM_SERIALIZE_11(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a12)
#define _SUDODEM_SERIALIZE_13(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) _SUDODEM_SERIALIZE_12(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a13)
#define _SUDODEM_SERIALIZE_14(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) _SUDODEM_SERIALIZE_13(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a14)
#define _SUDODEM_SERIALIZE_15(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) _SUDODEM_SERIALIZE_14(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a15)
#define _SUDODEM_SERIALIZE_16(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) _SUDODEM_SERIALIZE_15(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a16)
#define _SUDODEM_SERIALIZE_17(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) _SUDODEM_SERIALIZE_16(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a17)
#define _SUDODEM_SERIALIZE_18(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) _SUDODEM_SERIALIZE_17(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a18)
#define _SUDODEM_SERIALIZE_19(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) _SUDODEM_SERIALIZE_18(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a19)
#define _SUDODEM_SERIALIZE_20(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) _SUDODEM_SERIALIZE_19(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a20)
#define _SUDODEM_SERIALIZE_21(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) _SUDODEM_SERIALIZE_20(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a21)
#define _SUDODEM_SERIALIZE_22(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) _SUDODEM_SERIALIZE_21(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a22)
#define _SUDODEM_SERIALIZE_23(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) _SUDODEM_SERIALIZE_22(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a23)
#define _SUDODEM_SERIALIZE_24(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) _SUDODEM_SERIALIZE_23(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a24)
#define _SUDODEM_SERIALIZE_25(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) _SUDODEM_SERIALIZE_24(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a25)
#define _SUDODEM_SERIALIZE_26(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) _SUDODEM_SERIALIZE_25(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a26)
#define _SUDODEM_SERIALIZE_27(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) _SUDODEM_SERIALIZE_26(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a27)
#define _SUDODEM_SERIALIZE_28(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) _SUDODEM_SERIALIZE_27(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a28)
#define _SUDODEM_SERIALIZE_29(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) _SUDODEM_SERIALIZE_28(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a29)
#define _SUDODEM_SERIALIZE_30(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30) _SUDODEM_SERIALIZE_29(ar, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) _SUDODEM_SERIALIZE_ONE_ATTR(ar, a30)

// VA_NARGS helper to count arguments (supports up to 30)
#define _SUDODEM_VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, N, ...) N
#define _SUDODEM_VA_NARGS(...) _SUDODEM_VA_NARGS_IMPL(__VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Concatenation helper
#define _SUDODEM_CONCAT_IMPL(a, b) a##b
#define _SUDODEM_CONCAT(a, b) _SUDODEM_CONCAT_IMPL(a, b)

// Main macro to serialize all attributes
#define _SUDODEM_SERIALIZE_ATTRS(ar, ...) _SUDODEM_CONCAT(_SUDODEM_SERIALIZE_, _SUDODEM_VA_NARGS(__VA_ARGS__))(ar, __VA_ARGS__)

// REGISTER_ATTRIBUTES: Generate serialize method for cereal
// Usage: REGISTER_ATTRIBUTES(baseClass, attr1, attr2, attr3)
// This creates a serialize method that:
// 1. Serializes the base class
// 2. Calls preLoad/preSave hooks
// 3. Serializes all listed attributes
// 4. Calls postLoad/postSave hooks
#define REGISTER_ATTRIBUTES(baseClass, ...) \
    friend class cereal::access; \
    template<class Archive> \
    void serialize(Archive& ar, unsigned int version) { \
        ar(cereal::base_class<baseClass>(this)); \
        if(Archive::is_loading::value) preLoad(*this); else preSave(*this); \
        _SUDODEM_SERIALIZE_ATTRS(ar, __VA_ARGS__) \
        if(Archive::is_loading::value) postLoad(*this); else postSave(*this); \
    }

// Backward-compatible version for old sequence syntax: (attr1)(attr2)(attr3)
// Note: For seq format, you need to manually add ar(CEREAL_NVP(attr)) for each attribute
#define REGISTER_ATTRIBUTES_SEQ(baseClass, seq) \
    friend class cereal::access; \
    template<class Archive> \
    void serialize(Archive& ar, unsigned int version) { \
        ar(cereal::base_class<baseClass>(this)); \
        if(Archive::is_loading::value) preLoad(*this); else preSave(*this); \
        if(Archive::is_loading::value) postLoad(*this); else postSave(*this); \
    }