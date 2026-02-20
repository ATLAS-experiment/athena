//Dear emacs, this is -*-c++-*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARELECCALIB_LARPROVENANCE
#define LARELECCALIB_LARPROVENANCE

#include <stdint.h>

namespace LArProv {
  enum LArProvenance : uint16_t {

    PEAKAVG      = 0x1, // Only used in early commissioning
    PEAKPARABOLA = 0x3, // Only used in early commissioning
    PEAKCUBIC    = 0x4, // Only used in early commissioning
    PEAKOFC      = 0x5, // Default Run 1 - Run 3
    PEAKTILEINFO = 0x6, // Only used in early commissioning
    PEAKNN       = 0x7, // Neural-Network peak reco (run 4 plans) 
    PATCHED      = 0x8, // patched from the SuperCells or neighbors
    RAMPCONST     = 0x10, //use hard-coded constant Ramp value (only early commissioning)
    RAMPDB        = 0x20, //Ramp from DB (default)
    PEDSAMPLEZERO = 0x40, //use ADC[0] as pedestal value (only early commissioning)
    SCPASSBCIDMAX = 0x40, //Supercell peak finder (overlap with PEDSAMPLEZERO should not matter)
    PEDDB         = 0x80, //Pedestal from DB (default)
    
    DEFAULTRECO   = PEAKOFC | RAMPDB | PEDDB, //0x00A5;
    ITERCONVERGED = 0x0100, //Iteration convered (in case of OF-Iteration)
    SCTIMEPASS    = 0x0200, //Supercell inside time-window
    SATURATED     = 0x0400, //ADC 0 or 4096 
    MASKED        = 0x0800, //Known noisy cell, E set to zero
    DSPCALC       = 0x1000, //Energy from online calculation
    QTPRESENT     = 0x2000, //Quality and time values are valid
    DEADFEB       = 0x4000, //Front-End board broken or inactive
  };

  inline bool test(const uint16_t prov, const LArProvenance check) {
    if (check & 0xF) {
      //The first four bits are a number, require exact match 
      if ((prov & 0xF) != (check &0xF)) return false;
    }
    //The remaining bits are independent, check only if 'check' bits are set
    return (((prov >>4)  & (check >>4)) ==  (check >>4));
  }

  
} // end namespace 

#endif

/*
Bit definitions :
The first four bits are enumberating the peak reconstruction method
0 RecoMethod1 
1 RecoMethod2
2 RecoMethod3
3 RecoMethod4 

Bits 4 to 15 are flags:
4 -> 0x10 RAMPCONST
5 -> 0x20 RAMPDB
6 -> 0x40 PEDSAMPLEZERO  
7 -> 0x80 PEDDB
8 -> 0x0100 ITERCONVERGED
9
10 -> 0x0400 SATURATED
11 -> 0x0800 MASKED
12 -> 0x1000 DSPCALC
13 -> 0x2000 QTPRESENT
14
15


Copy from https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CaloEventDataModel
The Provenance (16 bits unsigned) defining how the LArRawChannel was reconstructed. The bit meaning is
0x00FF are used the store the offline algorithm used, in the same way as now. For instance provenance & 0x00FF == 0x00A5 would tell that the pulse was reconstructed with the OFC with all calibrations from the database.
0x0100 is used to store the information on the convergence of the OFC iteration.
0x0400 saturated pulse (ADC>=4095 or =< 0 )
0x0800 cell is masked as noisy cell (E/t/Q set to 0)
0x1000 is used to tell that the raw channel comes from the DSP and not offline computation.
0x2000 is used to tell that time and quality information are available for this channel
*/

