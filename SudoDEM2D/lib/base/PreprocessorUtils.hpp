/*************************************************************************
*  Copyright (C) 2024 - SudoDEM Project
*
*  Standard C++ Preprocessor Utilities
*  Provides macro utilities for code generation
*
*  This program is free software; it is licensed under the terms of the
*  GNU General Public License v2 or later. See file LICENSE for details.
*************************************************************************/

#pragma once

// ============================================================================
// Stringification macros
// ============================================================================

#define SUDODEM_STRINGIZE(x) #x
#define SUDODEM_STRINGIZE_EXPAND(x) SUDODEM_STRINGIZE(x)

// ============================================================================
// Token concatenation macros
// Note: For nested concatenation, use SUDODEM_CAT_EXPAND to force expansion
// ============================================================================

#define SUDODEM_CAT_I(a, b) a##b
#define SUDODEM_CAT(a, b) SUDODEM_CAT_I(a, b)
#define SUDODEM_CAT_EXPAND(a, b) SUDODEM_CAT(a, b)

// ============================================================================
// Helper macros for variadic expansion
// ============================================================================

#define SUDODEM_EXPAND(x) x
#define SUDODEM_EMPTY()
#define SUDODEM_COMMA() ,

#ifdef _MSC_VER
    #define SUDODEM_NOINLINE __declspec(noinline)
#else
    #define SUDODEM_NOINLINE __attribute__((noinline))
#endif

#if defined(_WIN32)
  #define SUDODEM_PYREGISTER_CLASS_API
#elif defined(__GNUC__)
  #define SUDODEM_PYREGISTER_CLASS_API __attribute__((visibility("default")))
#else
  #define SUDODEM_PYREGISTER_CLASS_API
#endif

#if defined(_WIN32)
  #ifdef SUDODEM_STATIC_MEMBER_ORIGIN
    #define SUDODEM_STATIC_MEMBER_API __declspec(dllexport)
  #else
    #define SUDODEM_STATIC_MEMBER_API __declspec(dllimport)
  #endif
#else
  #define SUDODEM_STATIC_MEMBER_API
#endif