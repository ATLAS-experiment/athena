//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_TEMPORARYHELPERS_H
#define CALORECGPU_TEMPORARYHELPERS_H

#define CALORECGPU_TEMP_CONCAT_HELPER_INNER(A, ...) A ## __VA_ARGS__
#define CALORECGPU_TEMP_CONCAT_HELPER(A, B) CALORECGPU_TEMP_CONCAT_HELPER_INNER(A, B)


//We can define a type that already contains all of the necessary temporary arrays
//(of appropriate size and type, of course, and with the same names as the functions),
//or we can simply rely on the individual arrays inside the moments.
//The former is more efficient whenever practical,
//but obviously not supported if the moments have been calculated.
//We may have better future solutions for this...

#ifdef CALORECGPU_TEMP_STRUCT_TO_USE

namespace CaloRecGPU
{
  template <class T, class PtrLike>
  __host__ __device__ T * get_pointer_to_temp_struct(PtrLike && p)
  {
    return std::launder(reinterpret_cast<T *>(&(p->moments)));
  }
}

#define CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)  _Pragma("nv_diag_suppress 177")                                                             \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_pointer_to_temp_struct<const CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME + idx;                                   \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_pointer_to_temp_struct<CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME + idx;                                         \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

#define CALORECGPU_TEMPARR_BASE_2D(TEMPNAME, TYPE)  _Pragma("nv_diag_suppress 177")                                                             \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return &(CaloRecGPU::get_pointer_to_temp_struct<const CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME[jdx][idx]);                            \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return &(CaloRecGPU::get_pointer_to_temp_struct<CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME[jdx][idx]);                                  \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int jdx, const unsigned int idx)     \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int jdx, const unsigned int idx)                 \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon
    
///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the 1D array accessible through @p BASEVAR.
///@remark The most common case of a 32-bit temporary variable per cluster.
#define CALORECGPU_TEMPARR_1(TEMPNAME, BASEVAR, TYPE) CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be split across two 1D arrays accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark This would be a 64-bit temporary variable per cluster.
#define CALORECGPU_TEMPARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the 2D array (i. e. per sampling) accessible through @p BASEVAR.
///@remark This would be a 32-bit per sampling temporary variable per cluster.
#define CALORECGPU_TEMP2DARR_1(TEMPNAME, BASEVAR, TYPE)  CALORECGPU_TEMPARR_BASE_2D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will will be split across two 2D arrays (i. e. per sampling) accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark The most common case of a 32-bit temporary variable per cluster.
#define CALORECGPU_TEMP2DARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  CALORECGPU_TEMPARR_BASE_2D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of the 2D array
///(i. e. per sampling) accessible through @p BASEVAR.
///@remark This would be enough for a 32-bit temporary variable per possible neighbour pair (NExactPairs)
///         if NMaxClusters == NCaloCells, otherwise it's not enough.
#define CALORECGPU_TEMPBIGARR_1(TEMPNAME, BASEVAR, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of two 2D arrays
///(i. e. per sampling) accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark This is always enough for a 32-bit temporary variable per possible neighbour pair (NExactPairs),
///        or even a 64-bit per neighbour pair if NMaxClusters == NCaloCells.
#define CALORECGPU_TEMPBIGARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of three 2D arrays
///(i. e. per sampling) accessible through @p BASEVAR1, @p BASEVAR2 and @p BASEVAR3.
///@remark This is a good match for fitting two NExactPairs arrays (in different directions)
///        while still being able to fit two in the six per-sampling variables in the moments.
#define CALORECGPU_TEMPBIGARR_3(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that can have up to three times NMaxClusters, split across three variables.
///@remark This would be a 32-bit temporary variable per cell regardless of NMaxClusters.
#define CALORECGPU_TEMPCELLARR_1(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///The definitions for a temporary variable of type @p TYPE with name @p TEMPNAME
///that can have up to three times NMaxClusters, split across six variables.
///@remark This would be a 64-bit temporary variable per cell regardless of NMaxClusters.
#define CALORECGPU_TEMPCELLARR_2(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, BASEVAR4, BASEVAR5, BASEVAR6, TYPE)  CALORECGPU_TEMPARR_BASE_1D(TEMPNAME, TYPE)

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the @p INDEX entry of an 1D array.
///@remark The @p INDEX represents the index of the array
///        of @p TYPE, not the offset within the original array.
///@warning The address will be appropriately aligned, so, for bigger types,
///         the maximum number of variables that can be stored in a single array
///         may be smaller, depending on the necessary alignment.
#define CALORECGPU_TEMPVAR(TEMPNAME, BASEVAR, INDEX, TYPE)  _Pragma("nv_diag_suppress 177")                                                    \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr)                \
  {                                                                                                                                            \
    return &CaloRecGPU::get_pointer_to_temp_struct<const CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME;                                        \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr)                            \
  {                                                                                                                                            \
    return &CaloRecGPU::get_pointer_to_temp_struct<CALORECGPU_TEMP_STRUCT_TO_USE>(arr)->TEMPNAME;                                              \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr)                                                     \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr);                                                                                \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr)                                                                 \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr);                                                                                \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

