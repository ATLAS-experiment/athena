/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFCLUSTERWIDTHDECORATOR_H
#define EFLOWREC_PFCLUSTERWIDTHDECORATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "PFClusterWidthCalculator.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "StoreGate/WriteDecorHandle.h"

class PFClusterWidthDecorator : public AthReentrantAlgorithm {

public:
  PFClusterWidthDecorator(const std::string& name, ISvcLocator* pSvcLocator);    
  ~PFClusterWidthDecorator() = default;

  StatusCode initialize() override;
  StatusCode execute(const EventContext & ctx) const override;

private:
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterContainerWidthEtaKey{this,"clusterContainerWidthEtaName","CaloCalTopoClusters.ClusterWidthEta","Cluster Container Width Eta Key"};
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterContainerWidthPhiKey{this,"clusterContainerWidthPhiName","CaloCalTopoClusters.ClusterWidthPhi","Cluster Container Width Phi Key"};
  PFClusterWidthCalculator m_clusterWidthCalculator;
};

#endif
