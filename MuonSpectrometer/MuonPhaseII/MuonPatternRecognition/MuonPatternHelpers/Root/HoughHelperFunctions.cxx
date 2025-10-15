/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "Acts/Utilities/MathHelpers.hpp"

namespace MuonR4::HoughHelpers {
    using namespace SegmentFit;
    namespace Eta{
        double houghParamMdtLeft(double tanBeta, const MuonR4::HoughHitType & DC){
            return DC->localPosition().y() - tanBeta * DC->localPosition().z() -
                    DC->driftRadius() * Acts::fastHypot(1., tanBeta);    // using cos(theta) = sqrt(1/[1+tan²(theta)])
        }
        double houghParamMdtRight(double tanBeta, const MuonR4::HoughHitType & DC){
            return DC->localPosition().y() - tanBeta * DC->localPosition().z() +
                   DC->driftRadius() * Acts::fastHypot(1., tanBeta);    // using cos(theta) = sqrt(1/[1+tan²(theta)])
        }
        double houghParamStrip(double tanBeta, const MuonR4::HoughHitType & strip){
            return strip->localPosition().y() - tanBeta * strip->localPosition().z();
        }
        double houghWidthMdt(double /*tanBeta*/, const MuonR4::HoughHitType & DC, double targetReso){
            /** Scale reported errors up to at least 1 mm or 3 times the reported error as drift
             *  circle calibration is not fully reliable at this stage */
            return std::max(3.* std::sqrt(DC->covariance()[Acts::toUnderlying(AxisDefs::etaCov)]),
                            targetReso);  // scale reported errors up to at least 1mm or 3
        }
        double houghWidthStrip(double /*tanBeta*/, const MuonR4::HoughHitType& strip, double targetReso) {
            const double nomCov = 3.*std::sqrt(strip->covariance()[Acts::toUnderlying(AxisDefs::etaCov)]);
            return std::max(targetReso, nomCov* ((strip->type() == xAOD::UncalibMeasType::TgcStripType && !strip->measuresPhi()) ? 1.5 : 1.0));  
            // return positional uncertainty defined during SP creation
        }
    }
    namespace Phi {
        double houghParamStrip(double tanAlpha, const MuonR4::HoughHitType & strip){
            return strip->localPosition().x() - tanAlpha * strip->localPosition().z();
        }
        double houghWidthStrip(double /*tanAlpha*/, const MuonR4::HoughHitType & strip, double targetReso){
            const double nomCov = 3.*std::sqrt(strip->covariance()[Acts::toUnderlying(AxisDefs::phiCov)]);
            return std::max(targetReso, 3 * nomCov);  // return positional uncertainty defined during SP creation
        }
    }
}