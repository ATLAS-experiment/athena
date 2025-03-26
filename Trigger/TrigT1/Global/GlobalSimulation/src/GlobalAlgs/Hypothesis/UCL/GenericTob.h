/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GENERICTOB_H
#define GLOBALSIM_GENERICTOB_H

#include "AlgoConstants.h"
#include "AlgoDataTypes.h"  //bitSetToInt
#include "eEmTob.h"

#include <bitset>
#include <vector>
#include <ostream>

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
    constexpr static std::size_t GenericOverflowWidth{1};

    constexpr static std::size_t GenericTobWidth =
      GenericEtBitWidth + GenericEtaBitWidth +
      GenericPhiBitWidth + GenericMuonFlagBitWidth +
      GenericOverflowWidth;
    
    GenericTob(){};
    GenericTob(const eEmTobPtr& in_tob);
    GenericTob(const std::string&);

    
    const std::bitset<13>& Et_bits() const {return m_Et_bits;}
    const std::bitset<9>& Eta_bits() const {return m_Eta_bits;}
    const std::bitset<7>& Phi_bits() const {return m_Phi_bits;}
    const std::bitset<2>& Charge_bits() const {return m_Charge_bits;}
    const std::bitset<1>& overflow_bits() const {return m_Overflow_bits;}
    std::bitset<32> as_bits() const;
 
    int Et () const {return bitSetToInt(m_Et_bits);}
    int Eta () const {return bitSetToInt(m_Eta_bits);}
    int Phi () const {return bitSetToInt(m_Phi_bits);}
    int Charge () const{return bitSetToInt(m_Charge_bits);}
      
    
  private:

    // at the time of writing the int widhs in the VHDL code were all < 16.
       
    std::bitset<GenericEtBitWidth> m_Et_bits;
    std::bitset<GenericEtaBitWidth> m_Eta_bits;
    std::bitset<GenericPhiBitWidth> m_Phi_bits;
    std::bitset<GenericMuonFlagBitWidth> m_Charge_bits;
    std::bitset<1> m_Overflow_bits;
    
  };

  using GenericTobContainer = std::vector<std::shared_ptr<GenericTob>>;

}

CLASS_DEF( GlobalSim::GenericTobContainer , 1078282199 , 1 )

std::ostream& operator << (std::ostream&, const GlobalSim::GenericTob&);

#endif
