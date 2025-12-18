/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmEg1eRatioTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  eEmEg1eRatioTOB::eEmEg1eRatioTOB(const xAOD::eFexEMRoI& eFexTOB,
				   std::bitset<IeEmEg1eRatioTOB::s_eGamma1eRatio_width> eGamma1eRatio_bits) :
    eEmTOB(eFexTOB),
    m_eGamma1eRatio_bits(eGamma1eRatio_bits){}

  eEmEg1eRatioTOB::eEmEg1eRatioTOB(const eEmTOB& tob,
				   std::bitset<IeEmEg1eRatioTOB::s_eGamma1eRatio_width> eGamma1eRatio_bits) :
    eEmTOB(tob),
    m_eGamma1eRatio_bits(eGamma1eRatio_bits){}
  
  std::bitset<IeEmEg1eRatioTOB::s_eGamma1eRatio_width> eEmEg1eRatioTOB::eGamma1eRatio_bits() const {
    return m_eGamma1eRatio_bits;
  }

  std::string eEmEg1eRatioTOB::to_string() const {

    auto ss = std::stringstream();
    ss <<  eEmTOB::to_string() << '\n';
    ss << m_eGamma1eRatio_bits;
    return ss.str();
    

  }

}
