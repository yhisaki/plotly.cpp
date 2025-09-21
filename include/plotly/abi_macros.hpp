#pragma once

/**
 * @file abi_macros.hpp
 * @brief Application Binary Interface compatibility macros for plotly.cpp
 */

// Visibility macros for symbol exports
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef PLOTLY_BUILDING_LIB
#define PLOTLY_EXPORT __declspec(dllexport)
#else
#define PLOTLY_EXPORT __declspec(dllimport)
#endif
#else
#if __GNUC__ >= 4
#define PLOTLY_EXPORT __attribute__((visibility("default")))
#else
#define PLOTLY_EXPORT
#endif
#endif

// Deprecation markers
#if defined(__GNUC__) || defined(__clang__)
#define PLOTLY_DEPRECATED __attribute__((deprecated))
#define PLOTLY_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define PLOTLY_DEPRECATED __declspec(deprecated)
#define PLOTLY_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#else
#define PLOTLY_DEPRECATED
#define PLOTLY_DEPRECATED_MSG(msg)
#endif

// Inline macros
#if defined(_MSC_VER) && !defined(__clang__)
#define PLOTLY_INLINE inline
#define PLOTLY_FORCE_INLINE __forceinline
#else
#define PLOTLY_INLINE inline
#define PLOTLY_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Restrict keyword
#if defined(_MSC_VER) && !defined(__clang__)
#define PLOTLY_RESTRICT __restrict
#else
#define PLOTLY_RESTRICT __restrict__
#endif
