/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulationUtils_Conversions
#define L1TopoSimulationUtils_Conversions

#include <math.h>
#include <string>

namespace TSU {

    static constexpr float phiRescaleFactor = 3.2/M_PI; 
    static constexpr unsigned int phiIntegerResolution = 20; // only correct after(!) multiplying float values with phiRescaleFactor  
    static constexpr unsigned int etaIntegerResolution = 40; 
    
    /* 
         @brief calculate the eta and phi L1Topo indices

         The exact eta and phi coordinates are rounded according to a particular L1Topo granularity
         Using product instead of division avoids unexpected rounding errors due to precision
         Also, LUTs for the firmware are built using Python 3.x numpy.round(), which is different from std::round()
         Input: value = float values, resolution = inverse of intended step size
         Output: integer L1Topo values
      */
    int toTopoInteger(float value, unsigned int resolution);
    
    /* @brief convert a floating point phi coordinate (in radians) to L1Topo's internal integer representation */
    unsigned int toTopoPhi(float phi);
    
    /* @brief convert a floating point eta coordinate to L1Topo's internal integer representation */
    int toTopoEta(float eta);
   
    /* @brief convert an (unsigned) representation of a bit string into a signed value assuming a given length of the bit string (incl. possible sign bit) */
    int toSigned(unsigned bits, unsigned length);
    /* @brief convert string representation of a bit string into a signed value assuming a given length of the bit string (incl. possible sign bit) */
    int toSigned(const std::string& bits);
   
   
}
#endif
