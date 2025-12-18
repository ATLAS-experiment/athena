/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmEg1BDTTOB.h"
#include <sstream>


namespace GlobalSim::IOBitwise {

  eEmEg1BDTTOB::eEmEg1BDTTOB(const xAOD::eFexEMRoI& eFexTOB,
			     std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits) :
    eEmTOB(eFexTOB),
    m_eGamma1BDT_bits(eGamma1BDT_bits){}

  eEmEg1BDTTOB::eEmEg1BDTTOB(const eEmTOB& tob,
			     std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits) :
    eEmTOB(tob),
    m_eGamma1BDT_bits(eGamma1BDT_bits){}

  std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eEmEg1BDTTOB::eGamma1BDT_bits() const {
    return m_eGamma1BDT_bits;
  }
  
  std::string eEmEg1BDTTOB::to_string() const {

    auto ss = std::stringstream();
    ss <<  eEmTOB::to_string() << '\n'
       << "eEmEg1BDTTOB: m_eGamma1BDT_bits " << m_eGamma1BDT_bits << '\n';
    return ss.str();
  }
  
}
