/*
 * Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/vec.h
 * @author scott snyder <snyder@bnl.gov>
 * @author Christos Anastopoulos (additional helper methods)
 * @date Mar, 2020
 * @brief Vectorization helpers.
 *
 * gcc and clang provide built-in types for writing vectorized code,
 * using the vector_size attribute.  This usually results in code
 * that is much easier to read and more portable than one would get
 * using intrinsics directly.  However, it is still non-standard,
 * and there are some operations which are kind of awkward.
 *
 * This file provides some helpers for writing vectorized code
 * in C++.
 *
 * A vectorized type may be named as @c CxxUtils::vec<T, N>.  Here @c T is the
 * element type, which should be an elementary integer or floating-point type.
 * @c N is the number of elements in the vector; it should be a power of 2.
 * This will either be a built-in vector type if the @c vector_size
 * attribute is supported or a fallback C++ class intended to be
 * (mostly) functionally equivalent.
 *
 *
 * The GCC, clang and fallback vector types support:
 * ++, --, +,-,*,/,%, =, &,|,^,~, >>,<<, !, &&, ||,
 * ==, !=, >, <, >=, <=, =, sizeof and Initialization from brace-enclosed lists
 *
 * Furthemore the GCC and clang>=10 vector types support the ternary operator.
 *
 * We also support some additional operations.
 *
 * Deducing useful types:
 *
 *  - @c CxxUtils::vec_type_t<VEC> is the element type of @c VEC.
 *  - @c CxxUtils::mask_type_t<VEC> is the vector type return by relational
 *                                  operations.
 *
 * Deducing the num of elements in a vectorized type:
 *
 *  - @c CxxUtils::vec_size<VEC>() is the number of elements in @c VEC.
 *  - @c CxxUtils::vec_size(const VEC&) is the number of elements in @c VEC.
 *
 * Additional Helpers for common SIMD operations:
 *
 *  - @c CxxUtils::vbroadcast (VEC& v, T x) initializes each element of
 *                                          @c v with @c x.
 *  - @c CxxUtils::vload (VEC& dst, const vec_type_t<VEC>* src)
 *                                          loads elements from @c src
 *                                          to @c dst
 *  - @c CxxUtils::vstore (vec_type_t<VEC>* dst, const VEC& src)
 *                                          stores elements from @c src
 *                                          to @c dst
 *  - @c CxxUtils::vselect (VEC& dst, const VEC& a, const VEC& b, const
 *                          mask_type_t<VEC>& mask) copies elements
 *                          from @c a or @c b, depending
 *                          on the value of @c  mask to @c dst.
 *                          dst[i] = mask[i] ? a[i] : b[i]
 *  - @c CxxUtils::vmin     (VEC& dst, const VEC& a, const VEC& b)
 *                         copies to @c dst[i]  the min(a[i],b[i])
 *  - @c CxxUtils::vmax    (VEC& dst, const VEC& a, const VEC& b)
 *                         copies to @c dst[i]  the max(a[i],b[i])
 *  - @c CxxUtils::vpermute<mask> (VEC& dst, const VEC& src)
 *                          Fills dst with permutation of src
 *                          according to mask.
 *                          Mask is a list of integers that specifies the elements
 *                          that should be extracted and returned in src.
 *                          dst[i] = src[mask[i]] where mask[i] is the ith integer
 *                          in the mask.
 *  - @c CxxUtils::vblend<mask> (VEC& dst, const VEC& src1,const VEC& src2)
 *                          Fills dst with permutation of src1 and src2
 *                          according to mask.
 *                          Mask is a list of integers that specifies the elements
 *                          that should be extracted and returned in src1 and src2.
 *                          An index i in the interval [0,N) indicates that element number i 
 *                          from the first input vector should be placed in the
 *                          corresponding position in the result vector.  
 *                          An index in the interval [N,2N)
 *                          indicates that the element number i-N
 *                          from the second input vector should be placed 
 *                          in the corresponding position in the result vector.
 *
 * In terms of expected performance it might be  advantageous to
 * use vector types that fit the size of the ISA.
 * e.g 128 bit wide for SSE, 256 wide for AVX.
 *
 * Specifying a combination that is not valid for the current architecture
 * causes the compiler to synthesize the instructions using a narrower mode.
 *
 * Consider using Function Multiversioning (CxxUtils/features.h)
 * if you really need to target efficiently multiple ISAs.
 */

#ifndef CXXUTILS_VEC_H
#define CXXUTILS_VEC_H

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <type_traits>

// Do we additionally support the ternary operator for vectorizes types.
// GCC and llvm clang >=10
#if !(defined(__clang__) && ((__clang_major__ < 10) || defined(__APPLE__)))
#define HAVE_VECTOR_TERNARY_OPERATOR 1
#else
#define HAVE_VECTOR_TERNARY_OPERATOR 0
#endif

