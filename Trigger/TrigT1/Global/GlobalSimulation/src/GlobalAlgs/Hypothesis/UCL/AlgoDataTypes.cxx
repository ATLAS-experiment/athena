/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgoDataTypes.h"

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

std::ostream& operator << (std::ostream& os, const GlobalSim::GenericTob& tob) {

  os << "GlobalSim::GenericTob\n"
     << "Et: " << tob.m_Et << '\n'
     << "Eta: " << tob.m_Eta << '\n'
     << "Phi: " << tob.m_Phi << '\n'
     << "Charge: " << tob.m_Charge << '\n'
     << "overflow: " << tob.m_Overflow << '\n';
  return os;
}

