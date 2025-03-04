/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPTCTOWERALG_H
#define TRIGL0GEPPERF_GEPTCTOWERALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
class EventContext;



class GepTCTowerAlg: public ::AthReentrantAlgorithm {
 public: 
  GepTCTowerAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GepTCTowerAlg();

  virtual StatusCode  initialize();     //once, before any input is loaded
  virtual StatusCode  execute(const EventContext&) const;        //per event
  virtual StatusCode  finalize();       //once, after all events processed

 private: 

  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputCaloClustersKey{
    this, "outputCaloClustersKey", "",
    "key for CaloCluster wrappers for GepClusters"};

}; 

#endif //> !TRIGL0GEPPERF_TCTOWER_H
