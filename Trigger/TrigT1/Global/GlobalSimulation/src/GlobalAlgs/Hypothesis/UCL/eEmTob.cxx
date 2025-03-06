/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmTob.h"

namespace GlobalSim {

  std::bitset<32>  eEmTob::asBits() const  {
    auto result = std::bitset<32>();
    std::size_t r_ptr{0};
    for (std::size_t s_ptr=0; s_ptr < Et.size(); ++s_ptr, ++r_ptr) {
      if(Et.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < REta.size(); ++s_ptr, ++r_ptr) {
      if(REta.test(s_ptr)) {result.set(r_ptr);}
    }
      
    for (std::size_t s_ptr=0; s_ptr < RHad.size(); ++s_ptr, ++r_ptr) {
      if(RHad.test(s_ptr)) {result.set(r_ptr);}
    }
            
    for (std::size_t s_ptr=0; s_ptr < WsTot.size(); ++s_ptr, ++r_ptr) {
      if(WsTot.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < Eta.size(); ++s_ptr, ++r_ptr) {
      if(Eta.test(s_ptr)) {result.set(r_ptr);}
    }

    for (std::size_t s_ptr=0; s_ptr < Phi.size(); ++s_ptr, ++r_ptr) {
      if(Phi.test(s_ptr)) {result.set(r_ptr);}
    }

    return result;
  }

  using eEmTobPtr = std::shared_ptr<eEmTob>;
}

std::ostream& operator << (std::ostream& os, const GlobalSim::eEmTob& tob) {

  os << "GlobalSim::eEmTob\n"
     << "Et: " << tob.Et << '\n'
     << "REta: " << tob.REta << '\n'
     << "RHad: " << tob.RHad << '\n'
     << "WsTot: " << tob.WsTot << '\n'
     << "Eta: " << tob.Eta << '\n'
     << "Phi: " << tob.Phi << '\n';
  return os;
}



