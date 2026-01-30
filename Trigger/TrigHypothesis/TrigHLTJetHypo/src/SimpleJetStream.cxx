/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./SimpleJetStream.h"
#include <ostream>
#include <ios> //for std::boolalpha

std::ostream& operator << (std::ostream& os ,
			   const SimpleJetStream& js) {

  os << "SimpleJetStream id " << js.m_id
     << " m_valid "  << std::boolalpha << js.m_valid
     << " no of jets: " << js.m_jets.size()
     << " m_ind "  << js.m_ind
     << std::noboolalpha;//restore stream state
  return os;
}

