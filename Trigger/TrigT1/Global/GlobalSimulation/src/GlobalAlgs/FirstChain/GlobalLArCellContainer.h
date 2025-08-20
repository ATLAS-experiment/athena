/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#ifndef GLOBALSIM_GLOBALLARCELLCONTAINER_H
#define GLOBALSIM_GLOBALLARCELLCONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "GlobalLArCell.h"

namespace GlobalSim {

  class GlobalLArCellContainer : public DataVector<GlobalSim::GlobalLArCell> {

  public:

    using DataVector<GlobalLArCell>::DataVector;

    /** @brief reimplementation of the push_back function to fill LArCells */
    void push_back(const GlobalLArCell& theCell);

  private:

  };

} //namespace GlobalSim

CLASS_DEF(GlobalSim::GlobalLArCellContainer, 1117669828, 1)
SG_BASE(GlobalSim::GlobalLArCellContainer, DataVector<GlobalSim::GlobalLArCell> );

#endif
