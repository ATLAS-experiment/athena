/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "gFexRhoTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  //Currently assuming that the gFex Rho is stored in the et bits (including the scale) in the TOB
  gFexRhoTOB::gFexRhoTOB(const xAOD::gFexJetRoI& gFexRhoTOB) :
    m_rho_bits(gFexRhoTOB.et()){}


  gFexRhoTOB::gFexRhoTOB(const GlobalSim::IOBitwise::gFexRhoTOB& tob) :
    m_rho_bits(tob.rho_bits()){}  

  std::bitset<gFexRhoTOB::s_rho_width> gFexRhoTOB::rho_bits() const {
    return m_rho_bits;
  }
  
  std::string gFexRhoTOB::to_string() const {
    std::stringstream ss;
    
    ss << "TODO";
	
    return ss.str();
  }
}
