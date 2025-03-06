/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GenericTob.h"

namespace GlobalSim {


    
  GenericTob::GenericTob(const eEmTobPtr& in_tob) {
      m_Et = bitSetToInt(in_tob->Et);

      auto etaBin = (in_tob->Eta).to_ulong();
      m_Eta = etaBin < 200 ? -100 + etaBin : 100;
      
      m_Phi = static_cast<unsigned int>(bitSetToInt(in_tob->Phi));
    
      m_Charge = 0;
      m_Overflow = 0;

      setBits(in_tob);
    }	      

  void GenericTob::setBits(const eEmTobPtr& in_tob) {
      
      std::size_t r_ptr{0};
      const auto& et = in_tob->Et;
      for (std::size_t s_ptr=0; s_ptr < et.size(); ++s_ptr, ++r_ptr) {
	if(et.test(s_ptr)) {m_bits.set(r_ptr);}
      }

      const auto& reta = in_tob->REta;
      for (std::size_t s_ptr=0; s_ptr < reta.size(); ++s_ptr, ++r_ptr) {
	if(reta.test(s_ptr)) {m_bits.set(r_ptr);}
      }

      const auto& rhad = in_tob->RHad;
      for (std::size_t s_ptr=0; s_ptr < rhad.size(); ++s_ptr, ++r_ptr) {
	if(rhad.test(s_ptr)) {m_bits.set(r_ptr);}
      }
      
      const auto& wstot = in_tob->WsTot;
      for (std::size_t s_ptr=0; s_ptr < wstot.size(); ++s_ptr, ++r_ptr) {
	if(wstot.test(s_ptr)) {m_bits.set(r_ptr);}
      }

      const auto& eta = in_tob->Eta;
      for (std::size_t s_ptr=0; s_ptr < eta.size(); ++s_ptr, ++r_ptr) {
	if(eta.test(s_ptr)) {m_bits.set(r_ptr);}
      }

      const auto& phi = in_tob->Phi;
      for (std::size_t s_ptr=0; s_ptr < phi.size(); ++s_ptr, ++r_ptr) {
	if(phi.test(s_ptr)) {m_bits.set(r_ptr);}
      }
    }
}

std::ostream& operator << (std::ostream& os, const GlobalSim::GenericTob& tob) {
  
  // using bitsetToInt = GlobalSim::bitSetToInt;
  
  os << "GlobalSim::GenericTob\n"
     << "Et: " << tob.Et() << ' ' << std::bitset<13>{tob.Et()} << '\n'
     << "Eta: " << tob.Eta() << '\n'
     << "Phi: " << tob.Phi() << '\n'
     << "Charge: " << tob.Charge() << '\n'
     << "overflow: " << tob.overflow() << '\n';
  return os;
}
