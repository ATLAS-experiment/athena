/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/* date of creation : 27/01/2004 */

#include <cstdint>

#ifndef CALO_GAIN_H
#define CALO_GAIN_H

namespace CaloGain {
  enum CaloGain : int8_t {
    TILELOWLOW =-16 ,
    TILELOWHIGH =-15 ,
    TILEHIGHLOW  = -12,
    TILEHIGHHIGH = -11,
    TILEONELOW   =-4,
    TILEONEHIGH  =-3,
    INVALIDGAIN = -1, LARHIGHGAIN = 0, LARMEDIUMGAIN = 1,  LARLOWGAIN = 2,
    LARNGAIN =3,
    LARNGAINR4=2,
    UNKNOWNGAIN=4};
}


#endif //CALO_GAIN_H

