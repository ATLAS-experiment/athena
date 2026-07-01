/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     eflowEEtaBinnedParameters.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  M.Hodgkinson, R Duxfield (based on R.Duxfields Root package)
CREATED:  18th Aug, 2005

********************************************************************/

//Athena Headers
#include "eflowEEtaBinnedParameters.h"
#include "eflowFirstIntParameters.h"
#include "eflowRingSubtractionManager.h"

#include "GaudiKernel/SystemOfUnits.h"

//C++ Headers
#include <float.h>
#include <cmath>
#include <stdexcept>

using namespace std;
///////////////////////////////////
//   eflowEEtaBinnedParameters   //
///////////////////////////////////


void eflowEEtaBinnedParameters::initialise(const std::vector<double>& eBinBounds, const std::vector<double>& etaBinBounds, bool useAbsEta) {
  m_useAbsEta = useAbsEta;
  m_eBinBounds = eBinBounds;
  m_etaBinBounds = etaBinBounds;

  /* Create all the bins */
  int nEBins = getNumEBins();
  int nEtaBins = getNumEtaBins()-1;
  m_bins.reserve(nEBins);
  for (int iEBin = 0; iEBin < nEBins; iEBin++) {
    std::vector<std::unique_ptr<eflowParameters> > tempVector;
    tempVector.reserve(nEtaBins);
    for (int iEtaBin = 0; iEtaBin < nEtaBins; iEtaBin++) {
      tempVector.push_back(std::make_unique<eflowParameters>());
    }
    m_bins.push_back(std::move(tempVector));
  }
}

 double eflowEEtaBinnedParameters::getInterpolation(const eflowParameters** bin1,
                                                          const eflowParameters** bin2,
                                                          double e, double eta, bool useLegacyEnergyBinIndexing) const {
  double weight;
  int eBin = 0;
  if (useLegacyEnergyBinIndexing) {
    eBin = getEBinIndexLegacy(e);
  } else {
    eBin = getEBinIndex(e);
  }
  int etaBin = getEtaBinIndex(eta);

  /* Check for invalid bins */
  if (eBin < 0 || etaBin < 0) {
    *bin1 = nullptr;
    *bin2 = nullptr;
    weight = -1.0;
    return weight;
  }

  if ((0 == eBin && e <= m_eBinBounds[eBin]) ||
      (eBin == getNumEBins() - 1 && e >= m_eBinBounds[getNumEBins() - 1])) {
    /* If e is below the lowest (above the highest) pinpoint, just return the lowest (highest) bin; no interpolation in this case. */
    *bin1 = m_bins[eBin][etaBin].get();
    *bin2 = m_bins[eBin][etaBin].get();
    weight = 1.0;
  } else {
    /* The "normal" case: interpolate between two energies */
    int lowEBin = eBin;
    int highEBin = eBin;

    if (e >= m_eBinBounds[eBin]) {
      highEBin = eBin + 1;
    } else {
      lowEBin = eBin - 1;
    }
    auto outOfRange = [this](int bin)->bool{
      return (bin<0) or (bin>=std::ssize(m_bins));
    };
    
    if (outOfRange(lowEBin) or outOfRange(highEBin)){
      throw std::out_of_range("Attempt to access element out of range in eflowEEtaBinnedParameters::getInterpolation");
    }
    *bin1 = m_bins[lowEBin][etaBin].get();
    *bin2 = m_bins[highEBin][etaBin].get();

    double higherEBound = m_eBinBounds[highEBin];
    double lowerEBound = m_eBinBounds[lowEBin];
    //deals with e = 0 bin to avoid dividing by zero
    if (lowerEBound < FLT_MIN) lowerEBound = FLT_MIN;

    weight = std::log(higherEBound / e) / std::log(higherEBound / lowerEBound);
  }
  return weight;
}

eflowFirstIntENUM 
eflowEEtaBinnedParameters::adjustLFI(double e, double eta,
				     eflowFirstIntENUM j1st, bool useLegacyEnergyBinIndexing) const {


  int eBinIndex = 0;               
  if (useLegacyEnergyBinIndexing) {
    eBinIndex = getEBinIndexLegacy(e);
  } else {
    eBinIndex = getEBinIndex(e);
  }

  //hard code this check - needs to be rechecked if ever remake e/p tables
  //if e.g track extrapolation slightly off can have inconsistent j1st and eta (e.g eta = 1.52, j1st = EMB2) and hence fudgeMean is zero
  if (0 <= eBinIndex && eBinIndex <= 5) {
    if (std::fabs(eta) < 1.5) {
      if (eflowFirstIntRegions::EME1 == j1st) j1st = eflowFirstIntRegions::EMB1;
      if (eflowFirstIntRegions::EME2 == j1st) j1st = eflowFirstIntRegions::EMB2;
      if (eflowFirstIntRegions::EME3 == j1st) j1st = eflowFirstIntRegions::EMB3;
      if (eflowFirstIntRegions::HEC == j1st) j1st = eflowFirstIntRegions::Tile;
    }
    if (std::fabs(eta) >= 1.6) {
      if (eflowFirstIntRegions::EMB1 == j1st) j1st = eflowFirstIntRegions::EME1;
      if (eflowFirstIntRegions::EMB2 == j1st) j1st = eflowFirstIntRegions::EME2;
      if (eflowFirstIntRegions::EMB3 == j1st) j1st = eflowFirstIntRegions::EME3;
      if (eflowFirstIntRegions::Tile == j1st) j1st = eflowFirstIntRegions::HEC;
    }

  }

  if (0 == eBinIndex || 1 == eBinIndex) {
    if (std::fabs(eta) >= 1.0 && std::fabs(eta) < 1.1) {
      if(eflowFirstIntRegions::Tile == j1st) j1st = eflowFirstIntRegions::EMB3;
    }
    else if (std::fabs(eta) >= 1.2 && std::fabs(eta) < 1.3) {
      if(eflowFirstIntRegions::Tile == j1st) j1st = eflowFirstIntRegions::EMB3;
    }
    else if (std::fabs(eta) >= 1.5 && std::fabs(eta) < 1.6) {
      if(eflowFirstIntRegions::EMB3 == j1st) j1st = eflowFirstIntRegions::EME3;
    }

  } else if (2 == eBinIndex) {
    if (std::fabs(eta) >= 1.5 && std::fabs(eta) < 1.6) {
      if(eflowFirstIntRegions::EMB2 == j1st) j1st = eflowFirstIntRegions::EME2;
    }

  } else if (3 == eBinIndex) {
    if (std::fabs(eta) >= 1.5 && fabs(eta) < 1.6) {
      if (eflowFirstIntRegions::EMB2 == j1st) j1st = eflowFirstIntRegions::EME2;
      if (eflowFirstIntRegions::EMB1 == j1st) j1st = eflowFirstIntRegions::EME1;
    }

  } else if (4 == eBinIndex || 5 == eBinIndex) {
    if (std::fabs(eta) >= 1.5 && std::fabs(eta) < 1.6) {
      if(eflowFirstIntRegions::EMB1 == j1st) j1st = eflowFirstIntRegions::EME1;
    }
  }

  return j1st;
}
