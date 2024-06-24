/*
  Copyright (C) 2002-202' CERN for the benefit of the ATLAS collaboration
*/
//  Count.cxx
//  TopoCore
//  Created by Carlos Moreno on 05/05/20.
//
// Adapted for GlobalSim

#include "Count.h"

namespace GlobalSim {
  void Count::setSizeCount(unsigned int sizeCount){

    m_sizeCount = sizeCount;
    
    // Compute the final bits that make up the output of the
    // multiplicity algorithms

    // saturation limit given by the number of bits allowed
    // for the trigger line
    unsigned int maximum = (1 << m_nBits) - 1;

    std::bitset<128> mask = (sizeCount < maximum)?sizeCount:maximum;

    m_count = (mask << m_firstBit);
    
  }
}

std::ostream & operator<<(std::ostream& os, const GlobalSim::Count& count) {
  os << "GlobalSim::Count:\n"
     << "first bit " << count.firstBit()
     << " last bit " << count.lastBit()
     << " nbits " << count.nBits()
     << " sizeCount " << count.sizeCount()
     << '\n';
  return os;
}
