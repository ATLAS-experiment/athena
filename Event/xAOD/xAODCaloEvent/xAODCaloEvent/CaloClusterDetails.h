/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCALOEVENT_CALOCLUSTERDETAILS_H
#define XAODCALOEVENT_CALOCLUSTERDETAILS_H

#include <CaloGeoHelpers/CaloSampling.h>
#include <bit>
#include <cstdint>
#include <span>

namespace xAOD
{
  namespace CaloClusterDetails
  {
    /// @file CaloClusterDetails.h
    ///
    /// Helper functions for CaloCluster accessors
    ///
    /// This file contains helper functions for more complex accessors
    /// in xAOD::CaloCluster that are shared with the columnar
    /// environment.  These are all implemented as inline standalone
    /// functions that get all the input variables passed in.  That
    /// completely separates them from the xAOD::CaloCluster class, as
    /// well as from the columnar environment.
    ///
    /// This is implemented as header-only inline code, which should
    /// give it large flexibility in how it can be used without adding
    /// additional overhead.  If any of these functions is to heavy it
    /// can later on be turned into a non-inlined function.
    ///
    /// @warn None of these functions are meant to be called by the user
    /// directly.  These are meant as the backend implementations of the
    /// corresponding accessors in xAOD::CaloCluster and the
    /// corresponding columnar accessors.


    using CaloSample = CaloSampling::CaloSample;


    /// the default error code to return in case of error
    ///
    /// FIXME: This value is currently taken from the
    /// `xAOD::CaloCluster` class, and should be changed to a more
    /// sensible value in the future.
    constexpr float defaultErrorValue = -999;


    ///  Checks if certain smapling contributes to cluster
    [[nodiscard]] inline bool hasSampling(const CaloSample s, const std::uint32_t samplingPattern) {
      return (samplingPattern & (0x1U<<(std::uint32_t)s));
    }


    [[nodiscard]] inline unsigned sampVarIdx(const CaloSample s, const std::uint32_t samplingPattern) {
      //std::cout << "Pattern=" << std::hex << pattern << std::dec << ", Sampling=" << s << std::endl;
      if ((samplingPattern & (0x1U << s)) == 0) {
        return CaloSampling::Unknown;
      }
      if (s == 0) {
        return 0;
      } // shifting a 32-bit int by 32 bits is undefined behavior!
      return std::popcount(samplingPattern << (32 - s));
      // Explanation: Need to get the number of bit (=samples) before the sampling in question
      // Shift to the left, so bits after the sampling in question fall off the 32bit integer
      // Then use  popcount to count the numbers of 1 in the rest
    }


    [[nodiscard]] inline float getSamplVar(const CaloSample sampling, const std::uint32_t samplingPattern, const std::span<const float> vec, const float errorvalue=defaultErrorValue) {
      const unsigned idx=sampVarIdx(sampling, samplingPattern);
      if (idx<vec.size() ) {
        return vec[idx];
      }
  
        //std::cout <<Sampling " << sampling << ", Pattern=" << std::hex <<m_samplingPattern << std::dec << ", index=" << idx << " size=" << vec.size() << std::endl;
      return errorvalue;
    }


    /// @brief Get the energy in one layer of the EM Calo
    /// @param layer Layer between 0 (Presampler) and 3 (Back)
    /// @return energy
    /// Works for both, barrel and endcap
    [[nodiscard]] inline float energyBE(const unsigned sample, const std::uint32_t samplingPattern, const std::span<const float> e_sampl) {
      if (sample>3) return defaultErrorValue;
      const CaloSample barrelSample=(CaloSample)(CaloSampling::PreSamplerB+sample);
      const CaloSample endcapSample=(CaloSample)(CaloSampling::PreSamplerE+sample);
      double energy=0;
      if (hasSampling(barrelSample, samplingPattern)) {
        energy+=getSamplVar(barrelSample,samplingPattern,e_sampl); //Check for errorcode? Should not happen...
      }
      if (hasSampling(endcapSample, samplingPattern)) {
        energy+=getSamplVar(endcapSample,samplingPattern,e_sampl);
      }
      return energy;
    }
  }
}

#endif
