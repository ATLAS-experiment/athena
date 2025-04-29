/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGT2CALOCOMMON_HLTCALOCELLSUMMAKER_H
#define TRIGT2CALOCOMMON_HLTCALOCELLSUMMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/ConstDataVector.h"
#include "CaloEvent/CaloCellContainerVector.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>

/**
 * @brief Algorithm tor print and debug cell sums
 *
 * @author Denis Oliveira Damazio
 */
class HLTCaloCellSumMaker: public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode initialize() override;

private:
  SG::ReadHandleKey<ConstDataVector<CaloCellContainerVector> > m_cellContainerVKey{
    this, "CellsVName", "CellsVClusters"};

  SG::ReadHandleKey<CaloCellContainer > m_cellContainerKey{
    this, "CellsName", "CellsClusters"};

  Gaudi::Property<bool> m_roiMode{
    this, "roiMode", true, "RoiMode roi->CaloCellCollection"};
};

#endif
