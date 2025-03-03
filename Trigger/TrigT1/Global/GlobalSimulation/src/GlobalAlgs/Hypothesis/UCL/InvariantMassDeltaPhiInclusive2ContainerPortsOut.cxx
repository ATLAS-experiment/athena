/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InvariantMassDeltaPhiInclusive2ContainerPortsOut.h"
using namespace GlobalSim;

std::ostream&
operator<< (std::ostream& os,
	    const GlobalSim::InvariantMassDeltaPhiInclusive2ContainerPortsOut& ports) {
  os << "\nInvariantMassDeltaPhiInclusive2ContainerPortsOut ports data:\n" <<
    "m_Results ["<< ports.m_Results->size() << "]\n";

  os << (*ports.m_Results) << '\n';
  
  return os;

}
