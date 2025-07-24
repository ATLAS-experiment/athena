/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStripNeighborhood.h"

#include <ostream>

namespace GlobalSim {
  
  LArStripNeighborhood::LArStripNeighborhood(const StripDataVector& phi_low,
					     const StripDataVector& phi_center,
					     const StripDataVector& phi_high,
					     const Coords& roi,
					     const Coords& cell,
					     std::size_t max_cell_pos) :
    m_phi_low{phi_low},
    m_phi_center{phi_center},
    m_phi_high{phi_high},
    m_roiCoords{roi},
    m_cellCoords{cell},
    m_max_cell_pos{max_cell_pos}{
  }
}

std::ostream&
operator<< (std::ostream& os, const GlobalSim::LArStripNeighborhood& n) {
  
  os << "LArStripNeighborhood: roi coords ("
     << n.m_roiCoords.first << ','  << n.m_roiCoords.second << ") cell coords ("
     << n.m_cellCoords.first << ','  << n.m_cellCoords.second
     << ") max_cell_pos " << n.m_max_cell_pos << '\n';
  
  os << "phi low: " << " [" << n.m_phi_low.size() <<"]\n";
  for(const auto& sd : n.m_phi_low) { os << sd << '\n';}
  
  os << '\n';
  
  os << "phi center: " << " [" << n.m_phi_center.size() <<"]\n";
  for(const auto& sd : n.m_phi_center) { os << sd << '\n';}

  os << '\n';
  
  os << "phi high: " << " [" << n.m_phi_high.size() <<"]\n";
  for(const auto& sd : n.m_phi_high) { os << sd << '\n';}
  
  os << '\n';
  return os;
}

    
    
    
  
