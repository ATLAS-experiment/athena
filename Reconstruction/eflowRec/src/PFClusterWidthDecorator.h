#ifndef PFCLUSTERWIDTHDECORATOR_H
#define PFCLUSTERWIDTHDECORATOR_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "PFClusterWidthCalculator.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "StoreGate/WriteDecorHandle.h"

class PFClusterWidthDecorator : public AthAlgorithm {
public:
  PFClusterWidthDecorator(const std::string& name, ISvcLocator* pSvcLocator);    
  ~PFClusterWidthDecorator() = default;

  StatusCode initialize() override;
  StatusCode execute() override;

private:
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterContainerWidthEtaKey{this,"clusterContainerWidthEtaName","CaloCalTopoClusters.ClusterWidthEta","Cluster Container Width Eta Key"};
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterContainerWidthPhiKey{this,"clusterContainerWidthPhiName","CaloCalTopoClusters.ClusterWidthPhi","Cluster Container Width Phi Key"};
  PFClusterWidthCalculator m_clusterWidthCalculator;
};

#endif
