/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InvariantMassDeltaPhiInclusive2ContainerPortsIn.h"
using namespace GlobalSim;

std::ostream&
operator<< (std::ostream& os,
	    const InvariantMassDeltaPhiInclusive2ContainerPortsIn& in_ports){

  os << "\nInvariantMassDeltaPhiInclusive2ContainerPortsIn  data:\n" <<
    "m_I_Tobs1 ["<< in_ports.m_I_Tobs1.size() << "]\n";
  
  
  std::size_t ind{0};
  for (const auto& gt_ptr : in_ports.m_I_Tobs1) {
    os  << "tob " << ind ++ <<'\n';
    if (gt_ptr) {
      os << *gt_ptr << "\n";
    }
  }
  
  
  os << "\n  m_I_Tobs2 [" << in_ports.m_I_Tobs2.size() << "]\n";
  
  
  ind = 0;
  for (const auto& gt_ptr : in_ports.m_I_Tobs2) {
    os  << "tob " << ind ++ <<'\n';
    if (gt_ptr) {
      os << *gt_ptr << "\n";
    }
  }
  
  return os;

}



