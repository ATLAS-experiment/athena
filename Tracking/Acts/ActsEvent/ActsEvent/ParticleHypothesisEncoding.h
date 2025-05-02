/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_ParticleHypothesisEncoding_h
#define ActsEvent_ParticleHypothesisEncoding_h
#include "xAODTracking/TrackingPrimitives.h"
#include "Acts/EventData/ParticleHypothesis.hpp"
#include "TrkEventPrimitives/ParticleHypothesis.h"

namespace ActsTrk {
namespace ParticleHypothesis {
    xAOD::ParticleHypothesis convert(Acts::ParticleHypothesis h);
    Acts::ParticleHypothesis convert(xAOD::ParticleHypothesis h);
    Acts::ParticleHypothesis convert(Trk::ParticleHypothesis h);
}  // namespace ParticleHypothesis
}  // namespace ActsTrk

#endif