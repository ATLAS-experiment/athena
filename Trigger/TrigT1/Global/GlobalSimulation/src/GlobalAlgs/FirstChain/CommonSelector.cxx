/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./CommonSelector.h"

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  CommonSelector::CommonSelector(const std::string& et_low,
					 const std::string& et_high,
					 const std::string& eta_low,
					 const std::string& eta_high,
					 const std::string& phi_low,
					 const std::string& phi_high) :
    m_et_low{std::bitset<ICommonTOB::s_et_width>(et_low).to_ulong()},
    m_eta_low{std::bitset<ICommonTOB::s_eta_width>(eta_low).to_ulong()},
    m_phi_low{std::bitset<ICommonTOB::s_phi_width>(phi_low).to_ulong()} {

    if(et_high == "inf") {
      m_et_high = ULONG_MAX;
    } else {
      m_et_high = std::bitset<ICommonTOB::s_et_width>(et_high).to_ulong();
    }

    if(eta_high == "inf") {
      m_eta_high = ULONG_MAX;
    } else {
      m_eta_high = std::bitset<ICommonTOB::s_eta_width>(eta_high).to_ulong();
    }

    if(eta_high == "inf") {
      m_phi_high = std::bitset<ICommonTOB::s_phi_width>(phi_high).to_ulong();
    } else {			    
      m_phi_high = std::bitset<ICommonTOB::s_phi_width>(phi_high).to_ulong();
    }
  }
			    

  bool CommonSelector::select(const ICommonTOB& tob) const {
    {
      auto et = tob.et_bits().to_ulong();
      if (et < m_et_low  or et >= m_et_high) {return false;}
    }

    {
      auto eta = tob.eta_bits().to_ulong();
      if (eta < m_eta_low  or eta >= m_eta_high) {return false;}
    }

    {
      auto phi = tob.phi_bits().to_ulong();
      if (phi < m_phi_low  or phi >= m_phi_high) {return false;}
    }
    
    return true;
  };
  


}
