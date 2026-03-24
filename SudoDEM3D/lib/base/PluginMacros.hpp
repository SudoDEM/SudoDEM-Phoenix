/*************************************************************************
*  Copyright (C) 2024 - SudoDEM Project
*
*  Plugin Registration Macros
*  Uses simple variadic macros for flexibility
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*
*  NOTE: ClassFactory has been removed. Class registration is now handled
*  by ClassRegistry via static initialization and pyRegisterClass() methods.
*************************************************************************/

#pragma once

#include <sudodem/lib/base/PreprocessorUtils.hpp>
#include <sudodem/lib/factory/ClassRegistry.hpp>

// ============================================================================
// Plugin registration macros - register classes with ClassRegistry
// ============================================================================

// Single class plugin registration - now a no-op
// All classes are registered manually in ClassRegistryAll.cpp and initCoreClassRegistry()
#define SUDODEM_PLUGIN_IMPL(ClassName) /* Manual registration in ClassRegistryAll.cpp */

// Helper to select the right macro based on number of arguments
#define _SUDODEM_PLUGIN_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, NAME, ...) NAME

// Variadic plugin registration - selects the right macro based on number of arguments
// This allows SUDODEM_PLUGIN(A, B, C, ...) to work correctly
#define SUDODEM_PLUGIN(...) \
    _SUDODEM_PLUGIN_GET_MACRO(__VA_ARGS__, _SUDODEM_PLUGIN_20, _SUDODEM_PLUGIN_19, _SUDODEM_PLUGIN_18, _SUDODEM_PLUGIN_17, _SUDODEM_PLUGIN_16, _SUDODEM_PLUGIN_15, _SUDODEM_PLUGIN_14, _SUDODEM_PLUGIN_13, _SUDODEM_PLUGIN_12, _SUDODEM_PLUGIN_11, _SUDODEM_PLUGIN_10, _SUDODEM_PLUGIN_9, _SUDODEM_PLUGIN_8, _SUDODEM_PLUGIN_7, _SUDODEM_PLUGIN_6, _SUDODEM_PLUGIN_5, _SUDODEM_PLUGIN_4, _SUDODEM_PLUGIN_3, _SUDODEM_PLUGIN_2, _SUDODEM_PLUGIN_1)(__VA_ARGS__)

// Variadic plugin registration helpers - expand to multiple SUDODEM_PLUGIN_IMPL calls
// These use SUDODEM_PLUGIN_IMPL directly to avoid infinite recursion
#define _SUDODEM_PLUGIN_1(c1) SUDODEM_PLUGIN_IMPL(c1)
#define _SUDODEM_PLUGIN_2(c1, c2) SUDODEM_PLUGIN_IMPL(c1) SUDODEM_PLUGIN_IMPL(c2)
#define _SUDODEM_PLUGIN_3(c1, c2, c3) SUDODEM_PLUGIN_IMPL(c1) SUDODEM_PLUGIN_IMPL(c2) SUDODEM_PLUGIN_IMPL(c3)
#define _SUDODEM_PLUGIN_4(c1, c2, c3, c4) SUDODEM_PLUGIN_IMPL(c1) SUDODEM_PLUGIN_IMPL(c2) SUDODEM_PLUGIN_IMPL(c3) SUDODEM_PLUGIN_IMPL(c4)
#define _SUDODEM_PLUGIN_5(c1, c2, c3, c4, c5) _SUDODEM_PLUGIN_4(c1, c2, c3, c4) SUDODEM_PLUGIN_IMPL(c5)
#define _SUDODEM_PLUGIN_6(c1, c2, c3, c4, c5, c6) _SUDODEM_PLUGIN_5(c1, c2, c3, c4, c5) SUDODEM_PLUGIN_IMPL(c6)
#define _SUDODEM_PLUGIN_7(c1, c2, c3, c4, c5, c6, c7) _SUDODEM_PLUGIN_6(c1, c2, c3, c4, c5, c6) SUDODEM_PLUGIN_IMPL(c7)
#define _SUDODEM_PLUGIN_8(c1, c2, c3, c4, c5, c6, c7, c8) _SUDODEM_PLUGIN_7(c1, c2, c3, c4, c5, c6, c7) SUDODEM_PLUGIN_IMPL(c8)
#define _SUDODEM_PLUGIN_9(c1, c2, c3, c4, c5, c6, c7, c8, c9) _SUDODEM_PLUGIN_8(c1, c2, c3, c4, c5, c6, c7, c8) SUDODEM_PLUGIN_IMPL(c9)
#define _SUDODEM_PLUGIN_10(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) _SUDODEM_PLUGIN_9(c1, c2, c3, c4, c5, c6, c7, c8, c9) SUDODEM_PLUGIN_IMPL(c10)
#define _SUDODEM_PLUGIN_11(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) _SUDODEM_PLUGIN_10(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) SUDODEM_PLUGIN_IMPL(c11)
#define _SUDODEM_PLUGIN_12(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) _SUDODEM_PLUGIN_11(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) SUDODEM_PLUGIN_IMPL(c12)
#define _SUDODEM_PLUGIN_13(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) _SUDODEM_PLUGIN_12(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) SUDODEM_PLUGIN_IMPL(c13)
#define _SUDODEM_PLUGIN_14(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) _SUDODEM_PLUGIN_13(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) SUDODEM_PLUGIN_IMPL(c14)
#define _SUDODEM_PLUGIN_15(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) _SUDODEM_PLUGIN_14(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) SUDODEM_PLUGIN_IMPL(c15)
#define _SUDODEM_PLUGIN_16(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) _SUDODEM_PLUGIN_15(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) SUDODEM_PLUGIN_IMPL(c16)
#define _SUDODEM_PLUGIN_17(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17) _SUDODEM_PLUGIN_16(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16) SUDODEM_PLUGIN_IMPL(c17)
#define _SUDODEM_PLUGIN_18(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18) _SUDODEM_PLUGIN_17(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17) SUDODEM_PLUGIN_IMPL(c18)
#define _SUDODEM_PLUGIN_19(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19) _SUDODEM_PLUGIN_18(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18) SUDODEM_PLUGIN_IMPL(c19)
#define _SUDODEM_PLUGIN_20(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20) _SUDODEM_PLUGIN_19(c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19) SUDODEM_PLUGIN_IMPL(c20)

// Legacy macro names (for backward compatibility)
#define _SUDODEM_DECLARE_CREATOR_FUNCS(ClassName)
#define _SUDODEM_REGISTER_SINGLE_CLASS(ClassName) SUDODEM_PLUGIN_IMPL(ClassName)
#define _SUDODEM_PLUGIN_SELECT(...)
