/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDALGS_MUONCOMBINEDMUONCANDIDATEALG_H
#define MUONCOMBINEDALGS_MUONCOMBINEDMUONCANDIDATEALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "BeamSpotConditionsData/BeamSpotData.h"

#include "MuidInterfaces/ICombinedMuonTrackBuilder.h"
#include "MuonCombinedToolInterfaces/IMuonTrackToSegmentAssociationTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonTrackExtrapolationTool.h"

#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "MuonCombinedEvent/MuonCandidateCollection.h"
#include "TrkTrack/TrackCollection.h"

class MuonCombinedMuonCandidateAlg : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    ~MuonCombinedMuonCandidateAlg() = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_muonTrackParticleLocation{
        this,
        "MuonSpectrometerTrackParticleLocation",
        "MuonSpectrometerTrackParticles",
        "MS Track Particle collection",
    };
    SG::WriteHandleKey<MuonCandidateCollection> m_candidateCollectionName{
        this,
        "MuonCandidateLocation",
        "MuonCandidates",
        "Muon candidate collection",
    };
    SG::WriteHandleKey<TrackCollection> m_msOnlyTracks{
        this,
        "MSOnlyExtrapolatedTrackLocation",
        "MSOnlyExtrapolatedTracks",
        "MS extrapolated muon tracks",
    };
    PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "Printer", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
    ToolHandle<Rec::ICombinedMuonTrackBuilder> m_trackBuilder{this, "TrackBuilder", "", ""};
    ToolHandle<Muon::IMuonTrackExtrapolationTool> m_trackExtrapolationTool{this, "TrackExtrapolationTool",
                                                                               "ExtrapolateMuonToIPTool/ExtrapolateMuonToIPTool"};
    ToolHandle<Trk::ITrackAmbiguityProcessorTool> m_ambiguityProcessor{this, "AmbiguityProcessor",
                                                                        "Trk::TrackSelectionProcessorTool/MuonAmbiProcessor"};
    ToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{this, "TrackSummaryTool", "MuonTrackSummaryTool"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

    /// Retrieve the segment container to perform the segment association offline
    SG::ReadHandleKey<Trk::SegmentCollection> m_segmentKey{this, "SegmentContainer", ""};
    PublicToolHandle<MuonCombined::IMuonTrackToSegmentAssociationTool> m_trackSegmentAssociationTool{
        this, "TrackSegmentAssociationTool", "MuonCombined::TrackSegmentAssociationTool/TrackSegmentAssociationTool"};

    Gaudi::Property<unsigned int> m_extrapolationStrategy{this, "ExtrapolationStrategy", 0};
};

#endif