namespace CxxUtils {

/// Define a nice alias for a built-in vectorized type.
template<typename T, size_t N>
using vec __attribute__((vector_size(N * sizeof(T)))) = T;

/**
 * @brief Deduce the element type from a vectorized type.
 */
template<class VEC>
struct vec_type
{
  static auto elt(const VEC& v) -> decltype(v[0]);
  typedef typename std::invoke_result<decltype(elt), const VEC&>::type type1;
  typedef std::remove_cv_t<std::remove_reference_t<type1>> type;
};

/// Deduce the element type from a vectorized type.
template<class VEC>
using vec_type_t = typename vec_type<VEC>::type;

/**
 * brief Deduce the type of the mask returned by relational operations,
 * for a vectorized type.
 */
template<class VEC>
struct mask_type
{
  static auto maskt(const VEC& v1, const VEC& v2) -> decltype(v1 < v2);
  typedef
    typename std::invoke_result<decltype(maskt), const VEC&, const VEC&>::type
      type1;
  typedef std::remove_cv_t<std::remove_reference_t<type1>> type;
};
/// Deduce the mask type for a vectorized type.
template<class VEC>
using mask_type_t = typename mask_type<VEC>::type;

/**
 * @brief Return the number of elements in a vectorized type.
 */
template<class VEC>
inline constexpr size_t
vec_size()
{
  typedef vec_type_t<VEC> ELT;
  return sizeof(VEC) / sizeof(ELT);
}

/**
 * @brief Return the number of elements in a vectorized type.
 */
template<class VEC>
inline constexpr size_t
vec_size(const VEC&)
{
  typedef vec_type_t<VEC> ELT;
  return sizeof(VEC) / sizeof(ELT);
}

/**
 * @brief Copy a scalar to each element of a vectorized type.
 */
template<typename VEC, typename T>
inline void
vbroadcast(VEC& v, T x)
{
  // using  - to avoid sign conversions.
  // using + adds  extra instructions due to float arithmetic.
  v = x - VEC{ 0 };
}

/*
 * @brief load elements from  memory address src (C-array)
 * to a vectorized type dst.
 * Used memcpy to avoid alignment issues
 */
template<typename VEC>
inline void
vload(VEC& dst, vec_type_t<VEC> const* src)
{
  std::memcpy(&dst, src, sizeof(VEC));
}

/*
 * @brief store elements from a vectorized type src to
 * to a memory address dst (C-array).
 * Uses memcpy to avoid alignment issues
 */
template<typename VEC>
inline void
vstore(vec_type_t<VEC>* dst, const VEC& src)
{
  std::memcpy(dst, &src, sizeof(VEC));
}

/*
 * @brief select elements based on a mask
 * Fill dst according to
 * dst[i] = mask[i] ? a[i] : b[i]
 */
template<typename VEC>
inline void
vselect(VEC& dst, const VEC& a, const VEC& b, const mask_type_t<VEC>& mask)
{
#if !HAVE_VECTOR_TERNARY_OPERATOR
  constexpr size_t N = vec_size<VEC>();
  for (size_t i = 0; i < N; i++) {
    dst[i] = mask[i] ? a[i] : b[i];
  }
#else
  dst = mask ? a : b;
#endif
}

/*
 * @brief vectorized min.
 * copies to @c dst[i]  the min(a[i],b[i])
 */
template<typename VEC>
inline void
vmin(VEC& dst, const VEC& a, const VEC& b)
{
#if !HAVE_VECTOR_TERNARY_OPERATOR
  constexpr size_t N = vec_size<VEC>();
  for (size_t i = 0; i < N; i++) {
    dst[i] = a[i] < b[i] ? a[i] : b[i];
  }
#else
  dst = a < b ? a : b;
#endif
}

/*
 * @brief vectorized max.
 * copies to @c dst[i]  the max(a[i],b[i])
 */
template<typename VEC>
inline void
vmax(VEC& dst, const VEC& a, const VEC& b)
{
#if !HAVE_VECTOR_TERNARY_OPERATOR
  constexpr size_t N = vec_size<VEC>();
  for (size_t i = 0; i < N; i++) {
    dst[i] = a[i] > b[i] ? a[i] : b[i];
  }
#else
  dst = a > b ? a : b;
#endif
}

/*
 * Helper for static asserts for argument packs
 */
namespace bool_pack_helper {
template<bool...>
struct bool_pack;
template<bool... bs>
using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
}
/**
 * @brief vpermute function.
 * move any element of a vector src
 * into any or multiple position inside dst.
 */
template<size_t... Indices, typename VEC>
inline void
vpermute(VEC& dst, const VEC& src)
{
  constexpr size_t N = vec_size<VEC>();
  static_assert((sizeof...(Indices) == N),
                "Number of indices different than vector size");
  static_assert(
    bool_pack_helper::all_true<(Indices >= 0 && Indices < N)...>::value,
    "permute indices outside allowed range");
#if defined(__clang__)
  dst = __builtin_shufflevector(src, src, Indices...);
#else // gcc
  dst = __builtin_shuffle(src, mask_type_t<VEC>{ Indices... });
#endif
}

/**
 * @brief vblend function.
 * permutes and blends elements from two vectors
 * Similar to the permute functions, but with two input vectors.
 */
template<size_t... Indices, typename VEC>
inline void
vblend(VEC& dst, const VEC& src1, const VEC& src2)
{
  constexpr size_t N = vec_size<VEC>();
  static_assert((sizeof...(Indices) == N),
                "Number of indices different than vector size");
  static_assert(
    bool_pack_helper::all_true<(Indices >= 0 && Indices < 2 * N)...>::value,
    "blend indices outside allowed range");
#if defined(__clang__)
  dst = __builtin_shufflevector(src1, src2, Indices...);
#else // gcc
  dst = __builtin_shuffle(src1, src2, mask_type_t<VEC>{ Indices... });
#endif
}

} // namespace CxxUtils

#endif // not CXXUTILS_VEC_H

