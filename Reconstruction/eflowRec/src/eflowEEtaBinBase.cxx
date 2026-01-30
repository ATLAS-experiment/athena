/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     eflowEEtaBinBase.cxx
PACKAGE:  offline/Reconstruction/eflowRec

AUTHOR:   R Duxfield
CREATED:  17th May, 2006

********************************************************************/

#include <cmath>
#include "eflowRec/eflowEEtaBinBase.h"


////////////////////////////////
//   eflowBaseEEtaBinSystem   //
////////////////////////////////

const double eflowEEtaBinBase::m_errorReturnValue = -999.0;

eflowEEtaBinBase::~eflowEEtaBinBase() = default;


int eflowEEtaBinBase::getEBinIndex(double e) const {
  int nEBins = getNumEBins();
  for (int i = 0; i < (nEBins-1); i++) {
    if (e > m_eBinBounds[i] && e < m_eBinBounds[i + 1]) return i;
  } 
  //for the final bin we simply check if the track energy is greater than the lower bound
  if ( !m_eBinBounds.empty() && e > m_eBinBounds.back() ) return nEBins-1;
  return 0;
}

int eflowEEtaBinBase::getEBinIndexLegacy(double e) const {

  int nEBins = getNumEBins();
  int bin = 0;
  for (int i = nEBins - 1; i > 0; i--) {
    if (e > sqrt(m_eBinBounds[i - 1] * m_eBinBounds[i])) {
      bin = i;
      break;
    }
  }
  return bin;

}

int eflowEEtaBinBase::getEtaBinIndex(double eta) const {
  if (m_useAbsEta) eta = fabs(eta);

  /* If eta is outside bin range, return highest/lowest bin to avoid binning failures
   * (in practice we always use absEta, so it can only be too high) */
  if (eta > m_etaBinBounds.back()) {
    return m_etaBinBounds.size()-2; // Yes, it's minus *two* --> we need to return the index of the *low* edge of the bin
  }
  if (eta < m_etaBinBounds[0]){
    return 0;
  }

  return getBinIndex(eta, m_etaBinBounds);
}

int eflowEEtaBinBase::getBinIndex(double x, const std::vector<double>& binBounds) {
  int nBins = binBounds.size() - 1;
  int bin = -1;
  for (int i = 0; i < nBins; i++) {
    if (x >= binBounds[i] && x < binBounds[i+1]) {
      bin = i;
      break;
    }
  }
  return bin;
}
