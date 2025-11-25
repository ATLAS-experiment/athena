/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ICommonTOB.h"

#include <sstream>

namespace GlobalSim::IOBitwise {

  std::string ICommonTOB::to_string() const {
    std::stringstream ss;
    
    auto et =  et_bits();
    auto eta = eta_bits();
    auto phi = phi_bits();
    
    ss << "Et: " << et << " (" << et.to_ulong() << ") "
       << "Eta: " << eta << " (" << eta.to_ulong() << ") "
       << "Phi: " << phi << " (" << phi.to_ulong() << ") ";
    
    return ss.str();
  
  }
}

std::ostream& operator << (std::ostream& os,
			   const GlobalSim::IOBitwise::ICommonTOB& tob) {
  os << tob.to_string();
  return os;
}

