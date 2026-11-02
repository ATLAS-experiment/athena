/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOTOPOTOWERALGORITHM_H
#define CALOREC_CALOTOPOTOWERALGORITHM_H
///////////////////////////////////////////////////////////////////////////////
/// \brief CaloTopoTower builder algorithm
///
/// The CaloTopoTowerAlgorithm executes a sequence of CaloTopoTowerBuilderTools 
/// defined in the jobOptions. It creates, records and locks a 
/// CaloTopoTowerContainer into StoreGate. Even though this class is probably
/// sufficient for all tower builder strategies, it has been designed as
/// a concrete base class, thus allowing to overwrite the default behavior with
/// very specific client implementations.
///
/// \author Ian M. Nugent <inugent@triumf.ca>
/// \date March 15, 2010 - first implementation
///
/// Based on CaloTowerAlgorithm
///
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "CaloUtils/CaloTopoTowerBuilderToolBase.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloEvent/CaloCell2ClusterMap.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "CaloEvent/CaloTopoTowerContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include <string>
#include <vector>

class ICaloTopoTowerBuilderToolBase;

class CaloTopoTowerAlgorithm : public AthReentrantAlgorithm
{
 public:

  typedef CaloTopoTowerBuilderToolBase         tool_type;
  typedef std::vector<tool_type*>              tool_store;
  typedef tool_store::const_iterator           tool_iterator;
  
  /// Algorithm constructor
  CaloTopoTowerAlgorithm(const std::string& name, ISvcLocator* pService);

  virtual ~CaloTopoTowerAlgorithm();

  /// inherited from Algorithm
  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 protected:

  ////////////////
  // Properties //
  ////////////////

  // link output container
  Gaudi::Property<bool> m_genericLink{this, "GenericLinked", true};

  // tool collection
  std::vector<std::string> m_toolNames;
  ToolHandleArray<ICaloTopoTowerBuilderToolBase> m_ptools{this, "TowerBuilderTools", {}};

  /////////////////////////
  // Stores and Services //
  /////////////////////////

  // list of tools
  tool_store m_tools;

  //////////////////////////////////////////
  // CaloTopoTowerContainer variables
  // Container name strings
  SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this, "CellContainerName", "AllCalo"};
  SG::ReadHandleKey<CaloClusterContainer> m_clusterKey{this, "ClusterContainerName", "CaloTopoClusters"};
  SG::ReadHandleKey<CaloCell2ClusterMap> m_cellToClusterMapKey{this, "Cell2ClusterMapName", "CaloCell2TopoCluster"};
  SG::ReadHandleKey<CaloTowerContainer> m_towerContainerKey{this, "InputTowerContainerName", "CmbTower"};
  SG::WriteHandleKey<CaloTopoTowerContainer> m_newTowerContainerKey{this, "OutputTowerContainerName", "TopoTower"};

  // Selection criteria
  Gaudi::Property<double> m_minimumCellEnergy{this, "MinimumCellEnergy", -1000000000.0};
  Gaudi::Property<double> m_minimumClusterEnergy{this, "MinimumClusterEnergy", -1000000000.0};
  Gaudi::Property<bool>   m_useCellWeights{this, "useCellWeight", true};

  // Noise tool stuff
  Gaudi::Property<float> m_noiseSigma{this, "DefaultNoiseSigma", 10.0};
  Gaudi::Property<float> m_cellESignificanceThreshold{this, "CellEnergySignificance", -1.f};

  // Type definitions
  //  typedef Navigable<CaloClusterContainer>           nav_t;

  // List of calorimeters from which to use cells
  Gaudi::Property<std::vector<std::string>> m_includedCalos{this, "IncludedCalos", {}};
  std::vector<CaloCell_ID::SUBCALO> m_caloIndices;
  bool m_caloSelection{false};

  //END CaloTopoTowerContainer variables
  ////////////////////////////////////////////////
};
#endif
