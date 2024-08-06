// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/PackedLinkImpl.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Definition of PackedLink type.
 *
 * This file defines the @c PackedLink type used to store @c ElementLinks
 * in packed form.  It is broken out from PackedLink.h due to dependency
 * issues.
 */


#ifndef ATHCONTAINERS_PACKEDLINKIMPL_H
#define ATHCONTAINERS_PACKEDLINKIMPL_H


#ifndef XAOD_STANDALONE
#include "AthLinks/tools/ElementLinkTraits.h"
#endif
#include "CxxUtils/ones.h"
#include <concepts>
#include <cstdint>
#include <cassert>


namespace SG {


/**
 * @brief A packed version of @c ElementLink.
 *
 * This is a packed form of @c ElementLink, which can be used as
 * an auxiliary variable.  It occupies 32 bits.  The high 8 bits specify
 * the collection, as an index into a linked vector of @c DataLink.
 * The remainder gives the element index.  A @c PackedLinkBase that's 0
 * represents a null link.
 *
 * This non-templated base class holds the actual packed value.  However,
 * users should use the @c PackedLink<STORABLE> derived types, to allow
 * specifying the target of the links.
 */
struct PackedLinkBase
{
  static constexpr unsigned NBITS = 32;
  static constexpr unsigned COLLECTION_NBITS = 8;
  static constexpr unsigned INDEX_NBITS = NBITS - COLLECTION_NBITS;
  static constexpr uint32_t COLLECTION_MAX = CxxUtils::ones<uint32_t> (COLLECTION_NBITS);
  static constexpr uint32_t INDEX_MAX = CxxUtils::ones<uint32_t> (INDEX_NBITS);

  /**
   * @brief Default constructor.
   *
   * Initialize a null link.
   */
  PackedLinkBase();


  /**
   * @brief Constructor.
   * @param collection The index of the collection in the @c DataLink vector,
   * @param index The index of the element within the collection.
   */
  PackedLinkBase (unsigned int collection, unsigned int index);


  /**
   * @brief Comparison.
   * @param other The link with which to compare.
   */
  bool operator== (const PackedLinkBase& other) const;


  /**
   * @brief Unpack the collection index from the link.
   */
  unsigned int collection() const;


  /**
   * @brief Unpack the element index from the link.
   */
  unsigned int index() const;


  /**
   * @brief Set the collection index;
   * @param collection The index of the collection in the @c DataLink vector,
   */
  void setCollection (unsigned int collection);


  /**
   * @brief Set the element index;
   * @param index The index of the element within the collection.
   */
  void setIndex (unsigned int index);


private:
  /// The packed form of the link.
  uint32_t m_packed;
};


/**
 * @brief A packed version of @c ElementLink.
 *
 * This represents a link to an element of @c STORABLE, packed into 32 bits.
 */
template <class STORABLE>
#ifndef XAOD_STANDALONE
// To be usable as a packed link, the index must be integral.
requires (std::integral<typename SG::ElementLinkTraits<STORABLE>::IndexingPolicy::index_type>)
#endif
struct PackedLink
  : public PackedLinkBase
{
public:
  using PackedLinkBase::PackedLinkBase;
};


} // namespace SG


#include "AthContainers/PackedLinkImpl.icc"


#endif // not ATHCONTAINERS_PACKEDLINKIMPL_H
