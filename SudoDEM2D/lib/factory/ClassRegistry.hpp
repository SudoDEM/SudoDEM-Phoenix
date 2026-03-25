/*************************************************************************
*  Copyright (C) 2024 - SudoDEM Project
*
*  Modern C++17 Class Registry for pybind11 compatibility
*  Replaces the old macro-based, function-pointer system
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*************************************************************************/

#pragma once
#include <sudodem/lib/base/Singleton.hpp>
#include <sudodem/lib/base/Math.hpp>
#include <pybind11/pybind11.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include <set>
#include <algorithm>
#include <algorithm>

class Factorable;

class ClassRegistry : public Singleton<ClassRegistry> {
private:
    // Type-safe factory function
    using CreateFn = std::function<std::shared_ptr<Factorable>()>;

    // Pybind11 registration function
    using Pybind11RegisterFn = std::function<void(pybind11::module_&)>;

    // Class metadata
    struct ClassInfo {
        CreateFn create;
        Pybind11RegisterFn pybind11Register;
        std::string className;
        std::string baseClassName;
        std::string fileName;
        std::string readableClassName; // 人类可读的类名
        bool registered = false;
    };

    // Registry storage
    std::unordered_map<std::string, ClassInfo> registry;
    // 映射：人类可读的类名 -> mangled name
    std::unordered_map<std::string, std::string> readableToMangled;

    // Ordered list for registration sequence
    std::vector<std::string> registrationOrder;

    ClassRegistry() = default;
    ~ClassRegistry() = default;

public:
    /**
     * Register a class with the registry.
     * This is typically called via static initialization.
     *
     * @tparam T The class type to register
     * @param fileName The source file where the class is defined (for debugging)
     */
    template<typename T>
    void registerClass(const std::string& fileName = "") {
        // Use type_info to get class name without instantiation
        std::string className = typeid(T).name();
        // Demangle if needed - for now use type_info name directly
        // The pyRegisterClass method will handle the actual Python name

        // Check if already registered
        auto it = registry.find(className);
        if (it != registry.end()) {
            return; // Already registered
        }

        // Register the class
        registry[className] = {
            []() { return std::make_shared<T>(); },
            [](pybind11::module_& m) { std::make_shared<T>()->pyRegisterClass(m); },
            className,
            "", // Base class will be set by the class itself
            fileName
        };
        registry[className].registered = true;

        // Track registration order
        registrationOrder.push_back(className);
    }

    /**
     * Register a class with explicit base class information.
     *
     * @tparam T The class type to register
     * @tparam Base The base class type
     * @param fileName The source file where the class is defined
     */
    template<typename T, typename Base>
    void registerClassWithBase(const std::string& fileName = "") {
        // Use type_info to get class names without instantiation
        std::string className = typeid(T).name();
        std::string baseClassName = typeid(Base).name();

        // Check if already registered
        auto it = registry.find(className);
        if (it != registry.end()) {
            return; // Already registered
        }

        // Register the class with base class info
        registry[className] = {
            []() { return std::make_shared<T>(); },
            [](pybind11::module_& m) { std::make_shared<T>()->pyRegisterClass(m); },
            className,
            baseClassName,
            fileName
        };
        registry[className].registered = true;

        // Track registration order
        registrationOrder.push_back(className);
    }

    /**
     * Register all classes with pybind11 module.
     * This should be called once during module initialization.
     * Classes are sorted so base classes are registered before derived classes.
     *
     * @param m The pybind11 module to register classes with
     */
    void registerAll(pybind11::module_& m) {
        // Create a sorted order where base classes come before derived classes
        std::vector<std::string> sortedOrder;
        std::set<std::string> processed;
        
        std::function<void(const std::string&)> processClass = [&](const std::string& className) {
            if (processed.count(className)) return;
            
            // First process base class if it exists
            auto it = registry.find(className);
            if (it != registry.end() && !it->second.baseClassName.empty()) {
                processClass(it->second.baseClassName);
            }
            
            // Then add this class
            if (!processed.count(className)) {
                sortedOrder.push_back(className);
                processed.insert(className);
            }
        };
        
        // Process all classes in registration order
        for (const auto& className : registrationOrder) {
            processClass(className);
        }
        
        // Now register in sorted order
        for (const auto& className : sortedOrder) {
            auto it = registry.find(className);
            if (it != registry.end() && it->second.pybind11Register) {
                try {
                    it->second.pybind11Register(m);
                    // std::cerr << "DEBUG: Registered " << className << " with pybind11" << std::endl;
                } catch (const std::exception& e) {
                    // Silently skip classes that are already registered or have missing base types
                    // This is expected for classes with complex inheritance hierarchies
                    // std::cerr << "DEBUG: Skipping class " << className << " due to exception: " << e.what() << std::endl;
                }
            }
        }
    }

