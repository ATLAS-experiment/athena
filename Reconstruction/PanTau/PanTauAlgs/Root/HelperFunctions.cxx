/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/HelperFunctions.h"
#include "PanTauAlgs/TauConstituent.h"

std::string PanTau::HelperFunctions::convertNumberToString(double x) const {
  std::stringstream tmpStream;
  tmpStream << x;
  return tmpStream.str();
}


int PanTau::HelperFunctions::getBinIndex(const std::vector<double>& binEdges, double value) const {
  int resBin = -1;
  for(unsigned int i=0; i<binEdges.size()-1; i++) {
    double lowerEdge = binEdges[i];
    double upperEdge = binEdges[i+1];
    if(lowerEdge <= value && value < upperEdge) resBin = i;
  }
  if(resBin == -1) {
    ATH_MSG_WARNING("Could not find matching bin for value " << value << " in these bin edges:");
    for(unsigned int i=0; i<binEdges.size(); i++) ATH_MSG_WARNING("\tbin edge " << i << ": " << binEdges[i]);
  }
  return resBin;
}


double PanTau::HelperFunctions::stddev(double sumOfSquares, double sumOfValues, int numConsts) const {
  // calculate standard deviations according to:
  // sigma^2 = (sum_i x_i^2) / N - ((sum_i x_i)/N)^2 (biased maximum-likelihood estimate)
  // directly set sigma^2 to 0 in case of N=1, otherwise numerical effects may yield very small negative sigma^2
  if(numConsts == 1) return 0;
  double a = sumOfSquares / (static_cast<double>(numConsts));
  double b = sumOfValues / (static_cast<double>(numConsts));
  double stdDev = a - b*b;
  if(stdDev < 0.) stdDev = 0;
  return std::sqrt(stdDev);
}

