/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "gFexRhoTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  gFexRhoTOB::gFexRhoTOB(const xAOD::gFexJetRoI& gFexRhoTOB) :
    m_rho_bits(gFexRhoTOB.gFexTobEt()),
    m_rho_scale(gFexRhoTOB.tobEtScale()){}

  gFexRhoTOB::gFexRhoTOB(const GlobalSim::IOBitwise::gFexRhoTOB& tob) :
    m_rho_bits(tob.rho_bits()),
    m_rho_scale(tob.rho_scale()){}  
  
  gFexRhoTOB::gFexRhoTOB(const uint& rho_bits, const uint& rho_scale) :
    m_rho_bits(rho_bits),
    m_rho_scale(rho_scale){}
  
  std::bitset<gFexRhoTOB::s_rho_width> gFexRhoTOB::rho_bits() const {
    return m_rho_bits;
  }

  std::bitset<gFexRhoTOB::s_rho_scale> gFexRhoTOB::rho_scale() const {
    return m_rho_scale;
  }

  std::string gFexRhoTOB::to_string() const {
    std::stringstream ss;
    
    ss << "rho_bits "<< rho_bits() << " rho_scale "<< rho_scale();
	
    return ss.str();
  }
}
