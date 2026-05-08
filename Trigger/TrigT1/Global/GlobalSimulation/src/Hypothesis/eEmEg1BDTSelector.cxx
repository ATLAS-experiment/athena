/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmEg1BDTSelector.h"
#include "../IO/eEmEg1BDTTOB.h"

#include "../Egamma1BDT//Egamma1BDT/parameters.h"

#include <sstream>

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  eEmEg1BDTSelector::eEmEg1BDTSelector(ulong Eg1BDT_cut,
				       const std::string& Eg1BDT_op) :
    m_Eg1BDT_cutter{make_cutter(Eg1BDT_cut, Eg1BDT_op)}{
  }


  bool eEmEg1BDTSelector::select(const eEmEg1BDTTOB& tob) const {

    score_t BDT_Result;
    std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> bits = tob.eGamma1BDT_bits();
    for (uint i=0;i<eEmEg1BDTTOB::s_eGamma1BDT_width;i++){
	BDT_Result[i] = bits[i];
      }

    if(!m_Eg1BDT_cutter->cut(BDT_Result.to_float())) {return false;}

    return true;
  };


  std::string eEmEg1BDTSelector::to_string() const {
    
    auto ss = std::stringstream();
    ss << "eGamma1 BDT cutter: " << m_Eg1BDT_cutter->to_string();
    return ss.str();
  };


}
