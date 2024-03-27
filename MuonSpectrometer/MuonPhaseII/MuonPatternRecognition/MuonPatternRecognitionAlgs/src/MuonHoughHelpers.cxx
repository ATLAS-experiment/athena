/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughHelpers.h"
using namespace MuonR4; 

double HoughHelpers::Eta::houghParamMdtLeft(double tanTheta, const MuonR4::HoughHitType & DC){
    return DC->positionInChamber().y() - tanTheta * DC->positionInChamber().z() -
           DC->driftRadius() * std::sqrt(1 + (tanTheta*tanTheta));    // using cos(theta) = sqrt(1/[1+tan²(theta)])
  };
  // right solution
double HoughHelpers::Eta::houghParamMdtRight(double tanTheta, const MuonR4::HoughHitType & DC){
  return DC->positionInChamber().y() - tanTheta * DC->positionInChamber().z() +
           DC->driftRadius() * std::sqrt(1 + (tanTheta*tanTheta));    // using cos(theta) = sqrt(1/[1+tan²(theta)])
  };
  // solution which doesn't look at the drift circle but places a hit at the tube center
double HoughHelpers::Eta::houghParamStrip(double tanTheta, const MuonR4::HoughHitType & DC){
    return DC->positionInChamber().y() - tanTheta * DC->positionInChamber().z();
  };

double HoughHelpers::Eta::houghWidthMdt(double /*tanTheta*/, const MuonR4::HoughHitType & DC){
    return std::min(DC->uncertainty().x() * 3.,
                    1.0);  // scale reported errors up to at least 1mm or 3
                           // times the reported error as drift circle calib not
                           // fully reliable at this stage
  };
double HoughHelpers::Eta::houghWidthStrip(double /*tanTheta*/, const MuonR4::HoughHitType & DC){
    return DC->uncertainty().x();  // assign full tube radius as uncertainty
  };

