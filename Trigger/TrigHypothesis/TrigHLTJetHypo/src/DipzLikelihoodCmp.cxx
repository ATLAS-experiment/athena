/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./DipzLikelihoodCmp.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <cmath>
#include <limits>
#include <vector>

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

double DipzLikelihood::operator()(const HypoJetVector& ips) const {

  // decorations are read once per jet: string-keyed lookups dominate the hypo
  std::vector<double> mu;
  std::vector<Width> w;
  mu.reserve(ips.size());
  w.reserve(ips.size());
  for (const auto& jp : ips) {
    w.push_back(getWidth(jp));
    // a NaN likelihood would raise FPE INVALID in the cut comparison
    if (std::isnan(w.back().sigma2)) {
      return -std::numeric_limits<double>::infinity();
    }
    mu.push_back(getDipzMLPLDecValue(jp, m_decName_z));
  }

  double zhat_num{0.};
  double zhat_den{0.};
  for (std::size_t i = 0; i != mu.size(); ++i) {
    zhat_num += checkedRatio(mu[i], w[i].sigma2);
    zhat_den += checkedRatio(1, w[i].sigma2);
  }

  const double zhat = checkedRatio(zhat_num, zhat_den);

  double logproduct{0.};
  for (std::size_t i = 0; i != mu.size(); ++i) {
    logproduct +=
      -0.5 * std::log(2.0 * M_PI)
      + 0.5 * w[i].negLogSigma2
      - checkedRatio(std::pow(zhat - mu[i], 2), (2.0 * w[i].sigma2));
  }

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


