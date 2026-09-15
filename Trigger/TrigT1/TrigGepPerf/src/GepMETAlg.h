/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPMISSINGETALG_K
#define TRIGGEPPERF_GEPMISSINGETALG_K

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

class GepMETAlg: public ::AthReentrantAlgorithm {
 public: 
  GepMETAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GepMETAlg();

  virtual StatusCode  initialize();
  virtual StatusCode  execute(const EventContext&) const;
  virtual StatusCode  finalize();
private:

  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};
  
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputMETKey {
    this, "outputMETKey", "", "key to write out a MET object"};

}; 

#endif //> !TRIGGEPPERF_MISSINGETGEP_H
