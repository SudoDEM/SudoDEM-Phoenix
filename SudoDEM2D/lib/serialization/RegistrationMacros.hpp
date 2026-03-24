/*************************************************************************
*  Copyright (C) 2024 - SudoDEM Project
*
*  Simplified Registration Macros for pybind11 compatibility
*  Uses variadic macros for flexibility
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*************************************************************************/

#pragma once
#include <sudodem/lib/factory/ClassRegistry.hpp>
#include <sudodem/lib/serialization/Serializable.hpp>
#include <cereal/types/polymorphic.hpp>

/**
 * Main registration macro for classes without attributes.
 * This is the simplest form and should be used for most classes.
 *
 * Usage:
 *   class MyClass : public Serializable {
 *       SUDODEM_REGISTER_CLASS(MyClass, Serializable);
 *   public:
 *       virtual void pyRegisterClass(pybind11::module_& m) override;
 *   };
 *   SUDODEM_REGISTER_CLASS_IMPL(MyClass, Serializable);
 */
#define SUDODEM_REGISTER_CLASS(ClassName, BaseClass, DocString) \
    using Base = BaseClass;

#define SUDODEM_REGISTER_CLASS_IMPL(ClassName, BaseClass) \
    namespace { \
        struct ClassName##_Registrar { \
            ClassName##_Registrar() { \
                ClassRegistry::instance().registerClassWithBase<ClassName, BaseClass>(__FILE__); \
                ClassRegistry::instance().setBaseClassName(typeid(ClassName).name(), typeid(BaseClass).name()); \
            } \
        } ClassName##_registrar; \
    }

/**
 * Registration macro with custom Python class name.
 * Use this when the C++ and Python class names should differ.
 *
 * Usage:
 *   class MyClass_Template<int> : public Serializable {
 *       SUDODEM_REGISTER_CLASS_PYNAME(MyClass_Template<int>, Serializable, "MyClassInt", "My class description");
 *   public:
 *       static void pyRegisterClass(pybind11::module_& m);
 *   };
 */
#define SUDODEM_REGISTER_CLASS_PYNAME(ClassName, BaseClass, PyClassName, DocString) \
    using Base = BaseClass; \
    static constexpr const char* pyClassName = PyClassName; \
    static void pyRegisterClass(pybind11::module_& m); \
    namespace { \
        struct ClassName##_Registrar { \
            ClassName##_Registrar() { \
                ClassRegistry::instance().registerClassWithBase<ClassName, BaseClass>(__FILE__); \
                ClassRegistry::instance().setBaseClassName(typeid(ClassName).name(), typeid(BaseClass).name()); \
            } \
        } static ClassName##_registrar; \
    }

/**
 * Plugin registration macro for classes in dynamic libraries.
 * This creates an explicit registration function that can be called from C.
 *
 * Usage (in .cpp file):
 *   SUDODEM_REGISTER_PLUGIN(MyClass)
 *
 * Note: This macro is defined in PluginMacros.hpp
 */

/**
 * Simplified plugin macro for multiple classes using variadic arguments.
 *
 * Usage:
 *   SUDODEM_REGISTER_PLUGINS(Class1, Class2, Class3)
 */
#define SUDODEM_REGISTER_PLUGINS(...) \
    _SUDODEM_REGISTER_PLUGINS_IMPL(__VA_ARGS__)

#define _SUDODEM_REGISTER_PLUGINS_IMPL(...) \
    _SUDODEM_REGISTER_PLUGINS_SELECT(__VA_ARGS__, \
        _SUDODEM_REGISTER_P5, _SUDODEM_REGISTER_P4, _SUDODEM_REGISTER_P3, \
        _SUDODEM_REGISTER_P2, _SUDODEM_REGISTER_P1)(__VA_ARGS__)

#define _SUDODEM_REGISTER_PLUGINS_SELECT(_1, _2, _3, _4, _5, N, ...) N

#define _SUDODEM_REGISTER_P1(c1) \
    extern "C" void sudodem_register_##c1() { ClassRegistry::instance().registerClass<c1>(__FILE__); }
#define _SUDODEM_REGISTER_P2(c1, c2) \
    extern "C" void sudodem_register_##c1() { ClassRegistry::instance().registerClass<c1>(__FILE__); } \
    extern "C" void sudodem_register_##c2() { ClassRegistry::instance().registerClass<c2>(__FILE__); }
#define _SUDODEM_REGISTER_P3(c1, c2, c3) \
    extern "C" void sudodem_register_##c1() { ClassRegistry::instance().registerClass<c1>(__FILE__); } \
    extern "C" void sudodem_register_##c2() { ClassRegistry::instance().registerClass<c2>(__FILE__); } \
    extern "C" void sudodem_register_##c3() { ClassRegistry::instance().registerClass<c3>(__FILE__); }
#define _SUDODEM_REGISTER_P4(c1, c2, c3, c4) \
    extern "C" void sudodem_register_##c1() { ClassRegistry::instance().registerClass<c1>(__FILE__); } \
    extern "C" void sudodem_register_##c2() { ClassRegistry::instance().registerClass<c2>(__FILE__); } \
    extern "C" void sudodem_register_##c3() { ClassRegistry::instance().registerClass<c3>(__FILE__); } \
    extern "C" void sudodem_register_##c4() { ClassRegistry::instance().registerClass<c4>(__FILE__); }
