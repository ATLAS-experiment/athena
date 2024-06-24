/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  TOBArray.cxx
//  TopoCore
//  Created by Joerg Stelzer on 11/17/12.
//
// Adapted for GlobalL1TopoSim


#include "GenericTOBArray.h"
#include "L1TopoEvent/CompositeTOB.h"

#include <algorithm>

namespace GlobalSim {

  void
  GenericTOBArray::push_back(const TCS::CompositeTOB& tob) {
    m_data.push_back(TCS::CompositeTOB::createOnHeap(tob));
  }

  void
  GenericTOBArray::sort(sort_fnc fnc) {
    std::sort(m_data.begin(), m_data.end(), fnc);
  }

  void GenericTOBArray::print(std::ostream& os) const {
    for(const_iterator tob = m_data.begin(); tob != m_data.end(); ++tob) {
      if( tob!=begin() ) os << std::endl;
      os << **tob;
    }
  }
}

std::ostream& operator << (std::ostream& os,
			   const GlobalSim::GenericTOBArray& gta) {
  os << "GenericTOB. name: " << gta.name() << ":\n";
  gta.print(os);
  os << '\n';
  return os;
}



  
