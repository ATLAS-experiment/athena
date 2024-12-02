/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPCLUSTERINGALG_H
#define TRIGL0GEPPERF_GEPCLUSTERINGALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "./CaloCellsHandlerTool.h"

class GepClusteringAlg: public ::AthReentrantAlgorithm { 
 public: 
  GepClusteringAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;   
  virtual StatusCode  execute(const EventContext& ) const override;    

 private: 

  
  Gaudi::Property<std::string> m_clusterAlg{
    this, "TopoClAlg", "", "name of Gep clustering algorithm"};

  ToolHandle<CaloCellsHandlerTool> m_caloCellsTool{
    this,
    "CaloCellHandler",
    "",
    "create cont of Gep::GepCaloCells"};

  SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {
    this, "caloCells", "AllCalo", "key to read in a CaloCell constainer"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "eventInfo", "EventInfo", "key to read in an EventInfo object"};


  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputCaloClustersKey{
    this, "outputCaloClustersKey", "",
    "key for CaloCluster wrappers for GepClusters"};  
  
}; 

#endif //> !TRIGL0GEPPERF_GEPCLUSTERINGALG_H




