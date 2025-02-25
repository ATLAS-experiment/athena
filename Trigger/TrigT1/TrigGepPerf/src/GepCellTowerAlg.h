/*
 *   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPCELLTOWERALG_H
#define TRIGL0GEPPERF_GEPCELLTOWERALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "GepCellMap.h"

//typedef std::map<unsigned int,Gep::GepCaloCell> GepCellMap;

class GepCellTowerAlg: public ::AthReentrantAlgorithm {
 public: 
  GepCellTowerAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GepCellTowerAlg();

  virtual StatusCode  initialize();     //once, before any input is loaded
  virtual StatusCode  execute(const EventContext&) const;        //per event
  virtual StatusCode  finalize();       //once, after all events processed

 private: 

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputCellTowerKey{
    this, "outputCellTowerKey", "",
    "key for CaloCluster wrappers for GepClusters"};

  SG::ReadHandleKey<Gep::GepCellMap> m_gepCellsKey {
    this, "gepCellMapKey", "GepCells", "Key to get the correct cell map"};  

}; 

#endif //> !TRIGL0GEPPERF_TOPOTOWER_H
