/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORK_MuonTruthClassifierFallback_H
#define DERIVATIONFRAMEWORK_MuonTruthClassifierFallback_H

#include <string>
#include <vector>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthPileupEventContainer.h"

namespace DerivationFramework {
  class MuonTruthClassifierFallback : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    // Athena algtool's Hooks
    virtual StatusCode initialize() override;

    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerKey{this, "ContainerKey", "", "Key of the container to be decorated"};
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthSGKey{this, "TruthSGKey", "TruthEvents", "Key of the truth event container"};
    SG::ReadHandleKey<xAOD::TruthPileupEventContainer> m_truthPileupSGKey{this, "TruthPileupContainerKey", "TruthPileupEvents",
      "Key of the pile-up event container"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonSGKey{this, "TruthMuonContainerKey", "MuonTruthParticles", ""};

    // FIXME These WriteDecorHandles are not being used. The
    // Decorators defined at the top of the .cxx are (incorrectly)
    // used instead.
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_dR_Key{
      this, "dRDecoration", m_containerKey, "MCTFallback_dR"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_type_Key{
      this, "typeDecoration", m_containerKey, "MCTFallback_truthType"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_origin_Key{
      this, "originDecoration", m_containerKey, "MCTFallback_truthOrigin"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_PU_dR_Key{
      this, "dRDecorationPU", m_containerKey, "MCTFallbackPU_dR"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_PU_type_Key{
      this, "typeDecorationPU", m_containerKey, "MCTFallbackPU_truthType"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_Truth_PU_origin_Key{
      this, "originDecorationPU", m_containerKey, "MCTFallbackPU_truthOrigin"};

    Gaudi::Property<float> m_minPt{this, "MinPt", 2500};

    ToolHandle<IMCTruthClassifier> m_mcTruthClassifier{this, "MCTruthClassifierTool", "", "Handle of the MC truth classifier"};
  };
}  // namespace DerivationFramework
#endif  //
