/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IeEmTOB.h"

#include <sstream>

namespace GlobalSim::IOBitwise{
  std::string IeEmTOB::to_string() const {
    std::stringstream ss;
    
    ss << '\n'
       << ICommonTOB::to_string()
       << " RHad " << RHad_bits() << " (" <<   RHad_bits().to_ulong() << ")"
       << " REta " << REta_bits() << " (" <<   REta_bits().to_ulong() << ")"
       << " WsTot " << WsTot_bits() << " (" <<   WsTot_bits().to_ulong() << ")";
    return ss.str();
  }
}

std::ostream& operator<<(std::ostream& os, const GlobalSim::IOBitwise::IeEmTOB& tob) {
  os << tob.to_string();
  return os;
}
