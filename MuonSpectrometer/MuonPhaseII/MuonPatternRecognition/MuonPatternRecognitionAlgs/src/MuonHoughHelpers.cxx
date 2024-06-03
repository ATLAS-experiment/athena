/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughHelpers.h"

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

  double HoughHelpers::Eta::houghWidthMdt(double /*tanTheta*/, const MuonR4::HoughHitType & DC){
    return std::min(DC->uncertainty().x() * 3.,
                    1.0);  // scale reported errors up to at least 1mm or 3
                           // times the reported error as drift circle calib not
                           // fully reliable at this stage
  }
  double HoughHelpers::Eta::houghWidthStrip(double /*tanTheta*/, const MuonR4::HoughHitType & strip){
      return strip->uncertainty().y();  // return positional uncertainty defined during SP creation
  }


  double HoughHelpers::Phi::houghParamStrip(double tanPhi, const MuonR4::HoughHitType & strip){
    return strip->positionInChamber().x() - tanPhi * strip->positionInChamber().z();
  }
  double HoughHelpers::Phi::houghWidthStrip(double /*tanPhi*/, const MuonR4::HoughHitType & DC){
      return DC->uncertainty().x();  // return positional uncertainty defined during SP creation
  }
