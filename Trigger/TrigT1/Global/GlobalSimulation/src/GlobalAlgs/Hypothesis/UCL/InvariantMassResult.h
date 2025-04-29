/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVARIANTMASSRESULT_H
#define GLOBALSIM_INVARIANTMASSRESULT_H

#include <bitset>

#include <ostream>

namespace GlobalSim {
  constexpr static std::size_t s_NumResultBits{4};
  using  InvariantMassResult = std::bitset<s_NumResultBits>;
}

CLASS_DEF( GlobalSim::InvariantMassResult , 227725379 , 1 )

#endif
