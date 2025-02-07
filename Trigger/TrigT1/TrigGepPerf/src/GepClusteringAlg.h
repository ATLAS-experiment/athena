/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPCLUSTERINGALG_H
#define TRIGGEPPERF_GEPCLUSTERINGALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "GepCellMap.h"

//typedef std::map<unsigned int,Gep::GepCaloCell> GepCellMap;

class GepClusteringAlg: public ::AthReentrantAlgorithm { 

 public: 

  GepClusteringAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;   
  virtual StatusCode  execute(const EventContext& ) const override;    

 private: 
  
  Gaudi::Property<std::string> m_clusterAlg{
    this, "TopoClAlg", "", "name of Gep clustering algorithm"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "eventInfo", "EventInfo", "key to read in an EventInfo object"};

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputCaloClustersKey{
    this, "outputCaloClustersKey", "", "key for CaloCluster wrappers for GepClusters"};  
  
  SG::ReadHandleKey<Gep::GepCellMap> m_gepCellsKey {
    this, "gepCellMapKey", "GepCells", "Key to get the correct cell map"};

}; 

#endif //> !TRIGGEPPERF_GEPCLUSTERINGALG_H

