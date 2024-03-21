/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef TRACKTOGENPARTICLEASSOCIATION_H
#define TRACKTOGENPARTICLEASSOCIATION_H
#include "boost/container/small_vector.hpp"
#include "xAODTruth/TruthParticle.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include <vector>
#include <array>
#include <cstdint>
#include <utility>

namespace ActsTrk
{
   constexpr unsigned int NHitCounter = static_cast< std::underlying_type<xAOD::UncalibMeasType>::type >(xAOD::UncalibMeasType::sTgcStripType)+1u;
   constexpr unsigned int NTruthParticlesPerTrack = 5;  // a tiny fraction of measurements will have more than
                                                        // 5 associated GenParticles

   // Wrapper around std::array to ensure that by default elements are initialised to zero
   class  HitCounterArray : public  std::array<uint8_t,  NHitCounter>
   {
   public:
      HitCounterArray() : std::array<uint8_t,  NHitCounter>{} {}
   };

   /** Container for hit counts per track
    * Contains hit counts per associated truth particle and the total hit counts
    * As last element in the container.
    * The sub-container returned by @ref countsPerTruthParticle contains
    * the subset where each of the pairs contains a valid pointer to a truth
    * particle and its associated hit counts.
    */
   class HitCountsPerTrack {
   public:
      using container = boost::container::small_vector<std::pair<const xAOD::TruthParticle *, HitCounterArray >, NTruthParticlesPerTrack>;

      /** vector with counts per associated truth particle
       */
      const container &countsPerTruthParticle() const { return m_counts; }

      /** vector with counts per associated truth particle (read only)
       */
      container &countsPerTruthParticle()             { return m_counts; }

      /** Total hit counts per track.
       */
      HitCounterArray &totalCounts()               { return m_recoCounts[kTotal]; }

      /** Total hit counts per track (read only).
       */
      const HitCounterArray &totalCounts() const   { return m_recoCounts[kTotal]; }

      /** Noise hit counts per track.
       */
      HitCounterArray &noiseCounts()               { return m_recoCounts[kNoise]; }

      /** Noise hit counts per track (read only).
       */
      const HitCounterArray &noiseCounts() const   { return m_recoCounts[kNoise]; }

   private:
      container m_counts;
      enum ECounts {kTotal, kNoise, kNRecoCounts };
      std::array<HitCounterArray,kNRecoCounts> m_recoCounts;
   };

   using TrackToTruthParticleAssociation = std::vector<HitCountsPerTrack> ;
}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TrackToTruthParticleAssociation, 101405904, 1 )

#endif
