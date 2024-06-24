/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ISOCUT_H
#define GLOBALSIM_ISOCUT_H

// Isolation cut functions used by various L1Topo Algorithms

#include <string>

namespace GlobalSim {
  bool isocut(const std::string& threshold, const unsigned int bit);
  bool isocut(const unsigned int threshold, const unsigned int bit);
}
  

#endif
