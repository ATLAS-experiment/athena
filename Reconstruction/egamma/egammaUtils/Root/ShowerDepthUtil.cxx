/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "egammaUtils/ShowerDepthUtil.h"

// Project include(s).
#include "AsgMessaging/MessageCheck.h"
#include "PathResolver/PathResolver.h"

// ROOT include(s).
#include <TFile.h>

// System include(s).
#include <cmath>
#include <string>
#include <stdexcept>

namespace {
// Note that std::numeric_limits<float>::epsilon()
// is just not large enough for what we need.  :-(
constexpr float epsilon = 1e-6;
// Calibration file / histogram name(s).
const char* const CONFIG_FILE_NAME =
    "ElectronIsolationSelection/v1/CaloDeltaRZ.root";
const char* const DATA_HISTO_NAME = "hData";
const char* const MC_HISTO_NAME = "hMC";

}  // namespace

namespace CP {

// Make the messaging functions available.
ANA_MSG_SOURCE(ShowerDepthUtilMessaging, "CP::ShowerDepthUtil");
using namespace ShowerDepthUtilMessaging;

ShowerDepthUtil::ShowerDepthUtil() {
  const std::string filename = PathResolverFindCalibFile(CONFIG_FILE_NAME);

  m_hData = getHistoFromFile(filename.c_str(), DATA_HISTO_NAME);
  m_hMC = getHistoFromFile(filename.c_str(), MC_HISTO_NAME);
  if(!m_hData || !m_hMC){
    throw std::runtime_error("Could not create instance of ShowerDepthUtil");
  }
}

/** Shower depth (in mm) on EM1 vs. eta, considering misalignments **/
float ShowerDepthUtil::getCorrectedShowerDepthEM1(float etas1, float phi,
                                                  bool isData) const {
  return getShowerDepthEM1(etas1) - getRZCorrection(etas1, phi, isData);
}

/** Shower depth (in mm) on EM2 vs. eta, considering misalignments **/
float ShowerDepthUtil::getCorrectedShowerDepthEM2(float etas2, float phi,
                                                  bool isData) const {
  return getShowerDepthEM2(etas2) - getRZCorrection(etas2, phi, isData);
}

/** Return the shower depth in sampling 1.
 * Found by finding the depth where the eta resolution is optimal.
 * See
 * Calorimeter/CaloDetDescr/src/CaloDepthTool.cxx
 **/
float ShowerDepthUtil::getShowerDepthEM1(float etas1) {
  float radius, aetas1 = std::abs(etas1);
  if (aetas1 < 0.8) {
    radius = (1558.859292 - 4.990838 * aetas1 - 21.144279 * aetas1 * aetas1);
  } else if (aetas1 < 1.5) {
    radius = (1522.775373 + 27.970192 * aetas1 - 21.104108 * aetas1 * aetas1);
  } else {
    // endcap so Z
    radius = 3790.671754;
  }
  if (etas1 < 0. and aetas1 > 1.5) {
    // negative endcap
    return -radius;
  }
  return radius;
}

/** Return the shower depth in sampling 1.
 * Found by finding the depth where the eta resolution is optimal.
 * See
 * Calorimeter/CaloDetDescr/src/CaloDepthTool.cxx
 **/
float ShowerDepthUtil::getShowerDepthEM2(float etas2) {
  float radius, aetas2 = std::abs(etas2);
  if (aetas2 < 1.425) {  // Barrel
    radius = (1698.990944 - 49.431767 * aetas2 - 24.504976 * aetas2 * aetas2);
  } else if (aetas2 < 1.5) {  // EME2 in tool
    radius = (8027.574119 - 2717.653528 * aetas2);
  } else {
    // endcap so Z
    radius = (3473.473909 + 453.941515 * aetas2 - 119.101945 * aetas2 * aetas2);
  }
  if (etas2 < 0. and aetas2 > 1.5) {
    // negative endcap
    return -radius;
  }
  return radius;
}

float ShowerDepthUtil::getCorrectedEtaDirection(float zvertex, float eta,
                                                float phi, bool isData,
                                                int sampling) const {
  std::pair<float, float> RZ = getCorrectedRZ(eta, phi, isData, sampling);
  return getEtaDirection(zvertex, RZ.first, RZ.second);
}

std::pair<float, float> ShowerDepthUtil::getRZ(float eta, int sampling) {
  if ((sampling != 1 && sampling != 2) || (std::abs(eta) > 10)) {
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Invalid sampling, eta: " << sampling << ", " << eta);
    return std::make_pair(0., 0.);
  }
  float depth =
      (sampling == 1 ? getShowerDepthEM1(eta) : getShowerDepthEM2(eta));
  if (std::abs(eta) < 1.5)
    return std::make_pair(depth, depth * std::sinh(eta));
  return std::make_pair(depth / std::sinh(eta), depth);
}

std::optional<float> ShowerDepthUtil::getCaloPointingEta(float etas1,
                                                         float etas2, float phi,
                                                         bool isData) const {
  std::pair<float, float> RZ1 = getCorrectedRZ(etas1, phi, isData, 1);
  std::pair<float, float> RZ2 = getCorrectedRZ(etas2, phi, isData, 2);
  // Sanity check
  if (std::abs(RZ2.first - RZ1.first) < epsilon) {
    return std::nullopt;
  }

  return {std::asinh((RZ2.second - RZ1.second) / (RZ2.first - RZ1.first))};
}

std::pair<float, float> ShowerDepthUtil::getCorrectedRZ(float eta, float phi,
                                                        bool isData,
                                                        int sampling) const {
  if ((sampling != 1 && sampling != 2) || (std::abs(eta) > 10)) {
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Invalid sampling, eta: " << sampling << ", " << eta);
    return std::make_pair(0., 0.);
  }
  float depth = (sampling == 1 ? getCorrectedShowerDepthEM1(eta, phi, isData)
                               : getCorrectedShowerDepthEM2(eta, phi, isData));
  if (std::abs(eta) < 1.5)
    return std::make_pair(depth, depth * std::sinh(eta));
  return std::make_pair(depth / std::sinh(eta), depth);
}

/** Return the calorimeter displacement in R(Z) for barrel (endcap) **/
float ShowerDepthUtil::getRZCorrection(float eta, float phi,
                                       bool isData) const {
  // Get the correct histogram.
  const TH1* histo = (isData ? m_hData.get() : m_hMC.get());
  if (!histo) {
    return 0;
  }
  // Make sure that we can perform the interpolation in both eta and phi.
  const Int_t etaBin = histo->GetXaxis()->FindFixBin(eta);
  if (etaBin < 1) {
    const float etaOld = eta;
    eta = histo->GetXaxis()->GetBinLowEdge(1) + epsilon;
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Using eta " << eta << " instead of " << etaOld);
  } else if (etaBin > histo->GetNbinsX()) {
    const float etaOld = eta;
    eta = histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX()) - epsilon;
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Using eta " << eta << " instead of " << etaOld);
  }
  const Int_t phiBin = histo->GetYaxis()->FindFixBin(phi);
  if (phiBin < 1) {
    const float phiOld = phi;
    phi = histo->GetYaxis()->GetBinLowEdge(1) + epsilon;
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Using phi " << phi << " instead of " << phiOld);
  } else if (phiBin > histo->GetNbinsY()) {
    const float phiOld = phi;
    phi = histo->GetYaxis()->GetBinUpEdge(histo->GetNbinsY()) - epsilon;
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Using phi " << phi << " instead of " << phiOld);
  }
  // Get the correction as an interpolation.
  return histo->Interpolate(eta, phi);
}

float ShowerDepthUtil::getEtaDirection(float zvertex, float R, float z) {
  return std::asinh((z - zvertex) / R);
}

std::unique_ptr<TH1> ShowerDepthUtil::getHistoFromFile(const char* fileName,
                                                       const char* histoName) {
  std::unique_ptr<TFile> f(TFile::Open(fileName, "READ"));
  if (!f) {
    ANA_MSG_LVL_SERIOUS(MSG::WARNING,
                        "Could not open file: \"" << fileName << "\"");
    return {};
  }
  TH1* h = dynamic_cast<TH1*>(f->Get(histoName));
  if (!h) {
    ANA_MSG_LVL_SERIOUS(MSG::WARNING, "Could not get histogram: \""
                                          << histoName << "\" from file: \""
                                          << fileName << "\"");
    return {};
  }
  // The file will be deleted. We take ownership.
  h->SetDirectory(nullptr);
  return std::unique_ptr<TH1>(h);
}

}  // namespace CP