///Wraps a function in another name, for better semantics while reusing storage...
#define CALORECGPU_TEMPWRAPPER(TEMPNAME, WRAPPED)  _Pragma("nv_diag_suppress 177")                                                             \
  template <class PtrLike, class ... Args> __host__ __device__ decltype(auto) TEMPNAME (PtrLike && p, Args && ... args)                        \
  {                                                                                                                                            \
    return std::forward<PtrLike>(p)-> WRAPPED (std::forward<Args>(args)...);                                                                   \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon
  

#else


#include "CaloRecGPU/BaseDefinitions.h"

#include <cassert>
#include <new>
#include <utility>
#include <type_traits>

namespace CaloRecGPU
{
  //Assumptions: each individual pointer-like object
  //refers to a contiguous memory region
  //whose first address is obtainable by &p[0].
  template <class T, unsigned int ... us, class ... PtrLikes>
  __host__ __device__ T * get_laundered_pointer(unsigned int idx, PtrLikes && ... p);

  inline constexpr unsigned int get_extra_alignment(const unsigned int base_align, const unsigned int required)
  {
    const unsigned int delta = base_align % required;

    return required * (delta != 0) - delta;
  }

  template <class T>
  constexpr bool __host__ __device__ check_sufficient_size(unsigned int offset, unsigned int index)
  {
    return (NMaxClusters >= get_extra_alignment(offset, alignof(T)) + index * sizeof(T));
  }

  template <class T, unsigned int offset, class PtrLike>
  __host__ __device__ T * get_laundered_pointer(unsigned int idx, PtrLike && ptr)
  {
    using PtrType = std::decay_t<decltype(ptr[0])>;
    using BasePtrType = std::conditional_t<std::is_const_v<PtrType>, const char *, char *>;

    constexpr unsigned int base_offset = offset % alignof(T);

    constexpr unsigned int extra_alignment = get_extra_alignment(base_offset, alignof(T));

    BasePtrType base_ptr = reinterpret_cast<BasePtrType>(&ptr[0]);

    return std::launder(reinterpret_cast<T *>(base_ptr + extra_alignment + idx * sizeof(T)));
  }

  template <class T, unsigned int offset, unsigned int ... us, class PtrLike, class ... PtrLikes>
  __host__ __device__ T * get_laundered_pointer(unsigned int idx, PtrLike && ptr, PtrLikes && ... ps)
  {
    using PtrType = std::decay_t<decltype(ptr[0])>;

    constexpr unsigned int max_size = NMaxClusters * sizeof(PtrType);

    constexpr unsigned int base_offset = offset % alignof(T);
    constexpr unsigned int extra_alignment = get_extra_alignment(base_offset, alignof(T));

    constexpr unsigned int real_size = (max_size - extra_alignment) / sizeof(T);

    return (real_size > idx ? get_laundered_pointer<T, offset>(idx, std::forward<PtrLike>(ptr)) : get_laundered_pointer<T, us...>(idx - real_size, std::forward<PtrLikes>(ps)...));
  }

  //In this case, the pointer-likes are to 2D arrays
  //and we stack them so that ptr[j][N] and ptr[j][N+1]
  //may be across different arrays.
  //For ease of implementation, we only support suitably aligned
  //classes (up to double).
  template <class T, class ... PtrLikes>
  __host__ __device__ T * get_laundered_pointer_striped(unsigned int jdx, unsigned int idx, PtrLikes && ... ps)
  {
    static_assert(alignof(T) <= alignof(double), "We don't support aligning in this case...");

    return get_laundered_pointer<T, (alignof(decltype(ps[0][0])) * 0)...>(idx, ps[jdx]...);
    //We pass a param pack of 0s (with the right size)
    //as we know we won't need alignment.
  }

  //In this case, we use the 2D arrays
  //to store a single, contiguous array
  //such that ptr[j][N] and ptr[j + 1][0]
  //may hold contiguous objects.
  template <class T, class ... PtrLikes>
  __host__ __device__ T * get_laundered_pointer_stacked(unsigned int idx, PtrLikes && ... ps);

  template <class T, class PtrLike>
  __host__ __device__ T * get_laundered_pointer_stacked(unsigned int idx, PtrLike && ptr)
  {
    static_assert(alignof(T) <= alignof(double), "We don't support aligning in this case...");

    using PtrType = std::decay_t<decltype(ptr[0][0])>;
    using BasePtrType = std::conditional_t<std::is_const_v<PtrType>, const char *, char *>;

    constexpr unsigned int num_per_array = (NMaxClusters * sizeof(PtrType)) / sizeof(T);

    const unsigned int first_idx = idx / num_per_array;

    const unsigned int second_idx = idx % num_per_array;

    return get_laundered_pointer<T, 0>(second_idx, ptr[first_idx]);
  }

  template <class T, class PtrLike, class ... PtrLikes>
  __host__ __device__ T * get_laundered_pointer_stacked(unsigned int idx, PtrLike && ptr, PtrLikes && ... ps)
  {
    static_assert(alignof(T) <= alignof(double));

    using PtrType = std::decay_t<decltype(ptr[0][0])>;

    constexpr unsigned int num_per_array = (NMaxClusters * sizeof(PtrType)) / sizeof(T);
    constexpr unsigned int total_num = num_per_array * NumSamplings;

    return (total_num > idx ? get_laundered_pointer_stacked<T>(idx, std::forward<PtrLike>(ptr)) : get_laundered_pointer_stacked<T>(idx - total_num, std::forward<PtrLikes>(ps)...));
  }
}

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the 1D array accessible through @p BASEVAR.
///@remark The most common case of a 32-bit temporary variable per cluster.
#define CALORECGPU_TEMPARR_1(TEMPNAME, BASEVAR, TYPE)  _Pragma("nv_diag_suppress 177")                                                         \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<const TYPE, offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR)>(idx, arr->moments. BASEVAR); \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<TYPE, offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR)>(idx, arr->moments. BASEVAR); \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be split across two 1D arrays accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark This would be a 64-bit temporary variable per cluster.
#define CALORECGPU_TEMPARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  _Pragma("nv_diag_suppress 177")                                              \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<const TYPE,                                                                                       \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                                                                              \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<TYPE,                                                                                             \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                                                                              \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the 2D array (i. e. per sampling) accessible through @p BASEVAR.
///@remark This would be a 32-bit per sampling temporary variable per cluster.
#define CALORECGPU_TEMP2DARR_1(TEMPNAME, BASEVAR, TYPE)  _Pragma("nv_diag_suppress 177")                                                       \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_striped<const TYPE>(jdx, idx, arr->moments. BASEVAR);                                             \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_striped<TYPE>(jdx, idx, arr->moments. BASEVAR);                                                   \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int jdx, const unsigned int idx)     \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int jdx, const unsigned int idx)                 \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will will be split across two 2D arrays (i. e. per sampling) accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark The most common case of a 32-bit temporary variable per cluster.
#define CALORECGPU_TEMP2DARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  _Pragma("nv_diag_suppress 177")                                            \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_striped<const TYPE>(jdx, idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                    \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int jdx, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_striped<TYPE>(jdx, idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                          \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int jdx, const unsigned int idx)     \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int jdx, const unsigned int idx)                 \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, jdx, idx);                                                                      \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of the 2D array
///(i. e. per sampling) accessible through @p BASEVAR.
///@remark This would be enough for a 32-bit temporary variable per possible neighbour pair (NExactPairs)
///         if NMaxClusters == NCaloCells, otherwise it's not enough.
#define CALORECGPU_TEMPBIGARR_1(TEMPNAME, BASEVAR, TYPE)  _Pragma("nv_diag_suppress 177")                                                      \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<const TYPE>(idx, arr->moments. BASEVAR);                                                  \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<TYPE>(idx, arr->moments. BASEVAR);                                                        \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of two 2D arrays
///(i. e. per sampling) accessible through @p BASEVAR1 and @p BASEVAR2.
///@remark This is always enough for a 32-bit temporary variable per possible neighbour pair (NExactPairs),
///        or even a 64-bit per neighbour pair if NMaxClusters == NCaloCells.
#define CALORECGPU_TEMPBIGARR_2(TEMPNAME, BASEVAR1, BASEVAR2, TYPE)  _Pragma("nv_diag_suppress 177")                                           \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<const TYPE>(idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                         \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<TYPE>(idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2);                               \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored as a 1D array split among the entries of three 2D arrays
///(i. e. per sampling) accessible through @p BASEVAR1, @p BASEVAR2 and @p BASEVAR3.
///@remark This is a good match for fitting two NExactPairs arrays (in different directions)
///        while still being able to fit two in the six per-sampling variables in the moments.
#define CALORECGPU_TEMPBIGARR_3(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, TYPE)  _Pragma("nv_diag_suppress 177")                                 \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<const TYPE>(idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3); \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer_stacked<TYPE>(idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3);       \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that can have up to three times NMaxClusters, split across three variables.
///@remark This would be a 32-bit temporary variable per cell regardless of NMaxClusters.
#define CALORECGPU_TEMPCELLARR_1(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, TYPE)  _Pragma("nv_diag_suppress 177")                                \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<const TYPE,                                                                                       \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR3)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3);                                                      \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<TYPE,                                                                                             \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR3)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3);                                                      \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///The definitions for a temporary variable of type @p TYPE with name @p TEMPNAME
///that can have up to three times NMaxClusters, split across six variables.
///@remark This would be a 64-bit temporary variable per cell regardless of NMaxClusters.
#define CALORECGPU_TEMPCELLARR_2(TEMPNAME, BASEVAR1, BASEVAR2, BASEVAR3, BASEVAR4, BASEVAR5, BASEVAR6, TYPE)  _Pragma("nv_diag_suppress 177")  \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr, const unsigned int idx) \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<const TYPE,                                                                                       \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR3),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR4),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR5),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR6)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3,                                                       \
            arr->moments. BASEVAR4, arr->moments. BASEVAR5, arr->moments. BASEVAR6);                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr, const unsigned int idx)    \
  {                                                                                                                                            \
    return CaloRecGPU::get_laundered_pointer<TYPE,                                                                                             \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR1),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR2),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR3),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR4),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR5),                                                                  \
           offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR6)>                                                                  \
           (idx, arr->moments. BASEVAR1, arr->moments. BASEVAR2, arr->moments. BASEVAR3,                                                       \
            arr->moments. BASEVAR4, arr->moments. BASEVAR5, arr->moments. BASEVAR6);                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr, const unsigned int idx)                             \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr, const unsigned int idx)                                         \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr, idx);                                                                           \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon


