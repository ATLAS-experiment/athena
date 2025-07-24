/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_LARSTRIPNEIGHBORHOOD_H
#define GLOBALSIM_LARSTRIPNEIGHBORHOOD_H

#include "StripData.h"

#include <vector>
#include <utility> //std::pair
#include <iosfwd>

namespace GlobalSim {
  class LArStripNeighborhood;
}


std::ostream&
operator<< (std::ostream&, const GlobalSim::LArStripNeighborhood&);

namespace GlobalSim {

  using StripDataVector = std::vector<StripData>;
  
  using Coords = std::pair<double, double>;

  class LArStripNeighborhood {
  public:
 
    
    LArStripNeighborhood(const StripDataVector& phi_low,
			 const StripDataVector& phi_center,
			 const StripDataVector& phi_high,
			 const Coords& roiCoords,
			 const Coords& cellCoords,
			 std::size_t max_cell_pos);
    
    const StripDataVector& phi_low() const {return m_phi_low;}
    const StripDataVector& phi_center() const {return m_phi_center;}
    const StripDataVector& phi_high() const {return m_phi_high;}

    std::size_t maxCellIndex() const {return m_max_cell_pos;}

    const Coords& roiCoords() const {return m_roiCoords;}
    const Coords& cellCoords() const {return m_cellCoords;}

  private:
    friend std::ostream& ::operator<<(std::ostream&,
				      const LArStripNeighborhood&);
    StripDataVector m_phi_low;
    StripDataVector m_phi_center;
    StripDataVector m_phi_high;
    Coords m_roiCoords{0., 0.};
    // coords of cell in RoI with maximum energy
    Coords m_cellCoords{0., 0.};
    std::size_t m_max_cell_pos{0};
  };
}

#endif
