/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_STRIPDATA_H
#define GLOBALSIM_STRIPDATA_H

#include <ostream>

/* Struct to carry et data for Strips in the neighbourhood of a RoI */

namespace GlobalSim {

  struct StripData{
    StripData() = default;
    double m_eta{0.};
    double m_phi{0.};
    double m_et{0.};
  };
  
}

std::ostream & operator<<(std::ostream&, const GlobalSim::StripData&);

#endif