#define _SUDODEM_REGISTER_P5(c1, c2, c3, c4, c5) \
    extern "C" void sudodem_register_##c1() { ClassRegistry::instance().registerClass<c1>(__FILE__); } \
    extern "C" void sudodem_register_##c2() { ClassRegistry::instance().registerClass<c2>(__FILE__); } \
    extern "C" void sudodem_register_##c3() { ClassRegistry::instance().registerClass<c3>(__FILE__); } \
    extern "C" void sudodem_register_##c4() { ClassRegistry::instance().registerClass<c4>(__FILE__); } \
    extern "C" void sudodem_register_##c5() { ClassRegistry::instance().registerClass<c5>(__FILE__); }

/**
 * Cereal registration macro.
 * Use this to register classes with Cereal for serialization.
 *
 * Usage:
 *   class MyClass : public Serializable {
 *       SUDODEM_REGISTER_CLASS(MyClass, Serializable, "My class");
 *       SUDODEM_REGISTER_CEREAL(MyClass);
 *   public:
 *       // ...
 *   };
 */
#define SUDODEM_REGISTER_CEREAL(ClassName) \
    CEREAL_REGISTER_TYPE(ClassName)

/**
 * Combined registration macro for Serializable classes.
 * This combines class registration and Cereal registration.
 *
 * Usage:
 *   class MyClass : public Serializable {
 *       SUDODEM_REGISTER_SERIALIZABLE(MyClass, Serializable, "My class description");
 *   public:
 *       static void pyRegisterClass(pybind11::module_& m);
 *   };
 */
#define SUDODEM_REGISTER_SERIALIZABLE(ClassName, BaseClass, DocString) \
    SUDODEM_REGISTER_CLASS(ClassName, BaseClass, DocString) \
    SUDODEM_REGISTER_CEREAL(ClassName)

/**
 * Helper macro to get the Python class name.
 * Returns pyClassName if defined, otherwise returns the C++ class name.
 */
#define SUDODEM_GET_PYCLASSNAME(ClassName) \
    (ClassName::pyClassName ? ClassName::pyClassName : #ClassName)

/**
 * Macro for static classes (non-instantiable, like functor classes).
 *
 * Usage:
 *   class MyStaticClass {
 *       SUDODEM_REGISTER_STATIC_CLASS(MyStaticClass, "My static class");
 *   public:
 *       static void pyRegisterClass(pybind11::module_& m);
 *   };
 */
#define SUDODEM_REGISTER_STATIC_CLASS(ClassName, DocString) \
    static void pyRegisterClass(pybind11::module_& m); \
    namespace { \
        struct ClassName##_Registrar { \
            ClassName##_Registrar() { \
                ClassRegistry::instance().registerClass<ClassName>(__FILE__); \
            } \
        } static ClassName##_registrar; \
    }

/**
 * Attribute registration helper.
 * This is used inside pyRegisterClass() to register attributes.
 *
 * Usage:
 *   static void pyRegisterClass(pybind11::module_& m) {
 *       auto cls = pybind11::class_<MyClass, Base>(m, "MyClass", "Description");
 *       cls.def(pybind11::init<>());
 *       SUDODEM_REGISTER_ATTR(cls, MyClass, myAttr, "My attribute");
 *   }
 */
#define SUDODEM_REGISTER_ATTR(cls, ClassName, attrName, doc) \
    cls.def_property(#attrName, \
        [](std::shared_ptr<ClassName> c) { return c->attrName; }, \
        [](std::shared_ptr<ClassName> c, const decltype(ClassName::attrName)& v) { \
            c->attrName = v; \
            c->callPostLoad(); \
        }, \
        doc)

/**
 * Read-only attribute registration helper.
 *
 * Usage:
 *   SUDODEM_REGISTER_ATTR_READONLY(cls, MyClass, myAttr, "My read-only attribute");
 */
#define SUDODEM_REGISTER_ATTR_READONLY(cls, ClassName, attrName, doc) \
    cls.def_property_readonly(#attrName, \
        [](std::shared_ptr<ClassName> c) { return c->attrName; }, \
        doc)

/**
 * Vector attribute registration helper (returns reference for in-place modification).
 *
 * Usage:
 *   SUDODEM_REGISTER_ATTR_VECTOR(cls, MyClass, myVector, "My vector attribute");
 */
#define SUDODEM_REGISTER_ATTR_VECTOR(cls, ClassName, attrName, doc) \
    cls.def_property(#attrName, \
        [](std::shared_ptr<ClassName> c) -> decltype(ClassName::attrName)& { \
            return c->attrName; \
        }, \
        [](std::shared_ptr<ClassName> c, const decltype(ClassName::attrName)& v) { \
            c->attrName = v; \
            c->callPostLoad(); \
        }, \
        doc)

/**
 * Method registration helper with documentation.
 *
 * Usage:
 *   SUDODEM_REGISTER_METHOD(cls, MyClass, myMethod, "My method description");
 */
#define SUDODEM_REGISTER_METHOD(cls, ClassName, methodName, doc) \
    cls.def(#methodName, &ClassName::methodName, doc)

/**
 * Static method registration helper.
 *
 * Usage:
 *   SUDODEM_REGISTER_STATIC_METHOD(cls, MyClass, myStaticMethod, "My static method description");
 */
#define SUDODEM_REGISTER_STATIC_METHOD(cls, ClassName, methodName, doc) \
    cls.def_static(#methodName, &ClassName::methodName, doc)
