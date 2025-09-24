/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CommonTOB.h"
#include <iostream>


namespace GlobalSim::IOBitwise {

  CommonTOB::CommonTOB(const xAOD::eFexEMRoI& eFexTOB): m_et_bits(eFexTOB.et()),
							m_eta_bits(eFexTOB.iEtaTopo()),
							m_phi_bits(eFexTOB.iPhiTopo()){}

  CommonTOB::CommonTOB(const GlobalSim::IOBitwise::ICommonTOB& CommonTOB): m_et_bits(CommonTOB.et_bits()),
								m_eta_bits(CommonTOB.eta_bits()),
								m_phi_bits(CommonTOB.phi_bits()){}

  std::bitset<CommonTOB::s_et_width> CommonTOB::et_bits() const {
    return m_et_bits;
  }

  std::bitset<CommonTOB::s_eta_width> CommonTOB::eta_bits() const {
    return m_eta_bits;
  }

  std::bitset<CommonTOB::s_phi_width> CommonTOB::phi_bits() const {
    return m_phi_bits;
  }
}
