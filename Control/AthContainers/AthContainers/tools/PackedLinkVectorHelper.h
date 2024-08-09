// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/PackedLinkVectorHelper.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Helper functions for managing @c PackedLink variables.
 */


#ifndef ATHCONTAINERS_PACKEDLINKVECTORHELPER_H
#define ATHCONTAINERS_PACKEDLINKVECTORHELPER_H


#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainers/tools/CurrentEventStore.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include "CxxUtils/span.h"
#include "CxxUtils/ranges.h"
#include <ranges>
#include <variant>
#include <utility>


namespace SG::detail {


/// Concept for a range of things that can convert to @c ElementLink<CONT>.
template <class RANGE, class CONT>
concept ElementLinkRange = CxxUtils::InputRangeOverT<RANGE, ElementLink<CONT> >;


/**
 * @brief Helper functions for managing @c PackedLink variables.
 *
 * Functions that don't depend on the target type.
 */
class PackedLinkVectorHelperBase
{
public:
#ifdef XAOD_STANDALONE
  using IProxyDict = AthContainers::IProxyDict;

  // To reduce the number of template instantiations, we want to be able
  // to treat the vectors of DataLinks as just vectors of DataLinkBase.
  // That works fine in Athena, where all the data members are in
  // DataLinkBase.  But in AnalysisBase, DataLink<T> adds more
  // data members.  So for this case, use a dummy DataLink type
  // rather than DataLinkBase.
  using DataLinkBase = DataLink<std::vector<float> >;
#else
  using DataLinkBase = ::DataLinkBase;
#endif

  /// Function to initialize a @c DataLink to a given hashed key.
  typedef void InitLinkFunc_t (DataLinkBase& dl,
                               sgkey_t sgkey,
                               IProxyDict* sg);


  /// Spans over the linked @c DataLinks.
  /// These hold the span data by reference, so they update if the
  /// underlying vector changes.
  using DataLinkBase_span = detail::AuxDataSpan<DataLinkBase>;
  using const_DataLinkBase_span = detail::AuxDataConstSpan<DataLinkBase>;


  /**
   * @brief Helper to access the @c IAuxTypeVector for an aux variable.
   *
   * Sometimes we may have an @c IAuxTypeVector directly, but sometimes
   * we may need to get it from an @c AuxVectorData an an aux variable ID.
   * The latter case has some significant overhead, though, so first
   * we want to defer doing that until we actually need, and second,
   * we want to cache the result when we get it.  That is the purpose
   * of this class.
   */
  class LinkedVector
  {
  public:
    /**
     * @brief Constructor from an @c IAuxTypeVector directly.
     * @param linkedVector The @c IAuxTypeVector for the aux variable.
     */
    LinkedVector (IAuxTypeVector& linkedVector);


    /**
     * @brief Constructor from a container and aux ID.
     * @param container Container holding the variables.
     * @param auxid The ID of the PackedLink variable.
     */
    LinkedVector (AuxVectorData& container, SG::auxid_t auxid);


    /**
     * @brief Return the @c IAuxTypeVector.
     */
    IAuxTypeVector* get();


    /**
     * @brief Return the @c IAuxTypeVector.
     */
    IAuxTypeVector& operator*();


    /**
     * @brief Return the @c IAuxTypeVector.
     */
    IAuxTypeVector* operator->();

  private:
    // Holds either a pointer to the @c IAuxTypeVector or a container, ID pair.
    std::variant<IAuxTypeVector*,
                 std::pair<AuxVectorData*, SG::auxid_t> > m_data;
  };


  /// Span over links.
  // Can't use std::span now because we want span::at(), which is C++26.
  using PackedLinkBase_span = CxxUtils::span<PackedLinkBase>;


  /**
   * @brief Return the current store from the first valid link in the span.
   *        If there are no valid links, then return the global default.
   * @param links The span of links.
   */
  static IProxyDict* storeFromSpan (DataLinkBase_span& links);


  /**
   * @brief Return a span over all the linked @c DataLinkBase's,
   *        from the linked vector.
   * @param linkedVec Interface for the linked vector of @c DataLinkBase's.
   */
  static DataLinkBase_span getLinkBaseSpan (IAuxTypeVector& linkedVec);


  /**
   * @brief Return a span over all the linked DataLinks.
   *        from the linked vector.
   * @param linkedVec Interface for the linked vector of DataLinks.
   */
  static const_DataLinkBase_span getLinkBaseSpan (const IAuxTypeVector& linkedVec);


