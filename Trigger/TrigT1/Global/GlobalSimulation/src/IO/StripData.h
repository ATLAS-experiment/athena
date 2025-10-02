/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_STRIPDATA_H
#define GLOBALSIM_STRIPDATA_H

#include <iosfwd>

/* Struct to carry et data for LAr strips consumned by various GlobalSim
* Algorithms
*/

namespace GlobalSim {

  struct StripData{
    StripData() = default;
    StripData(double eta, double phi, double e):
      m_eta{eta}, m_phi{phi}, m_e{e}{}
    double m_eta{0.};
    double m_phi{0.};
    double m_e{0.};
  };
  
}

std::ostream& operator<<(std::ostream&, const GlobalSim::StripData&);

#endif