///Defines a temporary variable of type @p TYPE with name @p TEMPNAME
///that will be stored in the @p INDEX entry of an 1D array.
///@remark The @p INDEX represents the index of the array
///        of @p TYPE, not the offset within the original array.
///@warning The address will be appropriately aligned, so, for bigger types,
///         the maximum number of variables that can be stored in a single array
///         may be smaller, depending on the necessary alignment.
#define CALORECGPU_TEMPVAR(TEMPNAME, BASEVAR, INDEX, TYPE)  _Pragma("nv_diag_suppress 177")                                                    \
  template <class PtrLike> __host__ __device__ const TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (const PtrLike & arr)                \
  {                                                                                                                                            \
    static_assert(CaloRecGPU::check_sufficient_size<TYPE>(offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR), INDEX));           \
    return CaloRecGPU::get_laundered_pointer<const TYPE, offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR)>(INDEX, arr->moments. BASEVAR); \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE * CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr) (PtrLike & arr)                            \
  {                                                                                                                                            \
    static_assert(CaloRecGPU::check_sufficient_size<TYPE>(offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR), INDEX));           \
    return CaloRecGPU::get_laundered_pointer<TYPE, offsetof(CaloRecGPU::ClusterInfoArr::ClusterMomentsArr, BASEVAR)>(INDEX, arr->moments. BASEVAR); \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ const TYPE & TEMPNAME (const PtrLike & arr)                                                     \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr);                                                                                \
  }                                                                                                                                            \
  template <class PtrLike> __host__ __device__ TYPE & TEMPNAME (PtrLike & arr)                                                                 \
  {                                                                                                                                            \
    return *CALORECGPU_TEMP_CONCAT_HELPER(TEMPNAME, _ptr)(arr);                                                                                \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon

///Wraps a function in another name, for better semantics while reusing storage...
#define CALORECGPU_TEMPWRAPPER(TEMPNAME, WRAPPED)  _Pragma("nv_diag_suppress 177")                                                             \
  template <class PtrLike, class ... Args> __host__ __device__ decltype(auto) TEMPNAME (PtrLike && p, Args && ... args)                        \
  {                                                                                                                                            \
    return std::forward<PtrLike>(p)-> WRAPPED (std::forward<Args>(args)...);                                                                   \
  } _Pragma("nv_diag_default 177") struct to_end_with_semicolon
  
#endif

#endif
