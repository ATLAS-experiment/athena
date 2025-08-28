/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "StripData.h"
#include <ostream>


std::ostream& operator<<(std::ostream& os,
			 const GlobalSim::StripData& sd) {

  os << "StripData eta: " << sd.m_eta
     << " phi " << sd.m_phi
     << " e " << sd.m_e;
  return os;
}
