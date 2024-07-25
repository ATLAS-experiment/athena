/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStripNeighborhood.h"

#include <algorithm>

namespace GlobalSim {
  
  LArStripNeighborhood::LArStripNeighborhood(const std::vector<StripData>& d,
					     const Coords& roi,
					     const Coords& cell) :
    m_data{d}, m_roiCoords{roi}, m_cellCoords{cell} {
    auto phi_cmp = [](const auto& l, const auto& r) {
      return l.m_phi < r.m_phi;
    };

    std::sort(m_data.begin(),
	      m_data.end(),
	      phi_cmp);
  }


  StructuredNeighborhood LArStripNeighborhood::neighborhood() const {

    auto by_eta = [](const auto& l, const auto& r) {
      return l.m_eta < r.m_eta;
    };

    auto nbhd = StructuredNeighborhood();
    for (std::size_t i{0}; i < 3; ++i) {
      
      std::vector<StripData> sdVec(m_data.cbegin() + i*17,
				   m_data.cbegin() + (i+1)*17);

      std::sort(sdVec.begin(),
		sdVec.end(),
		by_eta
		);

      nbhd.push_back(sdVec);
    }

    return nbhd;
  }
}

std::ostream&
operator<< (std::ostream& os, const GlobalSim::LArStripNeighborhood& n) {

  auto neighborhood = n.neighborhood();

  os << "LArStripNeighborhood: roi coords ("
     << n.m_roiCoords.first << ','  << n.m_roiCoords.second << ") cell coords ("
     << n.m_cellCoords.first << ','  << n.m_cellCoords.second << ")\n";

  unsigned int iphi{0};
  for (const auto& eta_vec : neighborhood) {
    os << "phi row: " << iphi++ << " [" << eta_vec.size() <<"]\n";
    for(const auto& sd : eta_vec) { os << sd << '\n';}
  }
  
  return os;
}

    
    
    
  
