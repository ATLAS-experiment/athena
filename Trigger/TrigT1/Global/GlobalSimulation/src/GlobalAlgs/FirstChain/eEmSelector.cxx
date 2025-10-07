/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmSelector.h"

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  eEmSelector::eEmSelector(const std::string& rhad_low,
			   const std::string& rhad_high,
			   const std::string& reta_low,
			   const std::string& reta_high,
			   const std::string& wstot_low,
			   const std::string& wstot_high) :
    m_rhad_low{std::bitset<IeEmTOB::s_et_width>(rhad_low).to_ulong()},
    m_reta_low{std::bitset<IeEmTOB::s_eta_width>(reta_low).to_ulong()},
    m_wstot_low{std::bitset<IeEmTOB::s_phi_width>(wstot_low).to_ulong()}{

    if (rhad_high == "inf") {
      m_rhad_high = ULONG_MAX;
    } else {
      m_rhad_high = std::bitset<IeEmTOB::s_et_width>(rhad_high).to_ulong();
    }

    if (reta_high == "inf") {
      m_reta_high = ULONG_MAX;
    } else {
      m_reta_high = std::bitset<IeEmTOB::s_eta_width>(reta_high).to_ulong();
    }

    if (wstot_high == "inf") {
      m_wstot_high = ULONG_MAX;
    } else {
      m_wstot_high = std::bitset<IeEmTOB::s_phi_width>(wstot_high).to_ulong();
    }

  }

  bool eEmSelector::select(const IeEmTOB& tob) const {
    {
      auto rhad = tob.RHad_bits().to_ulong();
      if (rhad < m_rhad_low  or rhad >= m_rhad_high) {return false;}
    }

    {
      auto reta = tob.REta_bits().to_ulong();
      if (reta < m_reta_low  or reta >= m_reta_high) {return false;}
    }

    {
      auto wstot = tob.WsTot_bits().to_ulong();
      if (wstot < m_wstot_low  or wstot >= m_wstot_high) {return false;}
    }
    
    return true;
  };
  


}
