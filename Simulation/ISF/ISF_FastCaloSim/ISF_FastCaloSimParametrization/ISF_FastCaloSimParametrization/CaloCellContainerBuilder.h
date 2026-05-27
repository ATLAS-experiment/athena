/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERBUILDER_H
#define ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERBUILDER_H

#include "CaloEvent/CaloCellContainer.h"
#include "HitManagement/AthenaHitsVector.h"

#include <memory>

/**
 * @brief Event-owned holder for the FastCaloSim CaloCellContainer.
 *
 * CaloCellContainer does not derive from HitsVectorBase, so this lightweight
 * holder lets the Geant4 event user info carry it through HitCollectionMap.
 * StoreGate ownership is taken in CaloCellContainerSDTool::Gather.
 */
class CaloCellContainerBuilder : public HitsVectorBase
{
public:
  CaloCellContainerBuilder()
    : container(std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS))
  {}

  std::unique_ptr<CaloCellContainer> container;
};

#endif // ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERBUILDER_H
