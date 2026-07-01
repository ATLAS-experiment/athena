/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmSelector.h"
#include "../IO/eEmTOB.h"
#include <sstream>

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  eEmSelector::eEmSelector(ulong rhad_cut,
			   const std::string& rhad_op,
			   ulong reta_cut,
			   const std::string& reta_op,
			   ulong wstot_cut,
			   const std::string& wstot_op) :
    m_rhad_cutter{make_cutter(rhad_cut, rhad_op)},
    m_reta_cutter{make_cutter(reta_cut, reta_op)},
    m_wstot_cutter{make_cutter(wstot_cut, wstot_op)}{
  }


  bool eEmSelector::select(const eEmTOB& tob) const {

    if(!m_rhad_cutter->cut(tob.RHad_bits().to_ulong())) {return false;}
    if(!m_reta_cutter->cut(tob.REta_bits().to_ulong())) {return false;}
    if(!m_wstot_cutter->cut(tob.WsTot_bits().to_ulong())) {return false;}

    return true;
  };


  std::string eEmSelector::to_string() const {
    
    auto ss = std::stringstream();
    ss << "rhad cutter: " << m_rhad_cutter->to_string() << ' '
       << "reta cutter: " << m_reta_cutter->to_string() << ' '
       << "wstot cutter: " << m_wstot_cutter->to_string();
    return ss.str();
  };


}
