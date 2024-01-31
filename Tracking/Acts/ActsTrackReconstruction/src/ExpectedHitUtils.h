/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
/* Dear emacs, this is -*-c++-*- */
#ifndef ACTSTRK_EXPECTEDHITUTILS_H
#define ACTSTRK_EXPECTEDHITUTILS_H

#include "GaudiKernel/EventContext.h"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include <array>

class IActsExtrapolationTool;

namespace ActsTrk
{
   /** Extrapolate from the perigee outwards and gather information which detector layers should have hits.
    * @param ctx the current athena EvetContext
    * @paran extrapolator referece to the Acts extrapolation tool
    * @param perigee_parameters the Acts defining track parameters (parameters at the perigee).
    * @param extrapolation_volume the volume around the perigee which is considered (e.g. pixel detector volume).
    * @return array of layer pattern words: two pairs for pixel and strips containing the barrel and end caps bit pattern with one bit per detector layer.
    * Will extrapolate from the perigee to the given cylinder volume surface and gather the layers which are crossed and on
    * which hits are expected.
    */
  std::array<unsigned int,4> expectedLayerPattern(const EventContext& ctx,
                                                  const IActsExtrapolationTool &extrapolator,
                                                  Acts::BoundTrackParameters perigee_parameters,
                                                  const Acts::CylinderSurface &extrapolation_volume);
}
#endif
