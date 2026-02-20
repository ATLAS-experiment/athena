/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRACKPARAMETERSATPV_H
#define DERIVATIONFRAMEWORK_TRACKPARAMETERSATPV_H

#include<string>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include <vector>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

namespace DerivationFramework {

  /** @class TrackParametersAtPV

      the code used in this implementation is kindly stolen from:
      atlasoff:: ISF/ISF_Core/ISF_Tools

      @author James Catmore -at- cern.ch
  */
  class TrackParametersAtPV : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /** Check that the current event passes this filter */
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_collTrackKey
    { this, "TrackParticleContainerName", "InDetTrackParticles", ""};
    SG::ReadHandleKey<xAOD::VertexContainer>        m_collVertexKey
      { this, "VertexContainerName", "PrimaryVertices", ""};

    SG::WriteHandleKey< std::vector<float> > m_trackZ0PVKey
      { this, "Z0SGEntryName", "", "" };

  };

}

#endif
