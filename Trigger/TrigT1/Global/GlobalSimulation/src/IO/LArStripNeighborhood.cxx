/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStripNeighborhood.h"

#include <sstream>

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

  std::string LArStripNeighborhood::to_string() const {
    std::stringstream ss;
    ss << "LArStripNeighborhood: roi coords ("
       << roiCoords().first << ','  << roiCoords().second << ") cell coords ("
       << cellCoords().first << ','  << cellCoords().second
       << ") max_cell_pos " << maxCellIndex() << '\n';
    
    ss << "phi low: " << " [" << phi_low().size() <<"]\n";
    for(const auto& sd : phi_low()) { ss << sd << '\n';}
    
    ss << '\n';
    
    ss << "phi center: " << " [" << phi_center().size() <<"]\n";
    for(const auto& sd : phi_center()) { ss << sd << '\n';}
    
    ss << '\n';
    
    ss << "phi high: " << " [" << phi_high().size() <<"]\n";
    for(const auto& sd : phi_high()) { ss << sd << '\n';}
    
    ss << '\n';
    return ss.str();
  }
}

std::ostream&
operator<< (std::ostream& os,
	    const GlobalSim::LArStripNeighborhood& n) {
  os << n.to_string();
  return os;
}

    
    
    
  
