/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFCLUSTERWIDTHCALCULATOR_H
#define EFLOWREC_PFCLUSTERWIDTHCALCULATOR_H

#include <vector>


struct PFClusterWidth {
  double etaMean{};
  double phiMean{};
  double etaVariance{};
  double phiVariance{};
};

class PFClusterWidthCalculator {

public:
    
    PFClusterWidthCalculator();
    ~PFClusterWidthCalculator() = default;

  PFClusterWidth getPFClusterCoordinateWidth(const std::vector<double>& eta, const std::vector<double>& phi, double clusterEta, double clusterPhi, unsigned int nCells) const;

private:
  double m_etaPhiLowerLimit{};

};

#endif
