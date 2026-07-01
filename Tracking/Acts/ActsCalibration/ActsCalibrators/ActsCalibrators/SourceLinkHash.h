/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSCALIBRATORS_SOURCELINKHASH_H
#define ACTSCALIBRATORS_SOURCELINKHASH_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "Acts/EventData/SourceLink.hpp"

/** @brief Hash functions to pack the source link into unordered_maps / unordered_sets */
namespace ActsTrk::detail{
    /** @brief Calculates the source link hash which is evaluated to be
     *         the identifier of the underlying sourcelink */
    std::size_t sourceLinkHash(const Acts::SourceLink& sl);
    /** @brief Returns whether two source links are equal */
    bool sourceLinkEquality(const Acts::SourceLink&a, const Acts::SourceLink& b);
}



#endif