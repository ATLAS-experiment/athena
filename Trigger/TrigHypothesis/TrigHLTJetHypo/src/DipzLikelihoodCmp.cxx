/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./DipzLikelihoodCmp.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <cmath>
#include <numeric>

DipzLikelihood::DipzLikelihood(const std::string &decName_z,
			       const std::string &decName_negLogSigma2):
  m_decName_z(decName_z),
  m_decName_negLogSigma2(decName_negLogSigma2){
}

double DipzLikelihood::checkedRatio(double num, double den) const {
  if (den == 0.) {
    // dividing x/0  is picked up by FPEAditor. C++ simply returns
    // +inf or -inf if x != 0, or a nan otherwise.
    throw std::runtime_error("DipzLikelihood::checkedRatio dividing by 0");
  }
  
  return num/den;
}

double DipzLikelihood::getDipzMLPLDecValue(const pHypoJet &ip,
					   const std::string &decName) const
{
  
  float momentValue;
  if (!(ip->getAttribute(decName, momentValue))) {
    throw std::runtime_error("Impossible to retrieve decorator \'" +
			     decName + "\' for jet hypo");
  }
  
  // momentValue is retrieved as a float, but will be heavily used in
  // further calculations. Convert to a double here
  return momentValue;
}

double DipzLikelihood::calcNum(double acml, const pHypoJet &ip) const {
  
  double sigma_squared =
    std::exp(-1 * getDipzMLPLDecValue(ip, m_decName_negLogSigma2));
  
  double muoversigmasq =
    checkedRatio( getDipzMLPLDecValue(ip, m_decName_z), sigma_squared);
  
  return acml + muoversigmasq;
}


double DipzLikelihood::calcDenom(double acml, const pHypoJet &ip) const { 
  double sigma_squared =
    std::exp(-1 * getDipzMLPLDecValue(ip, m_decName_negLogSigma2));

  double oneoversigmasq = checkedRatio(1, sigma_squared);
  
  return acml + oneoversigmasq;
}

double DipzLikelihood::calcLogTerm(double acml,
				   const pHypoJet &ip,
				   double zhat) const {

  double dipz_mu = getDipzMLPLDecValue(ip, m_decName_z);

  double dipz_negLogSigmaSq =
    getDipzMLPLDecValue(ip, m_decName_negLogSigma2);

  double sigma_squared = std::exp(-1 * dipz_negLogSigmaSq); 
  
  double logterm =
    -0.5 * std::log(2.0 * M_PI)
    + 0.5 * dipz_negLogSigmaSq
    - checkedRatio(std::pow(zhat - dipz_mu, 2), (2.0 * sigma_squared) );
  
  return acml + logterm;

}


double DipzLikelihood::operator()(const HypoJetVector& ips) const {
  
  auto zhat_num = std::accumulate(ips.begin(),
			      ips.end(),
			      0.0,
			      [this](double sum, const pHypoJet& jp) {
				return this->calcNum(sum, jp);});
  
  auto zhat_den = std::accumulate(ips.begin(),
				  ips.end(),
				  0.0,
				  [this](double sum, const pHypoJet& jp) {
				    return this->calcDenom(sum, jp);});
  
  auto zhat = checkedRatio(zhat_num, zhat_den);
  
  auto logproduct =
    std::accumulate(ips.begin(), 
		    ips.end(), 
		    0.0, 
		    [&zhat,this](double sum, const pHypoJet& jp) {
		      return this->calcLogTerm(sum, jp, zhat);});
  
  return logproduct;
}


DipzLikelihoodCmp::DipzLikelihoodCmp(const std::string &decName_z,
				     const std::string &decName_negLogSigma2):
  m_likelihoodCalculator(decName_z, decName_negLogSigma2) {
}

bool DipzLikelihoodCmp::operator()(const HypoJetVector& l,
				   const HypoJetVector& r) {
  return m_likelihoodCalculator(l) <  m_likelihoodCalculator(r);
}


