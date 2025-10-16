/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmEg1BDTTOB.h"

namespace GlobalSim::IOBitwise {

  eEmEg1BDTTOB::eEmEg1BDTTOB(const xAOD::eFexEMRoI& eFexTOB,
			     std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits) : eEmTOB(eFexTOB),
											       m_eGamma1BDT_bits(eGamma1BDT_bits){}

  eEmEg1BDTTOB::eEmEg1BDTTOB(const IeEmTOB& IeEmTOB,
			     std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits) : eEmTOB(IeEmTOB),
											       m_eGamma1BDT_bits(eGamma1BDT_bits){}

  std::bitset<IeEmEg1BDTTOB::s_eGamma1BDT_width> eEmEg1BDTTOB::eGamma1BDT_bits() const {
    return m_eGamma1BDT_bits;
  }
  
  std::string eEmEg1BDTTOB::to_string() const {
    return eEmTOB::to_string()  +
      " specifics of eEmEg1BDTTOB not yet implemented";

  }

  
}
