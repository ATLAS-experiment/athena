/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFClusterWidthCalculator.h"

#include "eflowUtil.h"

#include <cmath>

PFClusterWidthCalculator::PFClusterWidthCalculator() : m_etaPhiLowerLimit(0.0025) {}


PFClusterWidth PFClusterWidthCalculator::getPFClusterCoordinateWidth(const std::vector<double>& eta, const std::vector<double>& phi, double clusterEta, double clusterPhi, unsigned int nCells) const {
  PFClusterWidth pfClusterWidth;
  if (nCells <= 1) {
    pfClusterWidth.etaMean = clusterEta;
    pfClusterWidth.phiMean = clusterPhi;
    pfClusterWidth.etaVariance = m_etaPhiLowerLimit;
    pfClusterWidth.phiVariance = m_etaPhiLowerLimit;
    return pfClusterWidth;
  }

  double etaSum(0.0);
  double etaSum2(0.0);
  double phiSum(0.0);
  double phiSum2(0.0);

  for(unsigned int iCell=0; iCell<nCells; ++iCell){
    etaSum += eta[iCell];
    etaSum2 += eta[iCell]*eta[iCell];
    double thisCellPhi = eflowAzimuth(phi[iCell]).cycle(clusterPhi);
    phiSum += thisCellPhi;
    phiSum2 += thisCellPhi*thisCellPhi;
  }

  pfClusterWidth.etaMean = etaSum/static_cast<double>(nCells);
  pfClusterWidth.phiMean = phiSum/static_cast<double>(nCells);

  double varianceCorrection = (double)nCells / (double)(nCells-1);
  pfClusterWidth.etaVariance = std::max(m_etaPhiLowerLimit, varianceCorrection * (etaSum2/static_cast<double>(nCells) - pfClusterWidth.etaMean*pfClusterWidth.etaMean));
  pfClusterWidth.phiVariance = std::max(m_etaPhiLowerLimit, varianceCorrection * (phiSum2/static_cast<double>(nCells) - pfClusterWidth.phiMean*pfClusterWidth.phiMean));

  return pfClusterWidth;
}
