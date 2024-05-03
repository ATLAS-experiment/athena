/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoSimulationUtils/Conversions.h"

int TSU::toTopoInteger(float value, unsigned int resolution) {
    float tmp = value*resolution;
    float index;
    if ( (abs(tmp)-0.5)/2. == std::round((abs(tmp)-0.5)/2.) ) {
        if ( tmp>0 ) { index = std::floor(tmp); }               
        else { index = std::ceil(tmp); }                      
    } else { 
        index = std::round(tmp); 
    }
    
    return static_cast<int>(index);
}

unsigned int TSU::toTopoPhi(float phi) {
    if (phi < 0) { phi += 2*M_PI; }  //Convert to [0,2pi) range so phi can eventually be treated as unsigned
    phi *= TSU::phiRescaleFactor;
    int phiInt = toTopoInteger(phi, TSU::phiIntegerResolution) % 128;
    return static_cast<unsigned int>(phiInt);
}

int TSU::toTopoEta(float eta) {
    return toTopoInteger(eta, TSU::etaIntegerResolution);
}
   

int TSU::toSigned(unsigned bits, unsigned length) {
    if ( ((bits >> (length-1)) & 1) == 0) { //sign bit not set
        return bits & ((1<<length)-1);
    }
    //sign bit set, convert assuming twos complement
    return (bits & ((1<<length)-1)) - (1<<length);
}

int TSU::toSigned(const std::string& bits) {
    return TSU::toSigned(std::stoi(bits, nullptr, 2), bits.length());    
}
