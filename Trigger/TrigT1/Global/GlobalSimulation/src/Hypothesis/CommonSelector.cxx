/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./CommonSelector.h"
#include <sstream>

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  CommonSelector::CommonSelector(const std::string& et_low,
				 const std::string& et_high,
				 const std::string& eta_low,
				 const std::string& eta_high,
				 const std::string& phi_low,
				 const std::string& phi_high):
    m_et_low{std::stoul(et_low)},
    m_eta_low{std::stoul(eta_low)},
    m_phi_low{std::stoul(phi_low)} {
    //
    auto unsignedLong = [](const std::string & txt)->unsigned long{
      if (txt == "inf") return ULONG_MAX;
      return std::stoul(txt);
    };
    //
    m_et_high = unsignedLong(et_high);
    m_eta_high = unsignedLong(eta_high);
    m_phi_high = unsignedLong(phi_high);
  }
			    

  bool CommonSelector::select(const CommonTOB& tob) const {
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
  
  std::string CommonSelector::to_string() const {
    
    auto ss = std::stringstream();
    ss << "et_low: " << m_et_low <<' '
       << "et_high: " << m_et_high <<' '
       << "eta_low: " << m_eta_low <<' '
       << "eta_high: " << m_eta_high <<' '
       << "phi_low: " << m_phi_low <<' '
       << "phi_high: " << m_phi_high;
    
    return ss.str();
  };

}
