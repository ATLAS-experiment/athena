/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_DERCLCANG_H
#define TRKVKALVRTCORE_DERCLCANG_H


namespace Trk {
class VKPhiConstraint;
class VKThetaConstraint;
class VKPlaneConstraint;
class VKRadiusConstraint;
void calcPhiConstraint( VKPhiConstraint * cnst);
void calcThetaConstraint( VKThetaConstraint * cnst);
void calcPlaneConstraint( VKPlaneConstraint * cnst);
void calcRadiusConstraint( VKRadiusConstraint * cnst);
}  // namespace Trk

#endif

