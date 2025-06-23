/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODEGAMMA_EGAMMADETAILS_H
#define XAODEGAMMA_EGAMMADETAILS_H

#include "xAODEgamma/PhotonxAODHelpers.h"
#include "xAODEgamma/ElectronxAODHelpers.h"

namespace xAOD
{
  namespace EgammaDetails
  {
    /// @file EgammaDetails.h
    ///
    /// Helper functions for EgammaDetails accessors
    ///
    /// This file contains helper functions for more complex accessors
    /// in xAOD::Egamma, xAOD::Electron, xAOD::Photon, and EgammaHelpers
    /// that are shared with the columnar environment.  These are all
    /// implemented as inline standalone functions that get all the
    /// input variables passed in.  That completely separates them from
    /// the xAOD classes/functions, as well as from the columnar
    /// environment.
    ///
    /// This is implemented as header-only inline code, which should
    /// give it large flexibility in how it can be used without adding
    /// additional overhead.  If any of these functions is too heavy it
    /// can later on be turned into a non-inlined function.
    ///
    /// @warn None of these functions are meant to be called by the user
    /// directly.  These are meant as the backend implementations of the
    /// corresponding accessors in xAOD::EgammaDetails and the
    /// corresponding columnar accessors.

    using EgammaParameters::ConversionType;

    ///@brief return the photon conversion type (see EgammaEnums)
    inline ConversionType conversionType(const bool hasTrk1, const bool hasTrk2, const std::uint8_t nSiHits1, const std::uint8_t nSiHits2) {

      if (!hasTrk1) {return xAOD::EgammaParameters::unconverted;}

      if (!hasTrk2)
        {return nSiHits1 ? xAOD::EgammaParameters::singleSi : xAOD::EgammaParameters::singleTRT;}
      
      if (nSiHits1 && nSiHits2){
        return xAOD::EgammaParameters::doubleSi;
      }
      if (nSiHits1 || nSiHits2){
        return xAOD::EgammaParameters::doubleSiTRT;
      }  
      else{
        return xAOD::EgammaParameters::doubleTRT;
      }
    }

    ///@brief is the object a converted photon
    inline bool isConvertedPhoton (const bool excludeTRT, const float eta, const std::size_t nVertices, const ConversionType conversionType) {
      const bool hasVertices = nVertices > 0;
      if (excludeTRT) {
        // special case for Run3: consider unconv if TRT Conv in the barrel
        using enum EgammaParameters::ConversionType;
        const bool isTRTConv = (conversionType == singleTRT) || (conversionType == doubleTRT);
        return hasVertices && (std::abs(eta) > 0.8 || !isTRTConv);
      }
      return hasVertices;
    }
  }
}

#endif
