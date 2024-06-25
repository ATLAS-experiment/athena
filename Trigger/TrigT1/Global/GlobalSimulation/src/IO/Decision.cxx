/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  Decision.cxx
//  TopoCore
//  Created by Joerg Stelzer on 11/16/12.
//
// Adapted for GlobalL1TopoSim


#include "Decision.h"

#include <ostream>
#include <sstream>
#include <stdexcept>

namespace GlobalSim {
  void
  Decision::setBit(unsigned int index, bool set) {
    if(index >= m_nBits) {
      std::stringstream ss;
      ss << "Setting bit "
	 << index << ", but max index allowed is " << m_nBits-1;
      throw std::runtime_error(ss.str());
    }
    
    if(set) { 
      m_decision |= (0x1<<index);
    } else {
      m_decision &= ~(0x1<<index);
    }
  }
  
  std::ostream&
  operator<<(std::ostream& os, const Decision& dec) {
    unsigned int bit = dec.nBits();
    if(dec.nBits()==1) {
      os << "decision (1 bit, position " << dec.firstBit() << "): ";
    } else {
      os << "decision (" << dec.nBits()
	 << " bits, position " << dec.firstBit()
	 << " - "
	 << dec.lastBit() << "): ";
    }
    
    while(bit!=0) {
      os << (dec.bit(--bit)?1:0);
    }
    os << "(overflow "<<dec.overflow()<<")";
    return os;
  }
}

std::ostream & operator<<(std::ostream& os, const GlobalSim::Decision& d){
  os << "Decision:\n"
     << " decision : " << d.decision()
     << " first bit : " << d.firstBit() 
     << " last bit : " << d.lastBit() 
     << " n bits : " << d.nBits()
     << std::boolalpha
     << " overflow : " << d.overflow() 
     << " ambiguity : " << d.ambiguity() 
     <<'\n'<< std::noboolalpha; //restore stream state
  return os;
}


