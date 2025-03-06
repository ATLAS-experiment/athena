/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMTOB_H
#define GLOBALSIM_EEMTOB_H

#include "AlgoConstants.h"
#include <bitset>
#include <ostream>
#include <memory>

namespace GlobalSim {

  struct eEmTob {
    // vhdl type: record
    
    std::bitset<AlgoConstants::eFexEtBitWidth> Et;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> REta;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> RHad;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> WsTot;
    std::bitset<AlgoConstants::eFexEtaBitWidth> Eta;
    std::bitset<AlgoConstants::eFexPhiBitWidth> Phi;
    std::bitset<1> Overflow;

    std::bitset<32> asBits() const;
  };

  using eEmTobPtr = std::shared_ptr<eEmTob>;
}

std::ostream& operator << (std::ostream&, const GlobalSim::eEmTob&);


#endif
