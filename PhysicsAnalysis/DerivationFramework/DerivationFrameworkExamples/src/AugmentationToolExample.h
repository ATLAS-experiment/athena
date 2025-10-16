/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H
#define DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace DerivationFramework {

  class AugmentationToolExample : public extends<AthAlgTool, IAugmentationTool> {
  public:
    using base_class::base_class;
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{this, "VertexContainer", "PrimaryVertices",""};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackPartContainerKey{this, "TrackParticleContainer", "InDetTrackParticles", "Container to be decorated"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_exampleDecorKey{this, "ExampleDecorKey", m_trackPartContainerKey, "DFDecoratorExample", "Decoration"};
    SG::WriteHandleKey<std::vector<float> > m_decisionKey{this, "DecisionKey", "DFAugmentationExample", "Write decision to SG for access by downstream algs"};
  };
}

#endif // DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H
