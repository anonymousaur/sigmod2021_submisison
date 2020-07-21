/**
 * Definitions of types used frequently in the learned index.
 */

#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <set>

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
typedef double Scalar;
struct ScalarRange {
    Scalar first;
    Scalar second;
    ScalarRange() : first(0), second(0) {}
    ScalarRange(Scalar a, Scalar b) : first(a), second(b) {}
    bool operator==(const ScalarRange& other) const {
        return (first == other.first) && (second == other.second);
    }
    friend std::ostream& operator<<(std::ostream& os, const ScalarRange& sr) { 
        os << "[" << sr.first << ", " << sr.second << "]";
        return os;
    }
};

struct QueryFilter {
    // True if this filter is present
    bool present;
    bool is_range;
    // This can be interpreted as:
    // ... WHERE ranges[0].first <= attr <= ranges[0].second
    //     OR ranges[1].first <= attr <= ranges[1].second
    //     OR ...
    // Ranges may not always be sorted.
    // Queries may not be specified in this way, but after rewriting, they often are.
    std::vector<ScalarRange> ranges;
    // values in an IN clause.
    // Values are always sorted.
    std::vector<Scalar> values;   
};

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
    std::array<QueryFilter, D> filters;
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
    PhysicalIndex end;  // exclusive

    PhysicalIndexRange() : start(0), end(0) {}
    PhysicalIndexRange(PhysicalIndex s, PhysicalIndex e)
        : start(s), end(e) {}
    bool operator==(const PhysicalIndexRange& other) const {
        return (start == other.start) && (end == other.end);
    }
    friend std::ostream& operator<<(std::ostream& os, const PhysicalIndexRange& pr) { 
        os << "[" << pr.start << ", " << pr.end << "]";
        return os;
    }
};

struct RangeSet {
    // Compare ranges by their end value.
    struct rangecomp {
          bool operator() (const ScalarRange& lhs, const ScalarRange& rhs) const {
              return lhs.second < rhs.second;
          } 
    };
    std::set<ScalarRange, rangecomp> ranges;
    RangeSet() : ranges() {}
    RangeSet(std::vector<ScalarRange>::const_iterator begin,
           std::vector<ScalarRange>::const_iterator end) : ranges(begin, end) {}
    // Adds this range to the set. The endpoints of this range are both inclusive.
    // We assume there are no overlapping ranges.
    void AddRange(const ScalarRange& sr) { ranges.insert(sr); }
    void AddRanges(std::vector<ScalarRange>::const_iterator begin,
           std::vector<ScalarRange>::const_iterator end) {
        ranges.insert(begin, end);
    } 
    // Tests whether a point is in one of the ranges in this set.
    bool Test(Scalar s) const {
        const auto loc = ranges.lower_bound({s, s});
        // If the start of the pointed to range is less than the test value, the point must be in a
        // valid range.
        return (loc != ranges.end() && loc->first <= s);
    }
};
