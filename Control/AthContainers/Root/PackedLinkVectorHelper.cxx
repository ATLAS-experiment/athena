/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/PackedLinkVectorHelper.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Helper functions for managing @c PackedLink variables.
 */


#include "AthContainers/tools/PackedLinkVector.h"


namespace SG::detail {


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
std::pair<size_t, bool>
PackedLinkVectorHelperBase::findCollectionBase (LinkedVector& linkedVec,
                                                sgkey_t sgkey,
                                                DataLinkBase_span& links,
                                                IProxyDict* sg,
                                                InitLinkFunc_t* initLinkFunc)
{
  if (sgkey == 0) {
    if (!links.empty())
      return std::make_pair (0u, true);
  }
  else {
    for (size_t i = 1; i < links.size(); ++i) {
      if (links[i].key() == sgkey) return std::make_pair (i, true);
    }
  }
  size_t oldsz = links.size();
  size_t newsz = oldsz+1;
  if (oldsz == 0 && sgkey != 0) {
    newsz = 2;
  }
  bool cacheValid = resizeLinks (linkedVec, newsz);
  initLinkFunc (links.back(), sgkey, sg);
  return std::make_pair (links.size() - 1, cacheValid);
}


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
bool
PackedLinkVectorHelperBase::updateLinksBase (IAuxTypeVector& linkedVec,
                                             PackedLinkBase_span& links,
                                             const const_DataLinkBase_span& srcDLinks,
                                             IProxyDict* sg,
                                             InitLinkFunc_t* initLinkFunc)
{
  DataLinkBase_span DLinks = getLinkBaseSpan (linkedVec);
  if (sg == nullptr) {
    sg = storeFromSpan (DLinks);
  }
  bool cacheValid = true;
  for (PackedLinkBase& l : links) {
    size_t in_coll = l.collection();
    if (in_coll > 0) {
      LinkedVector lv (linkedVec);
      auto [index, thisValid] = findCollectionBase
        (lv, srcDLinks.at(in_coll).key(), DLinks, sg, initLinkFunc);
      l.setCollection (index);
      cacheValid &= thisValid;
    }
  }
  return cacheValid;
}


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
bool
PackedLinkVectorHelperBase::applyThinningBase (IAuxTypeVector& linkedVec,
                                               PackedLinkBase_span& links,
                                               DataLinkBase_span& dlinks,
                                               const SG::ThinningCache* tc,
                                               IProxyDict* sg,
                                               InitLinkFunc_t* initLinkFunc)
{
  bool cacheValid = true;
  for (PackedLinkBase& l : links) {
    if (l.collection() != 0) {
      sgkey_t sgkey = dlinks[l.collection()].key();
      size_t index = l.index();
      sgkey_t sgkey_out = sgkey;
      size_t index_out = index;
      if (sg) {
        sg->tryELRemap (sgkey, index, sgkey_out, index_out);
      }
      (void)DataProxyHolder::thin (sgkey_out, index_out, tc);
      l.setIndex (index_out);
      if (sgkey_out != sgkey) {
        LinkedVector lv (linkedVec);
        auto [index, flag] = findCollectionBase (lv, sgkey_out, dlinks,
                                                 sg, initLinkFunc);
        l.setCollection (index);
        cacheValid &= flag;
      }
    }
  }
  return cacheValid;
}
#endif


/**
 * @brief Resize a linked vector of @c DataLinks.
 * @param linkedVec How to find the linked vector.
 * @param sz The new size of the container.
 *
 * Returns true if it is known the the underlying vector did not move.
 */
bool
PackedLinkVectorHelperBase::resizeLinks (LinkedVector& linkedVec, size_t sz)
{
  return linkedVec->resize (sz);
}


} // namespace SG::detail