    /**
     * Create an instance of a class by name.
     *
     * @param className The type_info name of the class
     * @return Shared pointer to the created instance, or nullptr if not found
     */
    std::shared_ptr<Factorable> create(const std::string& className) {
        // 首先尝试直接查找（mangled name）
        auto it = registry.find(className);
        if (it != registry.end() && it->second.create) {
            return it->second.create();
        }
        
        // 如果没找到，尝试通过人类可读的类名查找
        auto mit = readableToMangled.find(className);
        if (mit != readableToMangled.end()) {
            auto it2 = registry.find(mit->second);
            if (it2 != registry.end() && it2->second.create) {
                return it2->second.create();
            }
        }
        
        return nullptr;
    }

    void setBaseClassName(const std::string& className, const std::string& baseClassName) {
        auto it = registry.find(className);
        if (it != registry.end()) {
            it->second.baseClassName = baseClassName;
        }
    }

    /**
     * Set the human-readable class name for a registered class.
     * This allows lookup by human-readable names (e.g., "Bo1_Superellipse_Aabb")
     * instead of mangled names (e.g., "N14Bo1_Superellipse_AabbE").
     *
     * @param mangledName The mangled type_info name
     * @param readableName The human-readable class name
     */
    void setReadableClassName(const std::string& mangledName, const std::string& readableName) {
        auto it = registry.find(mangledName);
        if (it != registry.end()) {
            it->second.readableClassName = readableName;
            readableToMangled[readableName] = mangledName;
        }
    }


    /**
     * Get the base class name for a registered class.
     *
     * @param className The class to query
     * @return The base class name, or empty string if not found
     */
    std::string getBaseClassName(const std::string& className) {
        auto it = registry.find(className);
        if (it != registry.end()) {
            return it->second.baseClassName;
        }
        return "";
    }

    /**
     * Check if a class is registered.
     *
     * @param className The class to check
     * @return true if registered, false otherwise
     */
    bool isRegistered(const std::string& className) {
        return registry.find(className) != registry.end();
    }

    /**
     * Get all registered class names.
     *
     * @return Vector of all registered class names
     */
    std::vector<std::string> getRegisteredClasses() {
        return registrationOrder;
    }

    /**
     * Get all classes that derive from a given base class.
     *
     * @param baseClassName The base class name
     * @return Vector of class names that derive from the base class
     */
    std::vector<std::string> getChildClasses(const std::string& baseClassName) {
        std::vector<std::string> children;
        for (const auto& className : registrationOrder) {
            if (registry[className].baseClassName == baseClassName) {
                children.push_back(className);
            }
        }
        return children;
    }

    /**
     * Check if a class derives from another class (recursive).
     *
     * @param className The class to check
     * @param baseClassName The base class name
     * @return true if className derives from baseClassName
     */
    bool isInheritingFrom(const std::string& className, const std::string& baseClassName) {
        std::string currentBase = getBaseClassName(className);
        if (currentBase.empty()) {
            return false;
        }
        if (currentBase == baseClassName) {
            return true;
        }
        return isInheritingFrom(currentBase, baseClassName);
    }

    /**
     * Get the demangled class name from type_info name.
     * This makes class names more readable in error messages.
     *
     * @param mangledName The type_info name
     * @return Demangled class name
     */
    static std::string demangleName(const std::string& mangledName) {
#ifdef __GNUG__
        int status = 0;
        char* demangled = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
        if (status == 0 && demangled) {
            std::string result(demangled);
            free(demangled);
            return result;
        }

#elif defined(_MSC_VER)

    std::string outname = mangledName;

    const char* prefixes[] = {
        "class ",
        "struct ",
        "enum ",
        "union "
    };

    for (const char* p : prefixes) {
        std::string pref(p);
        if (outname.rfind(pref, 0) == 0) {
            outname.erase(0, pref.size());
            break;
        }
    }

    return outname;
#endif

    return mangledName;
    }

    /**
     * Clear all registrations.
     * Useful for testing and debugging.
     */
    void clear() {
        registry.clear();
        registrationOrder.clear();
    }

    /**
     * Get the number of registered classes.
     *
     * @return Number of registered classes
     */
    size_t size() const {
        return registry.size();
    }

    FRIEND_SINGLETON(ClassRegistry);
};
