/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IeEmTOB.h"


using namespace GlobalSim::IOBitwise;

std::ostream& operator<<(std::ostream& os, const IeEmTOB& tob) {
  os << tob.to_string();
  return os;
}
