/**
 * Definitions of types used frequently in the learned index.
 */

#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <limits>

#ifndef DIM
#define DIM 4
#endif

// Test all known ways to detect endianness. The endianness
// is important when we do compression.
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#define BIG_ENDIAN_ORDER
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define LITTLE_ENDIAN_ORDER
#else
#error "Unknown compiler! No known endianness macros found"
#endif

#ifdef __linux__
typedef __float128 ldouble;
#elif __APPLE__
typedef double ldouble;
#endif

// This type must match the type of the preprocessed data.
typedef int32_t Scalar;
typedef std::vector<Scalar> ScalarList;

/*
 * A data point with DIM dimensions.
 * Simply exposes an interface to get its dimensions
 */
//typedef std::array<Scalar, DIM> Point;
//typedef std::vector<Scalar> Point;
template <size_t D>
using Point = std::array<Scalar, D>;

/**
 * Query bounding box to issue to the index.
 */
template <size_t D>
struct Query {
    std::array<ScalarList, D> filters;
};

// These limits are used in the query to signify unboundedness in that direction.
const Scalar SCALAR_MAX = std::numeric_limits<Scalar>::max();
const Scalar SCALAR_MIN = std::numeric_limits<Scalar>::lowest();
const Scalar SCALAR_PINF = 1UL << 30;
const Scalar SCALAR_NINF = -(1UL << 30);

/**
 * The true index of a point in the vector of data points.
 */
typedef size_t PhysicalIndex;

struct PhysicalIndexRange {
    PhysicalIndex start;
    PhysicalIndex end;  // inclusive
    bool exact;

    PhysicalIndexRange() : start(0), end(0), exact(false) {}
    PhysicalIndexRange(PhysicalIndex s, PhysicalIndex e, bool ex)
        : start(s), end(e), exact(ex) {}
};


