/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIGLOBAL_HIEVENTSHAPEFILLERTOOL_H
#define HIGLOBAL_HIEVENTSHAPEFILLERTOOL_H

#include "AsgTools/AsgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "HIGlobal/IHIEventShapeFiller.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "HIEventUtils/HIEventShapeIndex.h"
#include "HIEventUtils/HITowerWeightTool.h"
#include "HIEventUtils/HIEventShapeMapTool.h"
#include <NavFourMom/INavigable4MomentumCollection.h>

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloEvent/CaloCellContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class CaloCellContainer;

class HIEventShapeFillerTool : public asg::AsgTool, virtual public IHIEventShapeFiller
{
  ASG_TOOL_CLASS(HIEventShapeFillerTool, IHIEventShapeFiller)

public:
  HIEventShapeFillerTool(const std::string& myname);

  virtual StatusCode initialize() override;
  virtual StatusCode initializeIndex() override;
  virtual StatusCode initializeEventShapeContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape) const override;
  virtual StatusCode fillCollectionFromTowers(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<xAOD::CaloClusterContainer>& tower_container_key, const SG::ReadHandleKey<INavigable4MomentumCollection>& navi_container_key, const EventContext& ctx) const override;
  virtual StatusCode fillCollectionFromCells(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<CaloCellContainer>& cell_container_key, const EventContext& ctx) const override;

  virtual StatusCode fillCollectionFromTowerContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const INavigable4MomentumCollection* navInColl) const;
  virtual StatusCode fillCollectionFromCellContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const CaloCellContainer* CellContainer) const;
  virtual StatusCode fillCollectionFromClusterContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const xAOD::CaloClusterContainer* theClusters, const EventContext& ctx) const;

private:
  const HIEventShapeIndex* m_index;
  const CaloCell_ID * m_calo_id {nullptr};

  void updateShape(std::unique_ptr<xAOD::HIEventShapeContainer>& shape, const HIEventShapeIndex* index, const CaloCell* theCell, float geoWeight, float eta0, float phi0, bool isNeg = false) const;

  std::unique_ptr<const CaloCell> getMirroredCell(const CaloCell* pCell, const CaloCellContainer* ccc) const;

  ToolHandle<IHITowerWeightTool>   m_towerWeightTool{ this, "TowerWeightTool", "HITowerWeightTool", "Handle to Tower Weight Tool" };
  ToolHandle<IHIEventShapeMapTool> m_eventShapeMapTool{ this, "EventShapeMapTool", "HIEventShapeMapTool", "Handle to Event Shape Map Tool" };

  SG::ReadHandleKey<CaloCellContainer> m_caloCellKey{this, "CaloCellContainerKey","","cell container for mirror"};

  Gaudi::Property< bool >          m_useClusters{ this, "UseClusters", false, "use Clusters boolean switch" };
  Gaudi::Property< int >           m_numOrders{ this, "OrderOfFlowHarmonics", 7, "The number of Orders of harmonic flow to store in the EventShape" };

};

#endif
