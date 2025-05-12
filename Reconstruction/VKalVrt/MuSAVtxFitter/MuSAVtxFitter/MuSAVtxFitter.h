/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUSAVTXFITTER_MUSAVTXFITTER_H
#define MUSAVTXFITTER_MUSAVTXFITTER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "MuSAVtxFitter/MuSAVtxFitterTool.h"
#include "ITrackToVertex/ITrackToVertex.h"


/**
 @class MuSAVtxFitter
 Execute method for the main MuSA vertex finding module. MuSAVtxFitter uses the MuSAVtxFitterTool and records a MuSA vertex container.
 */

namespace Rec {
    class MuSAVtxFitterTool;

    class MuSAVtxFitter : public AthReentrantAlgorithm
    {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual ~MuSAVtxFitter();
        virtual StatusCode initialize() override;
        StatusCode fillCollections(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer,
                      xAOD::VertexContainer* MuSAVtxContainer,
                      xAOD::TrackParticleContainer* MuSAExtrapolatedTracksContainer,
                      const xAOD::MuonContainer& muonContainer,
                      const xAOD::TrackParticleContainer& MSTPContainer,
                      const EventContext& ctx) const;
        virtual StatusCode execute(const EventContext& ctx) const override;
        
    protected:
        SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainer { this, "MuonContainerName", "Muons", "Muon container key" };
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo { this, "EventInfoName", "EventInfo", "Event info key" };
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_MSTPContainer {this, "MSTPContainerName", "MuonSpectrometerTrackParticles", "MSTP Key"};

        // MuSA vtx candidate output
        SG::WriteHandleKey<xAOD::VertexContainer> m_MuSAVertices { this, "MuSAVtxContainerName", "MuSAVertices", "MuSA vtx container" };

        // MuSA extrapolated tracks
        SG::WriteHandleKey<xAOD::TrackParticleContainer> m_MuSAExtrapolatedTracks { this, "MuSAExtrapolatedTracksName", "MuSAExtrapolatedTrackParticles", "MuSA extrapolated tracks" };

        // Tools
        ToolHandle<Rec::MuSAVtxFitterTool> m_MuSAVtxFitterTool{this, "MuSAVtxToolName", "Rec::MuSAVtxFitterTool"};
        ToolHandle<Reco::ITrackToVertex> m_trackToVertexTool{this, "TrackToVertexTool", "Reco::TrackToVertex"};

    };; // end class MuSAVtxFitter
}
#endif // MUSAVTXFITTER_MUSAVTXFITTER_H
