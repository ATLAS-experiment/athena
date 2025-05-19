/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGEPPERF_GEPTOWERSALG_H
#define TRIGGEPPERF_GEPTOWERSALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "GepCellMap.h"

class GepTowersAlg: public ::AthReentrantAlgorithm { 

 public: 

  GepTowersAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;   
  virtual StatusCode  execute(const EventContext& ) const override;    

 private: 
  
  Gaudi::Property<std::string> m_towerAlg{
    this, "TowerAlg", "", "name of Gep Tower algorithm"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "eventInfo", "EventInfo", "key to read in an EventInfo object"};

  SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {
    this, "caloCells", "AllCalo", "key to read in a CaloCell constainer"};

  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputCaloClustersKey{
    this, "outputCaloClustersKey", "",
    "key for CaloCluster wrappers for GepClusters"};

  SG::ReadHandleKey<Gep::GepCellMap> m_gepCellsKey {
    this, "gepCellMapKey", "GepCells", "Key to get the correct cell map"};

}; 

#endif //> !TRIGGEPPERF_GEPTOWERSALG_H

