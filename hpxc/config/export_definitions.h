//  Copyright (c) 2007-2022 Hartmut Kaiser
//  Copyright (c)      2011 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define HPXC_SYMBOL_EXPORT __declspec(dllexport)
#define HPXC_SYMBOL_IMPORT __declspec(dllimport)
#define HPXC_SYMBOL_INTERNAL /* empty */
#define HPXC_APISYMBOL_EXPORT __declspec(dllexport)
#define HPXC_APISYMBOL_IMPORT __declspec(dllimport)
#else
#define HPXC_SYMBOL_EXPORT __attribute__((visibility("default")))
#define HPXC_SYMBOL_IMPORT __attribute__((visibility("default")))
#define HPXC_SYMBOL_INTERNAL __attribute__((visibility("hidden")))
#define HPXC_APISYMBOL_EXPORT __attribute__((visibility("default")))
#define HPXC_APISYMBOL_IMPORT __attribute__((visibility("default")))
#endif

// make sure we have reasonable defaults
#if !defined(HPXC_SYMBOL_EXPORT)
#define HPXC_SYMBOL_EXPORT /* empty */
#endif
#if !defined(HPXC_SYMBOL_IMPORT)
#define HPXC_SYMBOL_IMPORT /* empty */
#endif
#if !defined(HPXC_SYMBOL_INTERNAL)
#define HPXC_SYMBOL_INTERNAL /* empty */
#endif
#if !defined(HPXC_APISYMBOL_EXPORT)
#define HPXC_APISYMBOL_EXPORT /* empty */
#endif
#if !defined(HPXC_APISYMBOL_IMPORT)
#define HPXC_APISYMBOL_IMPORT /* empty */
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros used by the core library
#if defined(HPXC_EXPORTS)
#define HPXC_EXPORT HPXC_SYMBOL_EXPORT
#define HPXC_EXCEPTION_EXPORT HPXC_SYMBOL_EXPORT
#define HPXC_API_EXPORT
#else
#define HPXC_EXPORT HPXC_SYMBOL_IMPORT
#define HPXC_EXCEPTION_EXPORT HPXC_SYMBOL_IMPORT
#define HPXC_API_EXPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros used by applications
#if defined(HPXC_APPLICATION_EXPORTS)
#define HPXC_APPLICATION_EXPORT HPXC_SYMBOL_EXPORT
#else
#define HPXC_APPLICATION_EXPORT HPXC_SYMBOL_IMPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros for exceptions
#if defined(HPXC_EXPORTS)
#define HPXC_ALWAYS_EXPORT HPXC_SYMBOL_EXPORT
#else
#define HPXC_ALWAYS_EXPORT HPXC_SYMBOL_IMPORT
#endif
