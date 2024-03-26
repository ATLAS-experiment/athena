/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef TRUTHPARTICLEHITCOUNTS_H
#define TRUTHPARTICLEHITCOUNTS_H
#include <unordered_map>
#include "xAODTruth/TruthParticle.h"
#include "ActsEvent/TrackToTruthParticleAssociation.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include <array>
#include <cstdint>

namespace ActsTrk
{
   // constexpr unsigned int NHitCounter = static_cast< std::underlying_type<xAOD::UncalibMeasType>::type >(xAOD::UncalibMeasType::sTgcStripType)+1u;
   // constexpr unsigned int NTruthParticlesPerTrack = 5;  // a tiny fraction of measurements will have more than
   //                                                      // 6 associated GenParticles
   // using HitCounterArray = std::array<uint8_t,  NHitCounter>;
   using TruthParticleHitCounts = std::unordered_map<const xAOD::TruthParticle *,HitCounterArray> ;
}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TruthParticleHitCounts, 226325923, 1 )

#endif
