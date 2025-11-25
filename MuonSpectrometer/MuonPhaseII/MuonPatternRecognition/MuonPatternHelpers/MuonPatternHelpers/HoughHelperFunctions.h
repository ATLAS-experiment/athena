/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPatternHelpers_HoughHelperFunctions__H
#define MuonPatternHelpers_HoughHelperFunctions__H

#include "MuonPatternEvent/MuonHoughDefs.h"
#include "Acts/Seeding/HoughTransformUtils.hpp"

namespace MuonR4::HoughHelpers{
    namespace Eta{
        /// @brief left-side straight line parametrisation for drift circles 
        /// @param tanBeta the input inclination angle
        /// @param dc the drift circle (expressed as a space point)
        /// @return the y offset needed to touch the drift radius on the left for an inclination angle tanBeta
        double houghParamMdtLeft(double tanBeta, const MuonR4::HoughHitType & dc); 
        /// @brief right-side straight line parametrisation for drift circles 
        /// @param tanBeta the input inclination angle
        /// @param dc the drift circle (expressed as a space point) 
        /// @return the y offset needed to touch the drift radius on the right for an inclination angle tanBeta
        double houghParamMdtRight(double tanBeta, const MuonR4::HoughHitType & dc); 

        /// @brief straight line parametrisation for strip detector measurements 
        /// @param tanBeta the input inclination angle
        /// @param strip the strip measurement (expressed as a space point)
        /// @return the y offset needed to pass through the center of the strip space point for an inclination angle tanBeta
        double houghParamStrip(double tanBeta, const MuonR4::HoughHitType & strip); 

        /// @brief uncertainty parametrisation for drift circles 
        /// @param tanBeta the input inclination angle 
        /// @param dc the drift circle (expressed as a space point)
        /// @return the uncertainty on the y offset - calculated from an inflated 
        /// drift circle error and a baseline uncertainty to account for the not fully known t0
        double houghWidthMdt(double tanBeta, const MuonR4::HoughHitType & dc, double targetReso); 

        /// @brief Uncertainty parametrisation for strip measurements
        /// @param tanBeta: the input inclination angle (not used) 
        /// @param strip: the strip measurement (expressed as a space point) 
        /// @return the uncertainty on the y offset - based on the strip pitch 
        double houghWidthStrip(double tanBeta, const MuonR4::HoughHitType & strip, double targetReso);
    }
    namespace Phi{
        /// @brief straight line parametrisation for strip detector measurements, in the x-direction 
        /// @param tanAlpha the input inclination angle
        /// @param strip the strip measurement (expressed as a space point)
        /// @return the x offset needed to pass through the center of the strip space point for an inclination angle tanAlpha
        double houghParamStrip(double tanAlpha, const MuonR4::HoughHitType & dc); 

        /// @brief Uncertainty parametrisation for strip measurements
        /// @param tanAlpha: the input inclination angle (not used) 
        /// @param strip: the strip measurement (expressed as a space point) 
        /// @return the uncertainty on the x offset - based on the strip pitch 
        double houghWidthStrip(double tanAlpha, const MuonR4::HoughHitType & dc, double targetReso); 
    }
}

#endif
