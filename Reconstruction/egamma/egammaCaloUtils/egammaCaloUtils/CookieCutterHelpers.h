/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COOKIECUTTERHELPERS_H
#define COOKIECUTTERHELPERS_H

#include "xAODCaloEvent/CaloCluster.h"

#include "GaudiKernel/SystemOfUnits.h"

namespace CookieCutterHelpers
{
/** Find the reference position (eta, phi) relative to which cells are
   restricted.
*/
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

  CentralPosition() = default;
  CentralPosition(const std::vector<const xAOD::CaloCluster*>& clusters);
};

/** Find the size of the cluster in phi using L2 cells.
 *
 * @param cp0: the reference position in calo-coordinates
 * @param cluster: the cluster filled with L2 and L3 cells
 *
 * The window is computed using only cells in the second layer.
 * Asymmetric sizes are computed for barrel and endcap. The size
 * is the maximum difference in phi between the center of a cell
 * and the refence, considering separately cells in the barrel
 * and in the endcap. The computation is done separately for the
 * cells with phi < reference phi or >=. A cutoff value of 1 is used.
 */
struct PhiSize
{
  float plusB = 0;
  float minusB = 0;
  float plusEC = 0;
  float minusEC = 0;

  PhiSize() = default;
  PhiSize(const CentralPosition& cp0, const xAOD::CaloCluster& cluster);
};
}

#endif

