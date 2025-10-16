/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCALOSHOWERDECORATOR_H
#define DERIVATIONFRAMEWORK_TRUTHCALOSHOWERDECORATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "CaloCalibHitRec/CalibHitToCaloCellTool.h"
#include "xAODTruth/TruthParticleContainer.h"

#include <vector>

namespace DerivationFramework {

class TruthCaloShowerDecorator : public extends<AthAlgTool, IAugmentationTool>
{
public:
  TruthCaloShowerDecorator(const std::string& t,
                           const std::string& n,
                           const IInterface* p);

  StatusCode initialize();
  StatusCode finalize();
  virtual StatusCode addBranches(const EventContext& ctx) const;

private:
  /** @brief SG key of the truth particle container **/
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerName{
    this,
    "TruthParticleContainerName",
    "egammaTruthParticles",
    "SG key of the truth particle container"
  };

  // Non-configurable read handle keys
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_truthClusterContainerEtot{
    "TruthLArClustersEtot",
  };
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_truthClusterContainerEvis{
    "TruthLArClustersEvis"
  };
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_truthClusterContainerEem{
    "TruthLArClustersEem"
  };

  // Write decoration handle keys
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>
    m_linkDecoratorClusterEtot{ this, "truthLArClusterEtotLink", "", "" };
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>
    m_linkDecoratorClusterEvis{ this, "truthLArClusterEvisLink", "", "" };
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>
    m_linkDecoratorClusterEem{ this, "truthLArClusterEemLink", "", "" };

  ToolHandle<CalibHitToCaloCellTool> m_calibhitToCaloCellTool{
    this,
    "CalibHitToCaloCellTool",
    "",
    "Handle to the Calib Hit to Calo cell Tool"
  };
};
}

#endif // DERIVATIONFRAMEWORK_TRUTHCALOSHOWERDECORATOR_H
