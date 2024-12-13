/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <functional>
#include <map>
#include <stdio.h>
#include "TruthUtils/HepMCHelpers.h"

#define TEST_FUNCTION(f) { if (helper_function == std::string(#f)) { std::cout << "Listing pdg_ids passing " << helper_function << std::endl; for (int pdg_id = 1; pdg_id < max_pid; ++pdg_id) { if (MC::f(pdg_id)) { if (MC::f(-pdg_id)) { std::cout << -pdg_id << ", " << pdg_id << "," << std::endl; } else { std::cout << pdg_id << "," << std::endl; } } } std::cout << std::flush; return 0; } }

int main(int argc, char** argv) {
  std::string bottom = "PDG_ID";
  std::string  current;

  const char* pdgString = "\0";
  std::string helper_function{""};
  if (argc > 1) {
    pdgString = argv[1];
    helper_function = std::string(pdgString);
  }
  const int max_pid(10000000);
  if ( helper_function.empty() )  {
    std::cout << "Please specify the helper function that you want to test on the command-line."<<std::endl; return 11;
  }
  TEST_FUNCTION(hasBottom)
  TEST_FUNCTION(hasCharm)
  TEST_FUNCTION(hasStrange)
  TEST_FUNCTION(hasTop)
  TEST_FUNCTION(isBBbarMeson)
  TEST_FUNCTION(isBSM)
  TEST_FUNCTION(isBaryon)
  TEST_FUNCTION(isBoson)
  TEST_FUNCTION(isBottom)
  TEST_FUNCTION(isBottomBaryon)
  TEST_FUNCTION(isBottomHadron)
  TEST_FUNCTION(isBottomMeson)
  TEST_FUNCTION(isCCbarMeson)
  TEST_FUNCTION(isChLepton)
  TEST_FUNCTION(isCharged)
  TEST_FUNCTION(isCharm)
  TEST_FUNCTION(isCharmBaryon)
  TEST_FUNCTION(isCharmHadron)
  TEST_FUNCTION(isCharmMeson)
  TEST_FUNCTION(isDM)
  TEST_FUNCTION(isDiquark)
  TEST_FUNCTION(isEMInteracting)
  TEST_FUNCTION(isElectron)
  TEST_FUNCTION(isExcited)
  TEST_FUNCTION(isGaugino)
  TEST_FUNCTION(isGeantino)
  TEST_FUNCTION(isGenSpecific)
  TEST_FUNCTION(isGenericMultichargedParticle)
  TEST_FUNCTION(isGlueball)
  TEST_FUNCTION(isGluon)
  TEST_FUNCTION(isGraviton)
  TEST_FUNCTION(isHadron)
  TEST_FUNCTION(isHeavyBaryon)
  TEST_FUNCTION(isHeavyHadron)
  TEST_FUNCTION(isHeavyMeson)
  TEST_FUNCTION(isHiddenValley)
  TEST_FUNCTION(isHiggs)
  TEST_FUNCTION(isKK)
  TEST_FUNCTION(isLeptoQuark)
  TEST_FUNCTION(isLepton)
  TEST_FUNCTION(isLightBaryon)
  TEST_FUNCTION(isLightHadron)
  TEST_FUNCTION(isLightMeson)
  TEST_FUNCTION(isMeson)
  TEST_FUNCTION(isMonopole)
  TEST_FUNCTION(isMuon)
  TEST_FUNCTION(isNeutral)
  TEST_FUNCTION(isNeutrino)
  TEST_FUNCTION(isNucleus)
  TEST_FUNCTION(isParton)
  TEST_FUNCTION(isPentaquark)
  TEST_FUNCTION(isPhoton)
  TEST_FUNCTION(isPythia8Specific)
  TEST_FUNCTION(isQuark)
  TEST_FUNCTION(isRBaryon)
  TEST_FUNCTION(isRGlueball)
  TEST_FUNCTION(isRHadron)
  TEST_FUNCTION(isRMeson)
  TEST_FUNCTION(isResonance)
  TEST_FUNCTION(isSlepton)
  TEST_FUNCTION(isSleptonLH)
  TEST_FUNCTION(isSleptonRH)
  TEST_FUNCTION(isSMLepton)
  TEST_FUNCTION(isSMNeutrino)
  TEST_FUNCTION(isSMQuark)
  TEST_FUNCTION(isSUSY)
  TEST_FUNCTION(isSquark)
  TEST_FUNCTION(isSquarkLH)
  TEST_FUNCTION(isSquarkRH)
  TEST_FUNCTION(isStrange)
  TEST_FUNCTION(isStrangeBaryon)
  TEST_FUNCTION(isStrangeHadron)
  TEST_FUNCTION(isStrangeMeson)
  TEST_FUNCTION(isStrongInteracting)
  TEST_FUNCTION(isTau)
  TEST_FUNCTION(isTechnicolor)
  TEST_FUNCTION(isTetraquark)
  TEST_FUNCTION(isTop)
  TEST_FUNCTION(isTopBaryon)
  TEST_FUNCTION(isTopHadron)
  TEST_FUNCTION(isTopMeson)
  TEST_FUNCTION(isTrajectory)
  TEST_FUNCTION(isTransportable)
  TEST_FUNCTION(isValid)
  TEST_FUNCTION(isW)
  TEST_FUNCTION(isZ)
  std::cout << "Ignored unknown function " << helper_function << std::endl;
  return 1;
}
