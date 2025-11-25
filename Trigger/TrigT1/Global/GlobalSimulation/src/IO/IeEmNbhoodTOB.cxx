/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IeEmNbhoodTOB.h"

#include <sstream>

namespace GlobalSim::IOBitwise{
  std::string IeEmNbhoodTOB::to_string() const {
    std::stringstream ss;

    ss << '\n'
       << IeEmTOB::to_string()
       << "\n"
       << "Neighbourhood content:\n"
       << Neighbourhood().to_string();
    return ss.str();
  }
}

std::ostream& operator<<(std::ostream& os, const GlobalSim::IOBitwise::IeEmNbhoodTOB& tob) {
  os << tob.to_string();
  return os;
}
