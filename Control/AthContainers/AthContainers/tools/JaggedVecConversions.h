// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/JaggedVecConversions.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Conversions for accessing jagged vector variables.
 */


#ifndef ATHCONTAINERS_JAGGEDVECCONVERSIONS_H
#define ATHCONTAINERS_JAGGEDVECCONVERSIONS_H


#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainersInterfaces/AuxDataSpan.h"
#include "CxxUtils/range_with_conv.h"
#include <cassert>
#include <stdexcept>


namespace SG { namespace detail {


/**
 * @brief Helper: Make a span from a jagged vector element.
 *        An instance of this class is constructed from an @c AuxDataSpan
 *        over the payload vector.  (This has a reference to the range
 *        held by the @c AuxTypeVector, so if the payload vector changes,
 *        we'll see it here.)  @c operator() then takes a @c JaggedVecElt
 *        and produces a span representing that element.
 */
template <class PAYLOAD_T>
class JaggedVecConstConverter
{
public:
  /// The type of the jagged vector payload.
  using Payload_t = PAYLOAD_T;

  /// An @c AuxDataSpan representing the payload vector.
  using const_Payload_span = detail::AuxDataSpan<Payload_t>;

  /// The type of the span we produce.
  using element_type = CxxUtils::range_with_conv<typename AuxDataTraits<Payload_t>::const_span>;


  /**
   * @brief Constructor.
   * @param payload Span over the payload vector.
   */
  JaggedVecConstConverter (const_Payload_span payload);


  /**
   * @brief Convert to a span.
   * @param elt The jagged vector element to transform.
   */
  const element_type operator() (const JaggedVecEltBase& elt) const;


private:
  /// The span over the payload vector.
  const_Payload_span m_payload;
};


} } // namespace SG::detail


#include "AthContainers/tools/JaggedVecConversions.icc"


#endif // not ATHCONTAINERS_JAGGEDVECCONVERSIONS_H
