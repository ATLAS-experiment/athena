// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/PackedLinkConversions.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Conversions between PackedLink and ElementLink.
 */


#ifndef ATHCONTAINERS_PACKEDLINKCONVERSIONS_H
#define ATHCONTAINERS_PACKEDLINKCONVERSIONS_H


#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainers/tools/PackedLinkVectorHelper.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include "CxxUtils/ranges.h"
#include "CxxUtils/range_with_at.h"
#include "CxxUtils/range_with_conv.h"
#include <vector>


namespace SG { namespace detail {


/**
 * @brief Helper: Convert a PackedLink to an ElementLink.
 */
template <class CONT>
class PackedLinkConstConverter
{
public:
  using PLVH = PackedLinkVectorHelper<CONT>;


  /// Resulting ElementLink type.
  using value_type = ElementLink<CONT>;


  /// Type of span over DataLinks.
  using const_DataLink_span = typename PLVH::const_DataLink_span;


  /**
   * @brief Constructor.
   * @param dlinks Span over DataLinks.
   */
  PackedLinkConstConverter (const const_DataLink_span& dlinks);


  /**
   * @brief Convert a PackedLink to an ElementLink.
   * @param plink The link to transform.
   */
  const value_type operator() (const PackedLinkBase& plink) const;


private:
  /// Span over DataLinks.
  const_DataLink_span m_dlinks;
};


/**
 * @brief Helper: Convert a vector of PackedLink to a span over ElementLinks.
 */
template <class CONT>
class PackedLinkVectorConstConverter
{
public:
  using PLVH = PackedLinkVectorHelper<CONT>;


  /// A span over the input PackedLink objects.
  using const_PackedLink_span = typename AuxDataTraits<PackedLink<CONT> >::const_span;

  /// Transform the span of PackedLinks to a span of ElementLinks.
  using value_type =
    CxxUtils::range_with_conv<
      CxxUtils::transform_view_with_at<const_PackedLink_span,
                                       detail::PackedLinkConstConverter<CONT> > >;


  /// Type of span over DataLinks.
  using const_DataLink_span = typename PLVH::const_DataLink_span;


  /**
   * @brief Constructor.
   * @param dlinks Span over DataLinks.
   */
  PackedLinkVectorConstConverter (const const_DataLink_span& dlinks);


  /**
   * @brief Convert a vector of PackedLinks to a span over ElementLinks.
   * @param velt The vector to transform.
   */
  template <class VALLOC>
  value_type operator() (const std::vector<PackedLink<CONT>, VALLOC>& velt) const;


private:
  /// Span over DataLinks.
  const_DataLink_span m_dlinks;
};


/**
 * @brief Helper: Convert a PackedLink to an ElementLink and vice-versa.
 */
template <class CONT>
class PackedLinkConverter
{
private:
  /// Helper class for maintaining the DataLinks.
  using PLVH = detail::PackedLinkVectorHelper<CONT>;


public:
  /// Resulting ElementLink type.
  using value_type = typename PLVH::Link_t;

  /// The PackedLink type.
  using PLink_t = typename PLVH::PLink_t;

  /// The linked DataLink type.
  using DLink_t = typename PLVH::DLink_t;

  /// Type of span over DataLinkBase's.
  using DataLinkBase_span = typename PLVH::DataLinkBase_span;


  /**
   * @brief Constructor.
   * @param container Container holding the variables.
   * @param auxid The ID of the PackedLink variable.
   * @param linked_auxid The ID of the linked variable of DataLinks.
   */
  PackedLinkConverter (AuxVectorData& container,
                       SG::auxid_t auxid,
                       SG::auxid_t linked_auxid);


  /**
   * @brief Convert a PackedLink to an ElementLink.
   * @param plink The link to transform.
   */
  const value_type operator() (const PackedLinkBase& plink) const;


  /**
   * @brief Convert an ElementLink to a PackedLink.
   * @param pl The destination PackedLink.
   * @param link The link to transform.
   */
  void set (PackedLinkBase& pl, const value_type& link);


  /**
   * @brief Convert a range of ElementLinks to a vector of PackedLinks.
   * @param plv The destination vector of PackedLinks.
   * @param r The range of ElementLinks.
   */
  template <class VALLOC, ElementLinkRange<CONT> RANGE>
  void set (std::vector<PLink_t, VALLOC>& plv, const RANGE& r);


  /**
   * @brief Insert a range of ElementLinks into a vector of PackedLinks.
   * @param plv The destination vector of PackedLinks.
   * @param pos Position in the container at which to insert the range.
   * @param r The range of ElementLinks.
   */
  template <class VALLOC, ElementLinkRange<CONT> RANGE>
  void insert (std::vector<PLink_t, VALLOC>& plv, size_t pos, const RANGE& r);


private:
  /// The container holding the variables.
  AuxVectorData& m_container;

  /// The vector of DataLinks.
  PLVH::LinkedVector m_linkedVec;

  /// Span over DataLinks.
  DataLinkBase_span m_dlinks;
};


} } // namespace SG::detail


#include "AthContainers/tools/PackedLinkConversions.icc"


#endif // not ATHCONTAINERS_PACKEDLINKCONVERSIONS_H
