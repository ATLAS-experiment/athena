/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmSortSelectCountContainerPortsOut.h"
using namespace GlobalSim;

std::ostream&
operator<< (std::ostream& os,
	    const GlobalSim::eEmSortSelectCountContainerPortsOut & out) {
  os << "\neEmSortSelectCountContainerPortsOut port_out data:\n" <<
    "m_Ο_eEmGenTob ["<< out.m_O_eEmGenTob.size() << "]\n";


  std::size_t ind{0};
  for (const auto& gt_ptr :  out.m_O_eEmGenTob) {
    os  << "tob " << ind ++ <<'\n';
    if (gt_ptr) {
      os << *gt_ptr << "\n";
    }
  }
  
  
  os << '\n';
  os << "O_Multiplicity " << *(out.m_O_Multiplicity) << '\n';
  
  return os;

}



