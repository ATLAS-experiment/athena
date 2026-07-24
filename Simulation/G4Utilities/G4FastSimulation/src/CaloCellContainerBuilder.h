/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_CALOCELLCONTAINERBUILDER_H
#define G4FASTSIMULATION_CALOCELLCONTAINERBUILDER_H

#include "CaloEvent/CaloCellContainer.h"
#include "HitManagement/AthenaHitsVector.h"

#include <memory>

class CaloCellContainerBuilder : public HitsVectorBase
{
public:
  CaloCellContainerBuilder()
    : container(std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS))
  {}

  std::unique_ptr<CaloCellContainer> container;
};

#endif  // G4FASTSIMULATION_CALOCELLCONTAINERBUILDER_H
