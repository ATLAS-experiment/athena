/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   creteBlindingKeys.cxx
 * @author Wolfgang Walkowiak <Wolfgang.Walkowiak@cern.ch>
 *
 * @brief  Utility to create a set of blinding keys
 *
 * @param[in] option: -c : perform a quick encoding/decoding check
 */

// system includes:
#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <cstdlib>
#include <ctime>

// Local include(s):
#include "BPhysTools/SimpleEncrypter.h"

int main(int argc, char* argv[]) {

  // Enable check (-c flag)
  bool doCheck(false);
  int  nChecks(1);
  if ( argc > 1 ) {
    std::string arg(argv[1]);
    if ( arg == "-c" ) doCheck = true;
  }
  if ( argc > 2 ) {
    nChecks = atoi(argv[2]);
  }
  
  // Helper object
  xAOD::SimpleEncrypter senc;

  // Create key pair
  std::pair<std::string, std::string> keys = senc.genKeyPair();

  std::cout << "\nBlinding keys generated:\n";
  std::cout << "  Private key: " << keys.first << "\n";
  std::cout << "  Public  key: " << keys.second << "\n";
  std::cout << std::endl;

  // check that encryption works
  if ( doCheck ) {
    srand(static_cast<unsigned>(time(0)));
    
    std::cout << "Encryption test:\n";
    int nOK(0);
    //going to assume that no pathological values were used.
    //coverity[TAINTED_SCALAR]
    for (int i=0; i<nChecks; ++i) {
      //assuming that weak crypto is ok
      //coverity[dont_call]
      float val = 10000.* static_cast <float>(rand())/(static_cast <float> (RAND_MAX));
      // float val = 5267.23;
      float enc = senc.encrypt(val);
      float dec = senc.decrypt(enc);
      if ( dec == val ) ++nOK;
      if ( i == 0 || dec != val ) {
        std::cout << "  Test # " << i << "\n";
        std::cout << "    val = " << val << "\n";
        std::cout << "    enc = " << enc << "\n";
        std::cout << "    dec = " << dec << "\n";
        if ( dec == val ) {
          std::cout << "  => worked!\n";
        } else {
          std::cout << "  => FAILED!\n";
        }
      } // if
    } // for
    std::cout << std::endl;
    std::cout << "Summary:\n";
    std::cout << "  nChecks: " << std::setw(12) << nChecks << "\n";
    std::cout << "  nOK    : " << std::setw(12) << nOK << "\n";
    std::cout << "  nFailed: " << std::setw(12) << nChecks - nOK << std::endl;
  } // if
  
  return 0;
}
