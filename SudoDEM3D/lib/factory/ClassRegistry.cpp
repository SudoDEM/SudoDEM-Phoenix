/*************************************************************************
*  Copyright (C) 2024 - SudoDEM Project
*
*  ClassRegistry - Simplified factory only
*  Keeps registration for factory (create instances) but NOT for pybind11
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*************************************************************************/

#include <sudodem/lib/factory/ClassRegistry.hpp>

// Simple registerAll - just calls pyRegisterClass on each registered class
// Each class is responsible for its own pybind11 bindings including base class dependencies
void ClassRegistry::registerAll(pybind11::module_& m) {
    std::cerr << "DEBUG: registerAll called with " << registry.size() << " classes" << std::endl;
    
    for (const auto& className : registrationOrder) {
        auto it = registry.find(className);
        if (it != registry.end() && it->second.pybind11Register) {
            try {
                it->second.pybind11Register(m);
                std::cerr << "DEBUG: Registered: " << className << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "DEBUG: Skip " << className << ": " << e.what() << std::endl;
            }
        }
    }
}

std::shared_ptr<Factorable> ClassRegistry::create(const std::string& className) {
    // First try direct lookup (mangled name)
    auto it = registry.find(className);
    if (it != registry.end() && it->second.create) {
        return it->second.create();
    }
    
    // If not found, try lookup by human-readable name
    auto mit = readableToMangled.find(className);
    if (mit != readableToMangled.end()) {
        auto it2 = registry.find(mit->second);
        if (it2 != registry.end() && it2->second.create) {
            return it2->second.create();
        }
    }
    
    // Try demangled name - some classes may register with demangled names
    std::string demangled = demangleName(className);
    if (demangled != className) {
        mit = readableToMangled.find(demangled);
        if (mit != readableToMangled.end()) {
            auto it2 = registry.find(mit->second);
            if (it2 != registry.end() && it2->second.create) {
                return it2->second.create();
            }
        }
    }
    
    return nullptr;
}

void ClassRegistry::setBaseClassName(const std::string& className, const std::string& baseClassName) {
    auto it = registry.find(className);
    if (it != registry.end()) {
        it->second.baseClassName = baseClassName;
    }
}

void ClassRegistry::setReadableClassName(const std::string& mangledName, const std::string& readableName) {
    auto it = registry.find(mangledName);
    if (it != registry.end()) {
        it->second.readableClassName = readableName;
        readableToMangled[readableName] = mangledName;
    }
}

std::string ClassRegistry::getBaseClassName(const std::string& className) {
    auto it = registry.find(className);
    if (it != registry.end()) {
        return it->second.baseClassName;
    }
    return "";
}

bool ClassRegistry::isRegistered(const std::string& className) {
    return registry.find(className) != registry.end();
}

std::vector<std::string> ClassRegistry::getRegisteredClasses() {
    return registrationOrder;
}

std::vector<std::string> ClassRegistry::getChildClasses(const std::string& baseClassName) {
    std::vector<std::string> children;
    for (const auto& className : registrationOrder) {
        if (registry[className].baseClassName == baseClassName) {
            children.push_back(className);
        }
    }
    return children;
}

bool ClassRegistry::isInheritingFrom(const std::string& className, const std::string& baseClassName) {
    std::string currentBase = getBaseClassName(className);
    if (currentBase.empty()) {
        return false;
    }
    if (currentBase == baseClassName) {
        return true;
    }
    return isInheritingFrom(currentBase, baseClassName);
}

std::string ClassRegistry::demangleName(const std::string& mangledName) {
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

void ClassRegistry::clear() {
    registry.clear();
    registrationOrder.clear();
    readableToMangled.clear();
}

size_t ClassRegistry::size() const {
    return registry.size();
}
