//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_HEXTOB2BITSETTOB_H
#define GLOBALSIM_HEXTOB2BITSETTOB_H

#include <string>
#include <bitset>

namespace GlobalSim {
  std::bitset<72> hexTOB2bitsetTOB(std::string s);
}
#endif
