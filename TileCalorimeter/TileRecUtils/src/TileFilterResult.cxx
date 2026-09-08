/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileFilterResult.cxx
// Authors  : F. Merritt, A. Aurisano
// Created  : March 2004
//
//****************************************************************************
#include "TileRecUtils/TileFilterResult.h"
#include <algorithm>
#include <format>
#include <iostream>
#include <iomanip>

// Constructor
TileFilterResult::TileFilterResult(std::vector<float> &dig, double sig) {
  m_debug = false;
  int ND = dig.size();
  CLHEP::HepVector digtem(ND);
  m_digits = digtem;
  for (int id = 0; id < ND; id++) {
    m_digits[id] = dig[id];
  }
  //int nrow = digits.num_row();
  //int nrowtem = digtem.num_row();
  m_sigDigit = sig;
  // Initialize some of the variables that will be set later.
  m_nParam = 1;
  m_nPileup = 0;
  m_vCross.clear();
  m_chi2 = 999.;
  m_iFitIndex = -1;
}
//============================================================================= 
TileFilterResult::~TileFilterResult() {
  return;
}
//============================================================================= 
double TileFilterResult::getSigDig() const {
  return m_sigDigit;
}

//============================================================================= 
CLHEP::HepVector& TileFilterResult::getDigRef() {
  return m_digits;
}
//============================================================================= 
std::vector<int>& TileFilterResult::getVcrossRef() {
  return m_vCross;
}
//============================================================================= 
int& TileFilterResult::getFitIndexRef() {
  return m_iFitIndex;
}
//============================================================================= 
int& TileFilterResult::getNparRef() {
  return m_nParam;
}
//============================================================================= 
CLHEP::HepVector& TileFilterResult::getParamRef() {
  return m_fitParam;
}
//============================================================================= 
CLHEP::HepVector& TileFilterResult::getErrRef() {
  return m_fitErr;
}
//============================================================================= 
CLHEP::HepVector& TileFilterResult::getResidRef() {
  return m_residuals;
}
//============================================================================= 
double& TileFilterResult::getChi2Ref() {
  return m_chi2;
}
//============================================================================= 
void TileFilterResult::printFitParam() {
  std::cout << std::format(" Print fitted param from TileFilterResult:  Nparam={}, chisq={}\n", m_nParam, m_chi2);
  for (int ipar = 0; ipar < m_nParam; ipar++) {
    if (ipar == 0) {
      std::cout << std::format(" i={}, kcr=P, A={:>5.2g} +-{}\n", ipar, m_fitParam[ipar], m_fitErr[ipar]);
    } else {
      std::cout << std::format(" i={}, kcr={}, A={} +-{}\n", ipar, m_vCross[ipar - 1], m_fitParam[ipar], m_fitErr[ipar]);
    }
  }
  return;
}
//============================================================================= 
double TileFilterResult::getInTime(double &amp, double &err, double &ped, double &chi2, double &t) {
  amp = m_fitParam[1];
  err = m_fitErr[1];
  ped = m_fitParam[0];
  chi2 = m_chi2;
  t = 0.;

  return m_chi2;
}
//============================================================================= 
void TileFilterResult::snapShot(int imode) {
  // This print a short snapshot of the FilterResult state.
  std::cout << std::format(" SnapShot: imode={}.  Nparam={}, chisq={}, iFitIndex{}, Vcross=",
                           imode,
                           m_nParam,
                           m_chi2,
                           m_iFitIndex);
  int Namp = m_nParam - 1;
  for (int jamp = 0; jamp < Namp; jamp++) {
    std::cout << std::format(" {}", m_vCross[jamp]);
  }
  std::cout << std::endl;
  if (m_iFitIndex < 0) return;

  if (imode > 0) {
    std::cout << "   FitParam=";
    for (int ipar = 0; ipar < m_nParam; ipar++) {
      std::cout << std::format("{:>5.1g}+-{}", m_fitParam[ipar], m_fitErr[ipar]);
      if (ipar < m_nParam - 1) std::cout << ", ";
    }
    std::cout << std::endl;
  }
  if (imode > 1) {
    int Ndig = m_digits.num_row();
    std::cout << "   Residuals=";
    for (int idig = 0; idig < Ndig; idig++) {
      std::cout << std::format(" {:>4.3g}", m_residuals[idig]);
    }
    std::cout << std::endl;
  }
}
//============================================================================= 
int TileFilterResult::addCross(int kcrIndex) {

  int iret = -1;

  //Check that kcrIndex is not already in list.
  bool ldup = false;
  if (m_nParam > 1) {
    int Namp = m_nParam - 1;
    for (int icr = 0; icr < Namp; icr++) {
      if (kcrIndex == m_vCross[icr]) {
        ldup = true;
        if (m_debug) {
          std::cout << " TileFilterResult.addCross: kcrIndex=" << kcrIndex << " is already in crossing list: Kcross =";
          for (int j = 0; j < Namp; j++) {
            std::cout << " " << m_vCross[j];
          }
          std::cout << std::endl;
        }
        if (ldup) break;
      }
    } // end for loop
  } // end "if(Nparam>1)"
  if (ldup) {
    iret = 1;
    return iret;
  }

  // Add the new crossing.
  iret = 0;
  m_vCross.push_back(kcrIndex);
  std::sort(m_vCross.begin(), m_vCross.end());
  m_nParam = m_nParam + 1;

  // Since we have a new Vcross configuration, iFitIndex is not yet defined for it.
  m_iFitIndex = -1;

  if (m_debug) {
    int Namp = m_nParam - 1;
    std::cout << " TileFilterResult.addCross.  Exit with Nparam=" << m_nParam << ", Vcross=";
    for (int icr = 0; icr < Namp; icr++) {
      std::cout << " " << std::setw(3) << m_vCross[icr];
    }
    std::cout << std::endl;
  }

  return iret;
}
//============================================================================= 
int TileFilterResult::dropCross(int idrop) {
  // Drop the crossing stored in amplitude # iamp.
  int iret = -1;
  int Namp = m_nParam - 1;
  for (int iamp = 1; iamp < Namp; iamp++) {
    if (m_vCross[iamp] == idrop) {
      // Erase the crossing.
      iret = 0;
      m_vCross.erase(m_vCross.begin() + iamp);
      //  std::sort(Vcross.begin(), Vcross.end() );
      m_nParam = m_nParam - 1;
    }
  }
  if (iret != 0) {
    std::cout << "error in TileFilterResult.dropCross:  idrop=" << idrop << "but vcross =";
    for (int iamp = 0; iamp < Namp; iamp++) {
      std::cout << " " << m_vCross[iamp];
    }
    std::cout << std::endl;
    return iret;
  }

  // Since we have a new Vcross configuration, iFitIndex is not yet defined for it.
  m_iFitIndex = -1;

  if (m_debug) {
    int Namp = m_nParam - 1;
    std::cout << " TileFilterResult.dropCross.  Exit with Nparam=" << m_nParam << ", Vcross=";
    for (int icr = 0; icr < Namp; icr++) {
      std::cout << " " << std::setw(3) << m_vCross[icr];
    }
    std::cout << std::endl;
  }

  return iret;
}
