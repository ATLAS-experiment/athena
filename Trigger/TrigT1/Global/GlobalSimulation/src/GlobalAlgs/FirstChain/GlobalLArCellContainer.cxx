/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalLArCellContainer.h"

namespace GlobalSim {

  void GlobalLArCellContainer::push_back(const GlobalLArCell& theCell) {
    DataVector<GlobalLArCell>::push_back( std::make_unique<GlobalLArCell>(theCell) );
  }
 
} // namespace GlobalSim
