/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InDetTrackSelectionToolWrapper.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace DerivationFramework {

  class InDetTrackSelectionToolWrapper : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final ;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    ToolHandle< InDet::IInDetTrackSelectionTool > m_tool
    {this,"TrackSelectionTool",""};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey
      {this, "ContainerName", "InDetTrackParticles", "The input TrackParticleCollection"};

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>               m_decorationKey
      {this, "DecorationName", "","Name of the decoration which provides the track selection result."};
  };
}

#endif // DERIVATIONFRAMEWORK_INDETTRACKSELECTIONTOOLWRAPPER_H