  /**
   * @brief Find the collection index in the linked @c DataLinks for a sgkey.
   * @param linkedVec How to find the linked vector.
   * @param sgkey Hashed key for which to search.
   * @param links Span over the vector of @c DataLinks, as @c DataLinkBase.
   *              May be modified if the vector grows.
   * @param sg The @c IProxyDict of the current store.
   *           May be null to use the global, thread-local default.
   * @param initFunc Function to initialize a @c DataLink to a given
   *                 hashed key.
   *
   * Searches for a @c DataLinkBase matching @c sgkey in the linked vector.
   * If not found, then a new entry is added.
   * Returns a pair (INDEX, CACHEVALID).  INDEX is the index in the vector
   * of the sgkey (and thus the collection index to store in the @c PackedLink).
   * CACHEVALID is true if it is known that the payload of the vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   *
   * This does not actually initialize the @c DataLink if the vector grows.
   * The caller should test if the size of the span changed and if so,
   * initialize the last element using @c sgkey.
   */
  static std::pair<size_t, bool>
  findCollectionBase (LinkedVector& linkedVec,
                      sgkey_t sgkey,
                      DataLinkBase_span& links,
                      IProxyDict* sg,
                      InitLinkFunc_t* initLinkFunc);


  /**
   * @brief Update collection index of a collection of @c PackedLink.
   * @param linkedVec Interface for the linked vector of @c DataLinks.
   * @param links Span over the links to update, as @c PackedLinkBase.
   * @param srcDLinks Span over the source link vector, as @c DataLinkBase.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   * @param initFunc Function to initialize a @c DataLink to a given
   *                 hashed key.
   *
   * To be used after links have been copied/moved from one container
   * to another.  The collection indices are updated to be appropriate
   * for the destination container.
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static bool updateLinksBase (IAuxTypeVector& linkedVec,
                               PackedLinkBase_span& links,
                               const const_DataLinkBase_span& srcDLinks,
                               IProxyDict* sg,
                               InitLinkFunc_t* initLinkFunc);


#ifndef XAOD_STANDALONE
  /**
   * @brief Apply thinning to packed links, to prepare them for output.
   * @param linkedVec Interface for the linked vector of @c DataLinks.
   * @param links Span over the links to update, as @c PackedLinkBase.
   * @param dlinks Span over the source link vector, as @c DataLinkBase.
   * @param tc The @c ThinningCache for this object, if it exists.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   * @param initFunc Function to initialize a @c DataLink to a given
   *                 hashed key.
   *
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static bool applyThinningBase (IAuxTypeVector& linkedVec,
                                 PackedLinkBase_span& links,
                                 DataLinkBase_span& dlinks,
                                 const SG::ThinningCache* tc,
                                 IProxyDict* sg,
                                 InitLinkFunc_t* initLinkFunc);
#endif


private:
  /**
   * @brief Resize a linked vector of @c DataLinks.
   * @param linkedVec How to find the linked vector.
   * @param sz The new size of the container.
   *
   * Returns true if it is known the the underlying vector did not move.
   */
  static bool resizeLinks (LinkedVector& linkedVec, size_t sz);
};


/**
 * @brief Helper functions for managing @c PackedLink variables.
 *
 * The template argument is the target type of the links.
 */
template <class CONT>
class PackedLinkVectorHelper
  : public PackedLinkVectorHelperBase
{
public:
  /// The (linked) data links.
  using Link_t = ElementLink<CONT>;
  using DLink_t = DataLink<CONT>;
  using DataLink_span = detail::AuxDataSpan<DataLink<CONT> >;
  using const_DataLink_span = detail::AuxDataConstSpan<DataLink<CONT> >;

  /// The PackedLinks.
  using PLink_t = PackedLink<CONT>;
  using PackedLink_span = CxxUtils::span<PLink_t>;


  /**
   * @brief Find the collection index in the linked DataLinks for a sgkey.
   * @param linkedVec How to find the linked vector.
   * @param sgkey Hashed key for which to search.
   * @param links Span over the vector of DataLinks.
   *              May be modified if the vector grows.
   * @param sg The @c IProxyDict of the current store.
   *           May be null to use the global, thread-local default.
   *
   * Searches for a @c DataLink matching @c sgkey in the linked vector.
   * If not found, then a new entry is added.
   * Returns a pair (INDEX, CACHEVALID).  INDEX is the index in the vector
   * of the sgkey (and thus the collection index to store in the @c PackedLink).
   * CACHEVALID is true if it is known that the payload of the vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static std::pair<size_t, bool>
  findCollection (LinkedVector& linkedVec,
                  sgkey_t sgkey,
                  DataLinkBase_span& links,
                  IProxyDict* sg);


  /**
   * @brief Update collection index of a collection of @c PackedLink.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param ptr Pointer to the start of the PackedLink collection.
   * @param n Length of the PackedLink collection.
   * @param srcDLinks Span over the vector of DataLinks
   *                  for the source container of the links.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   *
   * To be used after links have been copied/moved from one container
   * to another.  The collection indices are updated to be appropriate
   * for the destination container.
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static bool updateLinks (IAuxTypeVector& linkedVec,
                           SG::PackedLink<CONT>* ptr,
                           size_t n,
                           const const_DataLinkBase_span& srcDLinks,
                           IProxyDict* sg);


  /**
   * @brief Update collection index of a collection of @c PackedLink.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param ptr Pointer to the start of the PackedLink collection.
   * @param n Length of the PackedLink collection.
   * @param srclv Interface for the linked vector
   *              for the source container of the links.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   *
   * To be used after links have been copied/moved from one container
   * to another.  The collection indices are updated to be appropriate
   * for the destination container.
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static bool updateLinks (IAuxTypeVector& linkedVec,
                           SG::PackedLink<CONT>* ptr,
                           size_t n,
                           const IAuxTypeVector& srclv,
                           IProxyDict* sg);


 /**
   * @brief Update collection index of a collection of @c PackedLink vectors.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param ptr Pointer to the start of the PackedLink vector collection.
   * @param n Length of the PackedLink vector collection.
   * @param srclv Interface for the linked vector
   *              for the source container of the links.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   *
   * To be used after links have been copied/moved from one container
   * to another.  The collection indices are updated to be appropriate
   * for the destination container.
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  template <class VALLOC>
  static bool updateLinks (IAuxTypeVector& linkedVec,
                           std::vector<SG::PackedLink<CONT>, VALLOC>* ptr,
                           size_t n,
                           const IAuxTypeVector& srclv,
                           IProxyDict* sg);


  /**
   * @brief Assign a range of @c ElementLink to a vector of @c Packedlink.
   * @param vect The vector of @c PackedLink to which to assign.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param dlinks Span over the link vector, as @c DataLinkBase.
   * @param x The range to assign.
   *
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  template <class VALLOC, ElementLinkRange<CONT> RANGE>
  static bool assignVElt (std::vector<PLink_t, VALLOC>& velt,
                          IAuxTypeVector& linkedVec,
                          DataLinkBase_span& dlinks,
                          const RANGE& x);


  /**
   * @brief Insert a range of @c ElementLink into a vector of @c Packedlink.
   * @param vect The vector of @c PackedLink to which to assign.
   * @param pos The position at which to do the insertion.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param dlinks Span over the link vector, as @c DataLinkBase.
   * @param x The range to assign.
   *
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  template <class VALLOC, ElementLinkRange<CONT> RANGE>
  static bool insertVElt (std::vector<PLink_t, VALLOC>& velt,
                          size_t pos,
                          IAuxTypeVector& linkedVec,
                          DataLinkBase_span& dlinks,
                          const RANGE& x);


#ifndef XAOD_STANDALONE
  /**
   * @brief Apply thinning to packed links, to prepare them for output.
   * @param linkedVec Interface for the linked vector of @c DataLinks.
   * @param ptr Pointer to the start of the PackedLink collection.
   * @param n Length of the PackedLink collection.
   * @param dlinks Span over the source link vector, as @c DataLinkBase.
   * @param tc The @c ThinningCache for this object, if it exists.
   * @param sg The @c IProxyDict of the current store.
   *           If null, take it from the links in @c srcDlinks,
   *           or use the global, thread-local default.
   *
   * Returns true if it is known that the payload of the linked vector
   * has not moved.  (If this is false, any caches/iterators must be assumed
   * to be invalid.)
   */
  static bool applyThinning (IAuxTypeVector& linkedVec,
                             PackedLink<CONT>* ptr,
                             size_t n,
                             DataLinkBase_span& dlinks,
                             const SG::ThinningCache* tc,
                             IProxyDict* sg);
#endif


private:
  // We want to be able to deal with spans over the base classes.
  static_assert (sizeof (PLink_t) == sizeof(PackedLinkBase));
  static_assert (sizeof (DLink_t) == sizeof(DataLinkBase));


  /**
   * @brief Initialize a @c DataLink.
   * @param dl The link to initialize.  Really of type @c DLink_t.
   * @param sgkey Hashed key to which to initialize the link.
   * @param sg The @c IProxyDict of the current store.
   *           May be null to use the global, thread-local default.
   */
  static void initLink (DataLinkBase& dl, sgkey_t sgkey, IProxyDict* sg);
};


} // namespace SG::detail


#include "AthContainers/tools/PackedLinkVectorHelper.icc"


#endif // not ATHCONTAINERS_PACKEDLINKVECTORHELPER_H
