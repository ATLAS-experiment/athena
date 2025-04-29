/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__HoughHelperFunctions__H
#define MUONR4__HoughHelperFunctions__H

#include "MuonPatternEvent/MuonHoughDefs.h"
#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"


namespace MuonR4{
    namespace HoughHelpers{
        namespace Eta{
            /// @brief left-side straight line parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle (expressed as a space point)
            /// @return the y offset needed to touch the drift radius on the left for an inclination angle tanTheta
            double houghParamMdtLeft(double tanTheta, const MuonR4::HoughHitType & dc); 

            /// @brief right-side straight line parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle (expressed as a space point) 
            /// @return the y offset needed to touch the drift radius on the right for an inclination angle tanTheta
            double houghParamMdtRight(double tanTheta, const MuonR4::HoughHitType & dc); 

            /// @brief straight line parametrisation for strip detector measurements 
            /// @param tanTheta the input inclination angle
            /// @param strip the strip measurement (expressed as a space point)
            /// @return the y offset needed to pass through the center of the strip space point for an inclination angle tanTheta
            double houghParamStrip(double tanTheta, const MuonR4::HoughHitType & strip); 

            /// @brief uncertainty parametrisation for drift circles 
            /// @param tanTheta the input inclination angle 
            /// @param dc the drift circle (expressed as a space point)
            /// @return the uncertainty on the y offset - calculated from an inflated 
            /// drift circle error and a baseline uncertainty to account for the not fully known t0
            double houghWidthMdt(double tanTheta, const MuonR4::HoughHitType & dc, double targetReso); 

            /// @brief Uncertainty parametrisation for strip measurements
            /// @param tanTheta: the input inclination angle (not used) 
            /// @param strip: the strip measurement (expressed as a space point) 
            /// @return the uncertainty on the y offset - based on the strip pitch 
            double houghWidthStrip(double tanTheta, const MuonR4::HoughHitType & strip, double targetReso);
        }
        namespace Phi{
            /// @brief straight line parametrisation for strip detector measurements, in the x-direction 
            /// @param tanPhi the input inclination angle
            /// @param strip the strip measurement (expressed as a space point)
            /// @return the x offset needed to pass through the center of the strip space point for an inclination angle tanPhi
            double houghParamStrip(double tanPhi, const MuonR4::HoughHitType & dc); 

            /// @brief Uncertainty parametrisation for strip measurements
            /// @param tanPhi: the input inclination angle (not used) 
            /// @param strip: the strip measurement (expressed as a space point) 
            /// @return the uncertainty on the x offset - based on the strip pitch 
            double houghWidthStrip(double tanPhi, const MuonR4::HoughHitType & dc, double targetReso); 
        }
    }
}

#endif
