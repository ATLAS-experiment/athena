#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <map>
#include <stdio.h>
#include "TruthUtils/HepMCHelpers.h"
#define TEST_FUNCTION(f) { current = std::string(#f); auto a = MC::f(pdg_id); current+=(std::string(" ")+std::to_string(int(a))); if (!processed) bottom += (std::string(" ") + std::string(#f)); printf("%s\n",current.c_str()); }
#define TEST_FUNCTION_DOUBLE(f) { current = std::string(#f); auto a = MC::f(pdg_id); current+=(std::string(" ")+std::to_string(double(a))); if (!processed) bottom += (std::string(" ") + std::string(#f)); printf("%s\n",current.c_str()); }
#define TEST_FUNCTION_VECTOR(f) { current = std::string(#f); auto a = MC::f(pdg_id); for (const auto& q: a) { current+=(std::string(" ")+std::to_string(int(q))); } if (!processed) bottom += (std::string(" ") + std::string(#f)); printf("%s\n",current.c_str()); }
int main(int argc, char** argv) {
  std::string bottom = "PDG_ID";
  bool processed = false;
  std::string  current;

  const char* pdgString = "\0";
  int pdg_id = 0;
  if (argc > 1) {
    pdgString = argv[1];
    pdg_id = std::strtod(pdgString, nullptr);
  }
  else {
    std::cout << "Please specify the pdg_id value that you want to test on the command-line."<<std::endl; return 11;
  }

  std::cout << "Helper function outputs for pdg_id: " << pdg_id << std::endl;
  std::cout << ">>>>>>>>>> General properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION_DOUBLE(charge)
  TEST_FUNCTION(charge3)
  TEST_FUNCTION_DOUBLE(fractionalCharge)
  TEST_FUNCTION(isBoson)
  TEST_FUNCTION(isBSM)
  TEST_FUNCTION(isCharged)
  TEST_FUNCTION(isEMInteracting)
  TEST_FUNCTION(isGeantino)
  TEST_FUNCTION(isGenSpecific)
  TEST_FUNCTION(isLepton)
  TEST_FUNCTION(isNeutral)
  TEST_FUNCTION(isNucleus)
  TEST_FUNCTION(isPythia8Specific)
  TEST_FUNCTION(isQuark)
  TEST_FUNCTION(isResonance)
  TEST_FUNCTION(isSUSY)
  TEST_FUNCTION(isStrongInteracting)
  TEST_FUNCTION(isTrajectory)
  TEST_FUNCTION(isTransportable)
  TEST_FUNCTION(isValid)
  TEST_FUNCTION_DOUBLE(spin)
  TEST_FUNCTION(spin2)
  TEST_FUNCTION_DOUBLE(threeCharge)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> Quark properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION(isBottom)
  TEST_FUNCTION(isCharm)
  TEST_FUNCTION(isStrange)
  TEST_FUNCTION(isSMQuark)
  TEST_FUNCTION(isTop)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> Lepton properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION(isChLepton)
  TEST_FUNCTION(isElectron)
  TEST_FUNCTION(isMuon)
  TEST_FUNCTION(isNeutrino)
  TEST_FUNCTION(isSMLepton)
  TEST_FUNCTION(isSMNeutrino)
  TEST_FUNCTION(isTau)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> Boson properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION(isGluon)
  TEST_FUNCTION(isGlueball)
  TEST_FUNCTION(isHiggs)
  TEST_FUNCTION(isPhoton)
  TEST_FUNCTION(isW)
  TEST_FUNCTION(isZ)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> Hadron properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION_VECTOR(containedQuarks)
  TEST_FUNCTION(hasBottom)
  TEST_FUNCTION(hasCharm)
  TEST_FUNCTION(hasStrange)
  TEST_FUNCTION(hasTop)
  TEST_FUNCTION(isBBbarMeson)
  TEST_FUNCTION(isBaryon)
  TEST_FUNCTION(isBottomBaryon)
  TEST_FUNCTION(isBottomHadron)
  TEST_FUNCTION(isBottomMeson)
  TEST_FUNCTION(isCCbarMeson)
  TEST_FUNCTION(isCharmBaryon)
  TEST_FUNCTION(isCharmHadron)
  TEST_FUNCTION(isCharmMeson)
  TEST_FUNCTION(isDiquark)
  TEST_FUNCTION(isHadron)
  TEST_FUNCTION(isHeavyBaryon)
  TEST_FUNCTION(isHeavyHadron)
  TEST_FUNCTION(isHeavyMeson)
  TEST_FUNCTION(isLightBaryon)
  TEST_FUNCTION(isLightHadron)
  TEST_FUNCTION(isLightMeson)
  TEST_FUNCTION(isMeson)
  TEST_FUNCTION(isParton)
  TEST_FUNCTION(isPentaquark)
  TEST_FUNCTION(isStrangeBaryon)
  TEST_FUNCTION(isStrangeHadron)
  TEST_FUNCTION(isStrangeMeson)
  TEST_FUNCTION(isTetraquark)
  TEST_FUNCTION(isTopBaryon)
  TEST_FUNCTION(isTopHadron)
  TEST_FUNCTION(isTopMeson)
  TEST_FUNCTION(leadingQuark)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> SUSY Properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION(isGaugino)
  TEST_FUNCTION(isRBaryon)
  TEST_FUNCTION(isRGlueball)
  TEST_FUNCTION(isRHadron)
  TEST_FUNCTION(isRMeson)
  TEST_FUNCTION(isSlepton)
  TEST_FUNCTION(isSleptonLH)
  TEST_FUNCTION(isSleptonRH)
  TEST_FUNCTION(isSquark)
  TEST_FUNCTION(isSquarkLH)
  TEST_FUNCTION(isSquarkRH)
  std::cout << "========================================" << std::endl;
  std::cout << ">>>>>>>>>> BSM Properties <<<<<<<<<<" << std::endl;
  TEST_FUNCTION(isDM)
  TEST_FUNCTION(isExcited)
  TEST_FUNCTION(isGenericMultichargedParticle)
  TEST_FUNCTION(isGraviton)
  TEST_FUNCTION(isHiddenValley)
  TEST_FUNCTION(isKK)
  TEST_FUNCTION(isLeptoQuark)
  TEST_FUNCTION(isMonopole)
  TEST_FUNCTION(isTechnicolor)
  std::cout << "========================================" << std::endl;
  return 0;
}
