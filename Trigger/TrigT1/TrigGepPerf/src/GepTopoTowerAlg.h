/*
 *   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPTOPOTOWERALG_H
#define TRIGL0GEPPERF_GEPTOPOTOWERALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "GepCellMap.h"

class GepTopoTowerAlg: public ::AthReentrantAlgorithm {
 public: 
  GepTopoTowerAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GepTopoTowerAlg();

  virtual StatusCode  initialize();     //once, before any input is loaded
  virtual StatusCode  execute(const EventContext&) const;        //per event
  virtual StatusCode  finalize();       //once, after all events processed

 private: 

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

#endif //> !TRIGL0GEPPERF_TOPOTOWER_H
