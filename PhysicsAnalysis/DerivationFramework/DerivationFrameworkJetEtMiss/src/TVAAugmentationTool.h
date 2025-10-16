/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef DERIVATIONFRAMEWORK_TVAAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_TVAAUGMENTATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "AsgTools/ToolHandle.h"
#include "TrackVertexAssociationTool/ITrackVertexAssociationTool.h"
#include "AthLinks/ElementLink.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include <memory>

namespace DerivationFramework {
  class TVAAugmentationTool : public extends<AthAlgTool, IAugmentationTool>
  {
  public:
    TVAAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;
  private:
    // Properties
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackName{this, "TrackName", "InDetTrackParticles"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexName{this, "VertexName", "PrimaryVertices"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_vtxDec_key {this, "LinkName", m_trackName, "", "Decoration for associated vertex"};
    PublicToolHandle<CP::ITrackVertexAssociationTool> m_tool{this, "TVATool", ""};
    // Internals
    using vtxLink_t = ElementLink<xAOD::VertexContainer>;

  }; //> end class TVAAugmentationTool
} //> end namespace DerivationFramework

#endif //> !DERIVATIONFRAMEWORK_TVAAUGMENTATIONTOOL_H
