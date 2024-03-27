/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MUONHOUGHHELPERS__H
#define MUONR4__MUONHOUGHHELPERS__H

#include "MuonHoughEventData.h"
#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"

namespace MuonR4{
    namespace HoughHelpers{
        namespace Eta{
            /// @brief left-side hough parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle 
            /// @return the z offset needed to touch the drift radius on the left for an inclination angle tanTheta
            double houghParamMdtLeft(double tanTheta, const MuonR4::HoughHitType & dc); 
            /// @brief right-side hough parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle 
            /// @return the z offset needed to touch the drift radius on the right for an inclination angle tanTheta
            double houghParamMdtRight(double tanTheta, const MuonR4::HoughHitType & dc); 
            /// @brief tube-level hough parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle 
            /// @return the z offset needed to pass through the center of the drift tube for an inclination angle tanTheta
            double houghParamStrip(double tanTheta, const MuonR4::HoughHitType & dc); 
            /// @brief tube-level hough parametrisation for drift circles 
            /// @param tanTheta the input inclination angle
            /// @param dc the drift circle 
            /// @return the z offset needed to pass through the center of the drift tube for an inclination angle tanTheta
            double houghWidthMdt(double tanTheta, const MuonR4::HoughHitType & dc); 
            /// @brief Uncertainty parametrisation when filling the hough plane using only tube information
            /// @param tanTheta: the input inclination angle (not used) 
            /// @param dc: the drift circle 
            /// @return an uncertainty corresponding to the full tube radius
            double houghWidthStrip(double tanTheta, const MuonR4::HoughHitType & dc); 
        }
    }
}

#endif
