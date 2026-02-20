#include "PFClusterWidthCalculator.h"

#include "eflowUtil.h"

#include <cmath>

PFClusterWidthCalculator::PFClusterWidthCalculator() : m_etaPhiLowerLimit(0.0025),m_etaMean(0.0),m_phiMean(0.0) {}



std::pair<double,double> PFClusterWidthCalculator::getPFClusterCoordinateWidth(const std::vector<double>& eta, const std::vector<double>& phi, const double& clusterEta, const double& clusterPhi, unsigned int nCells){

    if (nCells <= 1) {
        m_etaMean = clusterEta;
        m_phiMean = clusterPhi;
        return std::make_pair(m_etaPhiLowerLimit,m_etaPhiLowerLimit);
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

    m_etaMean = etaSum/static_cast<double>(nCells);
    m_phiMean = phiSum/static_cast<double>(nCells);

    double varianceCorrection = (double)nCells / (double)(nCells-1);
    double etaVariance = std::max(m_etaPhiLowerLimit,varianceCorrection * (etaSum2/static_cast<double>(nCells) - m_etaMean*m_etaMean));
    double phiVariance = std::max(m_etaPhiLowerLimit,varianceCorrection * (phiSum2/static_cast<double>(nCells) - m_phiMean*m_phiMean));

    return std::make_pair(etaVariance,phiVariance);
}