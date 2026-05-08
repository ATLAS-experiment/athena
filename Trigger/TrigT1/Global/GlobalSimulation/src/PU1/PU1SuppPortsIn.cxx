/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PU1SuppPortsIn.h"

#include <iomanip>
#include <sstream>
#include <ostream>

namespace GlobalSim {
// Extra space for future developments if needed
}

std::ostream& operator<<(std::ostream& os,
                         const GlobalSim::PU1SuppPortsIn& ports_in) {
  std::ios_base::fmtflags original_flags = os.flags();
  char original_fill = os.fill();

  os << "PU1 TOB: ";
  for (int i = 0; i < 4; ++i) {
    os << std::hex << std::setw(16) << std::setfill('0')
       << ports_in.m_I_PU1TobData[i];
  }
  os << " (rho: " << ports_in.m_rho << ")";

  os.flags(original_flags);
  os.fill(original_fill);

  return os;
}
