/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./DipzLikelihoodCmp.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <cmath>
#include <limits>
#include <numeric>

DipzLikelihood::DipzLikelihood(const std::string &decName_z,
			       const std::string &decName_sigma,
			       bool sigmaIsStdDev):
  m_decName_z(decName_z),
  m_decName_sigma(decName_sigma),
  m_sigmaIsStdDev(sigmaIsStdDev){
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

DipzLikelihood::Width DipzLikelihood::getWidth(const pHypoJet &ip) const {

  const double dec = getDipzMLPLDecValue(ip, m_decName_sigma);

  if (!m_sigmaIsStdDev) {
    // decorated as the log precision -2*log(sigma)
    return {std::exp(-1 * dec), dec};
  }

  // decorated as the standard deviation itself
  if (!(std::isfinite(dec) && dec > 0.)) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    return {nan, nan};
  }
  return {dec * dec, -2. * std::log(dec)};
}

double DipzLikelihood::calcNum(double acml, const pHypoJet &ip) const {
  
  double sigma_squared = getWidth(ip).sigma2;
  
  double muoversigmasq =
    checkedRatio( getDipzMLPLDecValue(ip, m_decName_z), sigma_squared);
  
  return acml + muoversigmasq;
}


double DipzLikelihood::calcDenom(double acml, const pHypoJet &ip) const { 
  double sigma_squared = getWidth(ip).sigma2;

  double oneoversigmasq = checkedRatio(1, sigma_squared);
  
  return acml + oneoversigmasq;
}

double DipzLikelihood::calcLogTerm(double acml,
				   const pHypoJet &ip,
				   double zhat) const {

  double dipz_mu = getDipzMLPLDecValue(ip, m_decName_z);

  const Width w = getWidth(ip);
  
  double logterm =
    -0.5 * std::log(2.0 * M_PI)
    + 0.5 * w.negLogSigma2
    - checkedRatio(std::pow(zhat - dipz_mu, 2), (2.0 * w.sigma2) );
  
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
				     const std::string &decName_sigma,
				     bool sigmaIsStdDev):
  m_likelihoodCalculator(decName_z, decName_sigma, sigmaIsStdDev) {
}

bool DipzLikelihoodCmp::operator()(const HypoJetVector& l,
				   const HypoJetVector& r) {
  return m_likelihoodCalculator(l) <  m_likelihoodCalculator(r);
}


