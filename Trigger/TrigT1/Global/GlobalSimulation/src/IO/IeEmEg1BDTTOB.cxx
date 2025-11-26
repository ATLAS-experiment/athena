/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IeEmEg1BDTTOB.h"

#include <sstream>

namespace GlobalSim::IOBitwise{
  std::string IeEmEg1BDTTOB::to_string() const {
    std::stringstream ss;
    
    ss << '\n'
       << IeEmTOB::to_string()
       << " eGamma1 BDT result " << eGamma1BDT_bits() << " (" << eGamma1BDT_bits().to_ulong() << ")";
    return ss.str();
  }
}

std::ostream& operator<<(std::ostream& os, const GlobalSim::IOBitwise::IeEmEg1BDTTOB& tob) {
  os << tob.to_string();
  return os;
}
