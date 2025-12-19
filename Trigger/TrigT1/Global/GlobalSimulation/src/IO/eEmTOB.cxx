/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  eEmTOB::eEmTOB(const xAOD::eFexEMRoI& eFexTOB) :
    CommonTOB(eFexTOB),
    m_RHad_bits(eFexTOB.RhadThresholds()),
    m_WsTot_bits(eFexTOB.WstotThresholds()),
    m_REta_bits(eFexTOB.RetaThresholds()),
    m_Seed_bits(eFexTOB.seed()),
    m_UpNotDown_bit(eFexTOB.UpNotDown()),
    m_SeedIsMax_bit(eFexTOB.seedMax()){}

  eEmTOB::eEmTOB(const GlobalSim::IOBitwise::eEmTOB& tob) :
    CommonTOB(tob),
    m_RHad_bits(tob.RHad_bits()),
    m_WsTot_bits(tob.WsTot_bits()),
    m_REta_bits(tob.REta_bits()),
    m_Seed_bits(tob.Seed_bits()),
    m_UpNotDown_bit(tob.UpNotDown_bit()),
    m_SeedIsMax_bit(tob.SeedIsMax_bit()){}

  
  eEmTOB::eEmTOB(const GlobalSim::IOBitwise::CommonTOB& commonTOB,
		 const std::bitset<eEmTOB::s_RHad_width>& RHad_bits,
		 const std::bitset<eEmTOB::s_WsTot_width>& WsTot_bits, 
		 const std::bitset<eEmTOB::s_REta_width>& REta_bits,
		 const std::bitset<eEmTOB::s_Seed_width>& Seed_bits,
		 const std::bitset<eEmTOB::s_UpNotDown_width>& UpNotDown_bit,
		 const std::bitset<eEmTOB::s_SeedIsMax_width>& SeedIsMax_bit
		 ) :
    CommonTOB(commonTOB),
    m_RHad_bits(RHad_bits),
    m_WsTot_bits(WsTot_bits),
    m_REta_bits(REta_bits),
    m_Seed_bits(Seed_bits),
    m_UpNotDown_bit(UpNotDown_bit),
    m_SeedIsMax_bit(SeedIsMax_bit){
  }


  
  const std::bitset<eEmTOB::s_RHad_width>& eEmTOB::RHad_bits() const {
    return m_RHad_bits;
  }

  const std::bitset<eEmTOB::s_WsTot_width>& eEmTOB::WsTot_bits() const {
    return m_WsTot_bits;
  }
  
  const std::bitset<eEmTOB::s_REta_width>& eEmTOB::REta_bits() const {
    return m_REta_bits;
  }

  const std::bitset<eEmTOB::s_Seed_width>& eEmTOB::Seed_bits() const {
    return m_Seed_bits;
  }
  
  const std::bitset<eEmTOB::s_UpNotDown_width>& eEmTOB::UpNotDown_bit() const {
    return m_UpNotDown_bit;
  }
  
  const std::bitset<eEmTOB::s_SeedIsMax_width>& eEmTOB::SeedIsMax_bit() const {
    return m_SeedIsMax_bit;
  }

  std::string eEmTOB::to_string() const {

    std::stringstream ss;
    
    ss << '\n'
       << CommonTOB::to_string()
       << " RHad " << RHad_bits() << " (" <<   RHad_bits().to_ulong() << ")"
       << " REta " << REta_bits() << " (" <<   REta_bits().to_ulong() << ")"
       << " WsTot " << WsTot_bits() << " (" <<   WsTot_bits().to_ulong() << ")";
    return ss.str();
  }

}
