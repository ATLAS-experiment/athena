/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SUBJET_BUILDER_ALG_H
#define SUBJET_BUILDER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODBase/IParticleContainer.h"

#include "AthLinks/ElementLink.h"

#include <string>
#include <vector>

namespace ftag {

  /// Recluster the ghost-associated constituents of a jet into small-R
  /// subjets, linked back from the parent jet.
  class SubjetBuilderAlg final : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

  private:
    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;

    SG::ReadHandleKey<xAOD::JetContainer> m_targetJetsKey{
      this, "targetJets", "", "Input large-R jet collection"};

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_ghostKey{
      this, "ghostAssociation", m_targetJetsKey, "GhostTruth",
      "Ghost-associated constituent links on the jet"};

    SG::WriteHandleKey<xAOD::JetContainer> m_outputKey{
      this, "outputContainer", "", "Output subjet container"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_linkKey{
      this, "linkName", m_targetJetsKey, "", "Decoration with links to subjets"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_countKey{
      this, "countName", m_targetJetsKey, "",
      "Decoration with the number of reconstructed subjets per jet"};

    Gaudi::Property<float> m_radius{
      this, "radius", 0.2, "Anti-kt clustering radius"};
    Gaudi::Property<float> m_ptMin{
      this, "ptMin", 5000, "Minimum subjet pT in MeV"};
  };

} // end namespace ftag

#endif
