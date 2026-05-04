/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace DerivationFramework {

  class InDetTrackSelectionToolWrapper : public AthReentrantAlgorithm { // TODO Rename class
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final ;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    ToolHandle< InDet::IInDetTrackSelectionTool > m_tool
    {this,"TrackSelectionTool",""};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey
      {this, "ContainerName", "InDetTrackParticles", "The input TrackParticleCollection"};

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_decorationKey
      {this, "DecorationName", m_tracksKey, "", "Name of the decoration which provides the track selection result."};
  };
}

#endif // DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H
