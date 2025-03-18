/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GenericTob.h"
#include "AlgoDataTypes.h"

#include <sstream>

namespace GlobalSim {
    
  GenericTob::GenericTob(const eEmTobPtr& in_tob) {

    {
      const auto& in = in_tob->Et_bits();
      auto sz = in.size();
      for (auto i = 0U; i != sz; ++i) {m_Et_bits[i] = in[i];}
    }

    
    {
      const auto& in = in_tob->Eta_bits();
      
      // vhdl etaL to_signed(unsigned(-100 + in, GenericEtaBitWidth)
      int val = (-0x64 + bitSetToInt(in));

      bool neg{val < 0};
      m_Eta_bits = std::abs(val);
      if (neg) {m_Eta_bits = m_Eta_bits.flip().to_ulong()+1;}
    }

    {
      const auto& in = in_tob->Phi_bits();
      m_Phi_bits = 2*(bitSetToInt(in)+2);
    }

  }

  GenericTob::GenericTob(const std::string& bit_string) {
    if(bit_string.size() != GenericTobWidth) {
      std::stringstream ss;
      ss << "GenericTob ctr expected " << GenericTobWidth
	 << " bits, received " << bit_string.size();
      throw std::out_of_range(ss.str());
    }

    auto bitstream = std::stringstream(bit_string);
    bitstream >> m_Et_bits;
    bitstream >> m_Eta_bits;
    bitstream >> m_Phi_bits;
    bitstream >> m_Charge_bits;

  }

  std::bitset<32> GenericTob::as_bits() const {

    auto result = std::bitset<32>();
    
    std::size_t begin = 0;
    std::size_t end = GenericEtBitWidth;;
    

    for(std::size_t i = begin; i != end; ++i) {
      result[i] = m_Et_bits[i];
    }

    begin = end;
    end = begin + GenericEtaBitWidth;

    for(std::size_t i = 0; i != GenericEtaBitWidth; ++i) {
      result[begin+i] = m_Eta_bits[i];
    }

    begin = end;
    end = begin + GenericPhiBitWidth;
    
    for(std::size_t i = 0; i != GenericPhiBitWidth; ++i) {
      result[begin+i] = m_Phi_bits[i];
    }

    
    
    return result;

  }

}

std::ostream& operator << (std::ostream& os, const GlobalSim::GenericTob& tob) {
  
  
  os << "GlobalSim::GenericTob\n"
     << "Et: " << tob.Et_bits() << '\n'
     << "Eta: " << tob.Eta_bits() << '\n'
     << "Phi: " << tob.Phi_bits() << '\n'
     << "Charge: " << tob.Charge_bits() << '\n'
     << "overflow: " << tob.overflow_bits() << '\n'
     << "bits: " << tob.as_bits() << '\n';
  return os;
}
