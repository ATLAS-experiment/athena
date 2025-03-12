/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmTob.h"

namespace GlobalSim {

  eEmTob::eEmTob(const GepAlgoHypothesisPortsIn& ports_in) {

    const auto& w_tob = ports_in.m_I_eEmTobs;

    std::size_t i;
    std::size_t j;

    for (i = 0; i != AlgoConstants::eFexEtBitWidth; ++i) {
      m_Et[i] = (*w_tob)[i];
    }

    for (i = 16, j=0;
	 i != 16+ AlgoConstants::eFexPhiBitWidth;
	 ++i, ++j) {
      m_Phi[j] = (*w_tob)[i];
    }

    for (i = 32, j=0;
	 i != 32+ AlgoConstants::eFexEtaBitWidth;
	 ++i, ++j) {
      m_Eta[j] = (*w_tob)[i];
    }


    for (i = 48, j=0;
	 i != 48+ AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	m_REta[j] = (*w_tob)[i];
      }
    

    for (i = 48+ AlgoConstants::eFexDiscriminantBitWidth, j=0;
	 i != 48 + 2*AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	m_RHad[j] = (*w_tob)[i];
      }

    
    for (i = 48+ 2*AlgoConstants::eFexDiscriminantBitWidth, j=0;
	 i != 48 + 3*AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	m_WsTot[j] = (*w_tob)[i];
      }

    m_Overflow[0] = (*w_tob)[63];

  }


  std::bitset<32>  eEmTob::asBits() const  {
    auto result = std::bitset<32>();
    std::size_t r_ptr{0};
    for (std::size_t s_ptr=0; s_ptr < m_Et.size(); ++s_ptr, ++r_ptr) {
      if(m_Et.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < m_REta.size(); ++s_ptr, ++r_ptr) {
      if(m_REta.test(s_ptr)) {result.set(r_ptr);}
    }
      
    for (std::size_t s_ptr=0; s_ptr < m_RHad.size(); ++s_ptr, ++r_ptr) {
      if(m_RHad.test(s_ptr)) {result.set(r_ptr);}
    }
            
    for (std::size_t s_ptr=0; s_ptr < m_WsTot.size(); ++s_ptr, ++r_ptr) {
      if(m_WsTot.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < m_Eta.size(); ++s_ptr, ++r_ptr) {
      if(m_Eta.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < m_Phi.size(); ++s_ptr, ++r_ptr) {
      if(m_Phi.test(s_ptr)) {result.set(r_ptr);}
    }

    return result;
  }

  using eEmTobPtr = std::shared_ptr<eEmTob>;


  std::ostream& operator << (std::ostream& os, const GlobalSim::eEmTob& tob) {

    os << "GlobalSim::eEmTob\n"
       << "Et: " << std::hex << tob.Et_bits().to_ulong() << '\n'
       << "REta: " << std::hex <<  tob.REta_bits().to_ulong() << '\n'
       << "RHad: " << std::hex<< tob.RHad_bits().to_ulong() << '\n'
       << "WsTot: " << std::hex<< tob.WsTot_bits().to_ulong() << '\n'
       << "Eta: " << std::hex<< tob.Eta_bits().to_ulong() << '\n'
       << "Phi: " << std::hex<< tob.Phi_bits().to_ulong() << '\n';
    return os;
  }
}



