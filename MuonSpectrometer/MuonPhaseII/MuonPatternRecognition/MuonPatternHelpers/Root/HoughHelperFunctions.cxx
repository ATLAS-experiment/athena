/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternHelpers/HoughHelperFunctions.h"

using namespace MuonR4; 

  double HoughHelpers::Eta::houghParamMdtLeft(double tanTheta, const MuonR4::HoughHitType & DC){
    return DC->positionInChamber().y() - tanTheta * DC->positionInChamber().z() -
           DC->driftRadius() * std::sqrt(1 + (tanTheta*tanTheta));    // using cos(theta) = sqrt(1/[1+tan²(theta)])
  }
  double HoughHelpers::Eta::houghParamMdtRight(double tanTheta, const MuonR4::HoughHitType & DC){
    return DC->positionInChamber().y() - tanTheta * DC->positionInChamber().z() +
           DC->driftRadius() * std::sqrt(1 + (tanTheta*tanTheta));    // using cos(theta) = sqrt(1/[1+tan²(theta)])
  }
  double HoughHelpers::Eta::houghParamStrip(double tanTheta, const MuonR4::HoughHitType & strip){
    return strip->positionInChamber().y() - tanTheta * strip->positionInChamber().z();
  }

  double HoughHelpers::Eta::houghWidthMdt(double /*tanTheta*/, const MuonR4::HoughHitType & DC, double targetReso){
    return std::max(DC->uncertainty().y() * 3.,
                    targetReso);  // scale reported errors up to at least 1mm or 3
                           // times the reported error as drift circle calib not
                           // fully reliable at this stage
  }
  double HoughHelpers::Eta::houghWidthStrip(double /*tanTheta*/, const MuonR4::HoughHitType & strip, double targetReso){
      return std::max(targetReso, 3 * strip->uncertainty().y() * ((strip->primaryMeasurement()->type() == xAOD::UncalibMeasType::TgcStripType && !strip->measuresPhi()) ? 1.5 : 1.0));  // return positional uncertainty defined during SP creation
  }

  double HoughHelpers::Phi::houghParamStrip(double tanPhi, const MuonR4::HoughHitType & strip){
    return strip->positionInChamber().x() - tanPhi * strip->positionInChamber().z();
  }
  double HoughHelpers::Phi::houghWidthStrip(double /*tanPhi*/, const MuonR4::HoughHitType & DC){
      return 3 * DC->uncertainty().x();  // return positional uncertainty defined during SP creation
  }
