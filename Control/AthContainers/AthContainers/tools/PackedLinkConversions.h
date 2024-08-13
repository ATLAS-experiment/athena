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


} } // namespace SG::detail


#include "AthContainers/tools/PackedLinkConversions.icc"


#endif // not ATHCONTAINERS_PACKEDLINKCONVERSIONS_H
