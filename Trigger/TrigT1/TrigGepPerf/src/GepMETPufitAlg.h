/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPMETPUFITALG_H
#define TRIGGEPPERF_GEPMETPUFITALG_H

/* construct MET objects from CalClusters. The origin of these may be
   standard ATLAS reconstruction, or by Gep Algorithms */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODCaloEvent/CaloClusterContainer.h"


class GepMETPufitAlg: public ::AthReentrantAlgorithm {
 public: 
  GepMETPufitAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GepMETPufitAlg();

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext&) const override;
  virtual StatusCode  finalize() override;

 private:

  
  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};
  
  SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputMETPufitKey {
    this, "outputMETPufitKey", "", "key to write out a MET object"};

  StatusCode PufitMET(const xAOD::CaloClusterContainer&,
		      float inputSigma,
		      const EventContext&) const;

}; 

#endif //> !TRIGGEPPERF_MISSINGETGEPPUFIT_H
