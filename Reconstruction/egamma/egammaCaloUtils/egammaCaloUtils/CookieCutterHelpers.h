/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COOKIECUTTERHELPERS_H
#define COOKIECUTTERHELPERS_H

#include "xAODCaloEvent/CaloCluster.h"

#include "GaudiKernel/SystemOfUnits.h"

namespace CookieCutterHelpers
{
struct CentralPosition
{
  float etaB = 999;
  float phiB = 999;
  float emaxB = -999 * Gaudi::Units::GeV;
  float etaEC = 999;
  float phiEC = 999;
  float emaxEC = -999 * Gaudi::Units::GeV;
  float etaF = 999;
  float phiF = 999;
  float emaxF = -999 * Gaudi::Units::GeV;
};

struct PhiSize
{
  float plusB = 0;
  float minusB = 0;
  float plusEC = 0;
  float minusEC = 0;
};

/** Find the reference position (eta, phi) relative to which cells are
   restricted.
*/
CentralPosition
findCentralPositionEM2(const std::vector<const xAOD::CaloCluster*>& clusters);

PhiSize
findPhiSize(const CentralPosition& cp0, const xAOD::CaloCluster& cluster);
}

#endif

