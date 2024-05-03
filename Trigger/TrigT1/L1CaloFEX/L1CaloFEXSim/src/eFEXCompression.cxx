/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/***************************************************************************
                          eFEXCompression.cxx  -  description
                             -------------------
    begin                : 07-02-2019
    email                : Alan.Watson@cern.ch antonio.jacques.costa@cern.ch
 ***************************************************************************/

#include "L1CaloFEXSim/eFEXCompression.h"

namespace LVL1 {

const int eFEXCompression::s_steps[] = {25, 50, 100, 400};
const int eFEXCompression::s_minET[] = {-750, 5600, 18400, 44000};
const int eFEXCompression::s_minCode[] = {2, 256, 512, 768};
std::atomic<bool> eFEXCompression::s_disableNoiseCuts = false;

unsigned int eFEXCompression::compress(int Et) {

  // check for saturation
  if (Et == s_eFEXOverflow*s_eFEXstep) return s_LArSaturated;
  // Check for overflow
  if (Et >= s_maxET) return s_LArOverflow;
 
  // Find which range the ET value is in
  int range = -1;
  for (unsigned int i = 0; i < s_nRanges; i++) {
    if (Et < s_minET[i]) break;
    range = i;
  }

  // Calculate code
  unsigned int code = 0;

  if (range < 0) {
    // Below minimum value
    code = s_LArUnderflow; 
  }
  else {
    // Lies inside one of the value ranges
    int steps = (Et - s_minET[range])/s_steps[range];
    code = s_minCode[range] + steps;
  }

  return code;
}

int eFEXCompression::expand(unsigned int code) {

  // Deal with special codes first:
  if (code == s_NoData)  return 0;
  else if (code == s_LArInvalid || code == s_LArReserved || code > s_LArMaxCode) return s_error;
  else if (code == s_LArOverflow) return s_maxET;
  else if (code == s_LArSaturated) return s_eFEXOverflow*s_eFEXstep;

  /** Now expand code into an ET value.
      Start by finding what range the code is in */
  int range = 0;
  for (unsigned int i = 0; i < s_nRanges-1; ++i) {
    if (code < (unsigned int)s_minCode[i+1]) break;
    range++;
  }
  /// Now expand the value
  int Et = s_minET[range] + (code-s_minCode[range])*s_steps[range];

  return Et;
}

bool eFEXCompression::noiseCut(unsigned int code, int layer, bool ignoreDisable) {
    // Check if noise cut is passed - one cut per layer  
    bool pass=true;

    if(!ignoreDisable && s_disableNoiseCuts) return pass;

    switch(layer){
    case 0: 
      if(code<m_noisecutPS){ pass = false; }
      break;
    case 1:
      if(code<m_noisecutL1){ pass = false; }
      break;
    case 2:
      if(code<m_noisecutL2){ pass = false; }
      break;
    case 3: 
      if(code<m_noisecutL3){ pass = false; }
      break;
    case 4:
      if(code<m_noisecutHad){ pass = false; }
      break;
    default: 
      pass = false;
      break;
    }
    
    return pass;
}


unsigned int eFEXCompression::threshold(unsigned int code, int threshold) {

  /// Convert threshold into a compressed code
  unsigned int cut = eFEXCompression::compress(threshold);

  /// Zero code if < threshold
  if (code < cut) code = 0;

  return code;
}

int eFEXCompression::decode(int EtVal, int layer, bool ignoreDisable) {

  // Calculate code
  unsigned int tcode = eFEXCompression::compress(EtVal);

  /// Check if noise cut is passed
  unsigned int code = 0; // corresponds to 0 GeV
  if (eFEXCompression::noiseCut(tcode,layer,ignoreDisable)) {
    code = tcode;
  }

  /// Expand the ET value
  int Et = eFEXCompression::expand(code);
  
  /// Convert to eFEX digit scale: 25 MeV
  return Et/s_eFEXstep;
}

} // end of namespace bracket
