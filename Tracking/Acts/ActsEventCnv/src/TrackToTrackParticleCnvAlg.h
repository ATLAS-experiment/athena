/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVALG_H
#define ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "Gaudi/Property.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODTracking/TrackParticleContainer.h"

#include "ActsEvent/TrackContainer.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "xAODTracking/VertexContainer.h"

#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"

namespace ActsTrk {

  /** @brief Conversion algorithm to translate multiple transient Acts track containers
   *         to an xAOD track particle container */
  class TrackToTrackParticleCnvAlg : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     static std::shared_ptr<Acts::PerigeeSurface> makePerigeeSurface(const InDet::BeamSpotData *beamspotptr);    
     static std::shared_ptr<Acts::PerigeeSurface> makePerigeeSurface(const xAOD::Vertex&);

    ToolHandle<ActsTrk::ITrackToTrackParticleCnvTool> m_cnvTool
       {this, "TrackToTrackParticleCnvTool", ""};

    SG::ReadHandleKeyArray<ActsTrk::TrackContainer> m_tracksContainerKey
       {this, "ACTSTracksLocation", {},"Track collection (ActsTrk variant)"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey
       {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot or empty." };

    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey
       {this, "VertexContainerKey", "", "Name of the Primary Vertex Container"};
    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_trackParticlesOutKey{this, "TrackParticlesOutKey",
      "ChangeMe", "Name of the produced track particle collection" };

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_decorator_actsTracks{this, "ActsTrackLink", m_trackParticlesOutKey, "actsTrack"};
      Gaudi::Property<std::string> m_perigeeExpression{this, "PerigeeExpression", "DontRecalculate"};

    enum class expressionStrategy {DontRecalculate, BeamLine, Vertex};
    expressionStrategy m_expression_strategy {expressionStrategy::BeamLine};
  };

}

#endif
