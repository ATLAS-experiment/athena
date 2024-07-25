/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_STRIPNEIGHBORHOOD_H
#define GLOBALSIM_STRIPNEIGHBORHOOD_H

#include "StripData.h"

#include <vector>
#include <ostream>

namespace GlobalSim {
  class LArStripNeighborhood;
}


std::ostream&
operator<< (std::ostream&, const GlobalSim::LArStripNeighborhood&);

namespace GlobalSim {
  using StructuredNeighborhood=std::vector<std::vector<GlobalSim::StripData>>;
  using Coords = std::pair<double, double>;

  class LArStripNeighborhood {
  public:
    friend std::ostream& ::operator<<(std::ostream&, const LArStripNeighborhood&);
    
    LArStripNeighborhood(const std::vector<StripData>&,
			 const Coords& roiCoords,
			 const Coords& cellCoords);
    
    StructuredNeighborhood neighborhood() const;
  private:
    std::vector<StripData> m_data;
    Coords m_roiCoords{0., 0.};
    Coords m_cellCoords{0., 0.};
  };
}

#endif
