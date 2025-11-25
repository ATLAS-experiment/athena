/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IeEmEg1eRatioTOB.h"

#include <sstream>

namespace GlobalSim::IOBitwise{
  std::string IeEmEg1eRatioTOB::to_string() const {
    std::stringstream ss;

    ss << '\n'
       << IeEmTOB::to_string()
       << " eGamma1 eRatio result " << eGamma1eRatio_bits() << " (" << eGamma1eRatio_bits().to_ulong() << ")";
    return ss.str();
  }
}

std::ostream& operator<<(std::ostream& os, const GlobalSim::IOBitwise::IeEmEg1eRatioTOB& tob) {
  os << tob.to_string();
  return os;
}
