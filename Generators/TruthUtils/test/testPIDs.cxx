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
            if  (myline!=bottom) {return 2; }
            else continue;
        }
        int i = std::strtod(myline.substr(0,10).c_str(),nullptr);
        current.clear();
        current = std::to_string(i);
        TEST_FUNCTION(charge)
        TEST_FUNCTION(charge3)
        TEST_FUNCTION(hasBottom)
        TEST_FUNCTION(hasCharm)
        TEST_FUNCTION(hasStrange)
        TEST_FUNCTION(hasTop)
        TEST_FUNCTION(isBaryon)
        TEST_FUNCTION(isBottom)
        TEST_FUNCTION(isBottomBaryon)
        TEST_FUNCTION(isBottomHadron) // 10
        TEST_FUNCTION(isBottomMeson)
        TEST_FUNCTION(isBSM)
        TEST_FUNCTION(isCharged)
        TEST_FUNCTION(isCharm)
        TEST_FUNCTION(isCharmBaryon)
        TEST_FUNCTION(isCharmHadron)
        TEST_FUNCTION(isCharmMeson)
        TEST_FUNCTION(isChLepton)
        TEST_FUNCTION(isDiquark)
        TEST_FUNCTION(isDM) // 20
        TEST_FUNCTION(isElectron)
        TEST_FUNCTION(isEMInteracting)
        TEST_FUNCTION(isGenSpecific)
        TEST_FUNCTION(isGlueball)
        TEST_FUNCTION(isGluon)
        TEST_FUNCTION(isGraviton)
        TEST_FUNCTION(isHadron)
        TEST_FUNCTION(isHeavyBaryon)
        TEST_FUNCTION(isHeavyHadron)
        TEST_FUNCTION(isHeavyMeson) // 30
        TEST_FUNCTION(isHiggs)
        TEST_FUNCTION(isLepton)
        TEST_FUNCTION(isLeptoQuark)
        TEST_FUNCTION(isLightBaryon)
        TEST_FUNCTION(isLightHadron)
        TEST_FUNCTION(isLightMeson)
        TEST_FUNCTION(isMeson)
        TEST_FUNCTION(isMuon)
        TEST_FUNCTION(isNeutral)
        TEST_FUNCTION(isNeutrino) // 40
        TEST_FUNCTION(isNucleus)
        TEST_FUNCTION(isParton)
        TEST_FUNCTION(isPentaquark)
        TEST_FUNCTION(isPhoton)
        TEST_FUNCTION(isQuark)
        TEST_FUNCTION(isResonance)
        TEST_FUNCTION(isStrange)
        TEST_FUNCTION(isStrangeBaryon)
        TEST_FUNCTION(isStrangeHadron)
        TEST_FUNCTION(isStrangeMeson) // 50
        TEST_FUNCTION(isStrongInteracting)
        TEST_FUNCTION(isSUSY)
        TEST_FUNCTION(isTau)
        TEST_FUNCTION(isTop)
        TEST_FUNCTION(isTransportable)
        TEST_FUNCTION(isValid)
        TEST_FUNCTION(isW)
        TEST_FUNCTION(isZ)
        TEST_FUNCTION(threeCharge)
        /* NEW **/
        TEST_FUNCTION(isBBbarMeson) // 60
        TEST_FUNCTION(isCCbarMeson)
        TEST_FUNCTION(isExcited)
        TEST_FUNCTION(isGeantino)
        TEST_FUNCTION(isHiddenValley)
        TEST_FUNCTION(isKK)
        TEST_FUNCTION(isMonopole)
        TEST_FUNCTION(isPythia8Specific)
        TEST_FUNCTION(SUSY::isRBaryon)
        TEST_FUNCTION(SUSY::isRGlueball)
        TEST_FUNCTION(SUSY::isRHadron) // 70
        TEST_FUNCTION(SUSY::isRMeson)
        TEST_FUNCTION(SUSY::isSLepton)
        TEST_FUNCTION(isSMLepton)
        TEST_FUNCTION(isSMNeutrino)
        TEST_FUNCTION(isTetraquark)
        TEST_FUNCTION(isTopBaryon)
        TEST_FUNCTION(isTopHadron)
        TEST_FUNCTION(isTopMeson) // 80
        TEST_FUNCTION(isTrajectory)
        TEST_FUNCTION(leadingQuark)
        TEST_FUNCTION(SUSY::spin2)
        TEST_FUNCTION(SUSY::isSquark)
        if  (myline!=current) { printf("reference :%s\ncalculated:%s\n",myline.c_str(),current.c_str()); return 1; }
        //printf("%s\n",current.c_str());
        processed=true;
    }
    return 0;
//cat ../AtlasPID.h | grep template | grep class | tr -s ' '| sort | cut -f 5 -d' '| cut -f 1 -d'(' | sort | sed 's/^/TEST_FUNCTION(/' | sed 's/$/)/'

}
