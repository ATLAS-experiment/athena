/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GENERICTOB_H
#define GLOBALSIM_GENERICTOB_H


#include "AlgoConstants.h"
#include "AlgoDataTypes.h"
#include "eEmTob.h"

#include <bitset>
#include <ostream>
#include <vector>
#include <memory>

namespace GlobalSim {


  class GenericTob {
     // vhdl type: record

  public:
    constexpr static std::size_t GenericEtBitWidth{13};
    constexpr static std::size_t GenericEtaBitWidth{9};
    
    constexpr static std::size_t
    GenericAbsoluteEtBitWidth{GenericEtaBitWidth-1};
    
    constexpr static std::size_t GenericPhiBitWidth{7};
    constexpr static std::size_t GenericMuonFlagBitWidth{2};
    
    
    GenericTob(const eEmTobPtr& in_tob);
    
    unsigned int Et() const {return m_Et;}
    int Eta() const {return m_Eta;}
    int Phi() const {return m_Phi;}
    int Charge() const {return m_Charge;}
    const std::bitset<1>& overflow() const {return m_Overflow;}
    const std::bitset<32>& as_bits() const {return m_bits;}
    
  private:

    void setBits(const eEmTobPtr& in_tob);
 
    // at the time of writing the int widhs in the VHDL code were all < 16.
    
    unsigned int m_Et{0}; // originally 13 bits
    int m_Eta{0}; // originally 9 bits
    int m_Phi{0}; // originally 7 bits
    int m_Charge{0}; // originally 2 bits
    std::bitset<1> m_Overflow; // originally 1 bit

    std::bitset<32> m_bits;
    
  };

}

std::ostream& operator << (std::ostream&, const GlobalSim::GenericTob&);

#endif
