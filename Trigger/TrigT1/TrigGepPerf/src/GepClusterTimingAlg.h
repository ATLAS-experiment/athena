/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPCLUSTERTIMINGALG_H
#define TRIGGEPPERF_GEPCLUSTERTIMINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "AthContainers/ConstDataVector.h"

class GepClusterTimingAlg: public ::AthReentrantAlgorithm { 
 public: 
  GepClusterTimingAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~GepClusterTimingAlg(); 

  std::string m_outputClusterName; 
  std::string m_inputClusterName;
  
  virtual StatusCode  initialize();
  virtual StatusCode  execute(const EventContext&) const;
  virtual StatusCode  finalize();

private: 

  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_inCaloClustersKey {
    this, "inCaloClustersKey", "CaloTopoClusters", "key to read in a CaloCluster constainer"};

  SG::WriteHandleKey<ConstDataVector<xAOD::CaloClusterContainer>>
  m_outCaloClustersKey {
    this, "outCaloClustersKey", "Clusters420Timing", "key to write out a CaloCluster constainer"};


  Gaudi::Property<float> m_lambdaCalDivide {this, "lambdaCalDivide", 317, ""};
  Gaudi::Property<float> m_qualityCut {this, "qualityCut", 0.02, ""};
  Gaudi::Property<float> m_timeCutLargeQ {this, "timeCutLargeQ", 5, ""};
  Gaudi::Property<float> m_timeCutSmallQ{this, "timeCutSmallQ", 15, ""};
  Gaudi::Property<float> m_etaCut{this, "maxEtaForCut", 5.0, "Default apply to all cluster eta regions"};


}; 

#endif
