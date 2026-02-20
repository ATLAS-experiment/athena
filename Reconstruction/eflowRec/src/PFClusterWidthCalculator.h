#ifndef PFCLUSTERWIDTHCALCULATOR_H
#define PFCLUSTERWIDTHCALCULATOR_H

#include <vector>

class PFClusterWidthCalculator {

    public:
    
    PFClusterWidthCalculator();
    ~PFClusterWidthCalculator() = default;
    
    std::pair<double,double> getPFClusterCoordinateWidth(const std::vector<double>& eta, const std::vector<double>& phi,const double& clusterEta, const double& clusterPhi, unsigned int nCells);
    double getEtaMean() const {return m_etaMean;}
    double getPhiMean() const {return m_phiMean;}


    private:
    double m_etaPhiLowerLimit;
    double m_etaMean;
    double m_phiMean;

    
};

#endif