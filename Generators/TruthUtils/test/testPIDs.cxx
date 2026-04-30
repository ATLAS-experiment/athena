/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <map>
#include <stdio.h>
#include "TruthUtils/HepMCHelpers.h"
#define TEST_FUNCTION(f) { auto a = MC::f(i); current+=(std::string(" ")+std::to_string(int(a))); if (!processed) bottom += (std::string(" ") + std::string(#f));}
int main(int argc, char** argv) {
  std::string bottom = "PDG_ID";
  bool processed = false;
  std::string  current;

  const char* reffile_name = "\0";
  if (argc > 1)
    reffile_name = argv[1];
  else {
    reffile_name = getenv ("REFERENCE_FILE");
    if (!reffile_name)
      reffile_name = "share/AtlasPIDTest.ref";
  }
  std::ifstream myfile (reffile_name);
  std::string myline;
  if ( !myfile.is_open() )  { std::cerr<<"Cannot open the reference file"<<std::endl; return 11;}
  while ( myfile ) {
    std::getline (myfile, myline);
    if (!myline.size()||myline.at(0)=='*') continue;
    if (myline.substr(0,6)=="PDG_ID") {
      if  (myline!=bottom) {
        // uncomment when adding new helper functions
        // printf("%s\n",bottom.c_str()); continue;
        return 2;
      }
      else continue;
    }
    int i = std::strtod(myline.substr(0,10).c_str(),nullptr);
    current.clear();
    current = std::to_string(i);
    TEST_FUNCTION(baryonNumber)
    TEST_FUNCTION(baryonNumber3)
    TEST_FUNCTION(charge)
    TEST_FUNCTION(charge3)
    TEST_FUNCTION(hasBottom)
    TEST_FUNCTION(hasCharm)
    TEST_FUNCTION(hasStrange)
    TEST_FUNCTION(hasTop)
    TEST_FUNCTION(isBBbarMeson)
    TEST_FUNCTION(isBSM) // 10
    TEST_FUNCTION(isBaryon)
    TEST_FUNCTION(isBoson)
    TEST_FUNCTION(isBottom)
    TEST_FUNCTION(isBottomBaryon)
    TEST_FUNCTION(isBottomHadron)
    TEST_FUNCTION(isBottomMeson)
    TEST_FUNCTION(isCCbarMeson)
    TEST_FUNCTION(isChLepton)
    TEST_FUNCTION(isCharged)
    TEST_FUNCTION(isCharm) // 20
    TEST_FUNCTION(isCharmBaryon)
    TEST_FUNCTION(isCharmHadron)
    TEST_FUNCTION(isCharmMeson)
    TEST_FUNCTION(isDM)
    TEST_FUNCTION(isDiquark)
    TEST_FUNCTION(isEMInteracting)
    TEST_FUNCTION(isElectron)
    TEST_FUNCTION(isExcited)
    TEST_FUNCTION(isGaugino)
    TEST_FUNCTION(isGeantino) // 30
    TEST_FUNCTION(isGenSpecific)
    TEST_FUNCTION(isGenericMultichargedParticle)
    TEST_FUNCTION(isGlueball)
    TEST_FUNCTION(isGluon)
    TEST_FUNCTION(isGraviton)
    TEST_FUNCTION(isHadron)
    TEST_FUNCTION(isHeavyBaryon)
    TEST_FUNCTION(isHeavyBoson)
    TEST_FUNCTION(isHeavyHadron)
    TEST_FUNCTION(isHeavyMeson) // 40
    TEST_FUNCTION(isHiddenValley)
    TEST_FUNCTION(isHiggs)
    TEST_FUNCTION(isKK)
    TEST_FUNCTION(isLeptoQuark)
    TEST_FUNCTION(isLepton)
    TEST_FUNCTION(isLightBaryon)
    TEST_FUNCTION(isLightHadron)
    TEST_FUNCTION(isLightMeson)
    TEST_FUNCTION(isMeson)
    TEST_FUNCTION(isMonopole) // 50
    TEST_FUNCTION(isMSSMHiggs)
    TEST_FUNCTION(isMuon)
    TEST_FUNCTION(isNeutral)
    TEST_FUNCTION(isNeutrino)
    TEST_FUNCTION(isNeutrinoRH)
    TEST_FUNCTION(isNucleus)
    TEST_FUNCTION(isParton)
    TEST_FUNCTION(isPentaquark)
    TEST_FUNCTION(isPhoton)
    TEST_FUNCTION(isPythia8Specific) // 60
    TEST_FUNCTION(isQuark)
    TEST_FUNCTION(isRBaryon)
    TEST_FUNCTION(isRGlueball)
    TEST_FUNCTION(isRHadron)
    TEST_FUNCTION(isRMeson)
    TEST_FUNCTION(isResonance)
    TEST_FUNCTION(isSlepton)
    TEST_FUNCTION(isSleptonLH)
    TEST_FUNCTION(isSleptonRH)
    TEST_FUNCTION(isSMLepton) // 70
    TEST_FUNCTION(isSMNeutrino)
    TEST_FUNCTION(isSUSY)
    TEST_FUNCTION(isSquark)
    TEST_FUNCTION(isSquarkLH)
    TEST_FUNCTION(isSquarkRH)
    TEST_FUNCTION(isStrange)
    TEST_FUNCTION(isStrangeBaryon)
    TEST_FUNCTION(isStrangeHadron)
    TEST_FUNCTION(isStrangeMeson)
    TEST_FUNCTION(isStrongInteracting) // 80
    TEST_FUNCTION(isTau)
    TEST_FUNCTION(isTechnicolor)
    TEST_FUNCTION(isTetraquark)
    TEST_FUNCTION(isTop)
    TEST_FUNCTION(isTopBaryon)
    TEST_FUNCTION(isTopHadron)
    TEST_FUNCTION(isTopMeson)
    TEST_FUNCTION(isTrajectory)
    TEST_FUNCTION(isTransportable)
    TEST_FUNCTION(isValid) // 90
    TEST_FUNCTION(isW)
    TEST_FUNCTION(isWeaklyDecayingBHadron)
    TEST_FUNCTION(isWeaklyDecayingCHadron)
    TEST_FUNCTION(isZ)
    TEST_FUNCTION(leadingQuark)
    TEST_FUNCTION(numberOfLambdas)
    TEST_FUNCTION(numberOfProtons)
    TEST_FUNCTION(spin2)
    TEST_FUNCTION(strangeness)
    TEST_FUNCTION(threeCharge) // 100
    processed=true;
    // uncomment when updating reference file
    // printf("%s\n",current.c_str()); continue;
    if  (myline!=current) { printf("reference :%s\ncalculated:%s\n",myline.c_str(),current.c_str()); return 1; }
  }
  return 0;
  //cat ../AtlasPID.h | grep template | grep class | tr -s ' '| sort | cut -f 5 -d' '| cut -f 1 -d'(' | sort | sed 's/^/TEST_FUNCTION(/' | sed 's/$/)/'

}
