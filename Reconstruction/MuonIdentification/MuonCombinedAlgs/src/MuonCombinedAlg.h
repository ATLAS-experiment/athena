/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDALGS_MUONCOMBINEDALG_H
#define MUONCOMBINEDALGS_MUONCOMBINEDALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonCombinedEvent/InDetCandidateCollection.h"
#include "MuonCombinedEvent/InDetCandidateToTagMap.h"
#include "MuonCombinedEvent/MuonCandidateCollection.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTrack/TrackCollection.h"

#include "MuidInterfaces/IMuonAlignmentUncertTool.h"
#include "MuonCombinedToolInterfaces/IMuonCombinedTagTool.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"

class MuonCombinedAlg : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    ~MuonCombinedAlg() = default;

    StatusCode execute(const EventContext& ctx) const override;
    StatusCode initialize() override;

private:
    /** @brief Checks whether the Id & the muon candidate are compatible to attempt
     *         a combination of the two as a single track */
    bool pass_prematching(const MuonCombined::MuonCandidate& muonCandidate, 
                          const MuonCombined::InDetCandidate& idCandidate) const;

    SG::ReadHandleKey<InDetCandidateCollection> m_indetCandidateCollectionName{this, "InDetCandidateLocation", "InDetCandidates",
                                                                               "name of ID candidate collection"};
    SG::ReadHandleKey<MuonCandidateCollection> m_muonCandidateCollectionName{this, "MuonCandidateLocation", "MuonCandidates",
                                                                             "name of muon candidate collection"};
    SG::WriteHandleKeyArray<MuonCombined::InDetCandidateToTagMap> m_combTagMaps{
        this, "CombinedTagMaps", {"muidcoTagMap", "stacoTagMap"}, "combined muon tag maps"};
    SG::WriteHandleKey<TrackCollection> m_muidCombinedTracks{this, "MuidCombinedTracksLocation", "MuidCombinedTracks",
                                                             "Muidco combined Tracks"};
    SG::WriteHandleKey<TrackCollection> m_muidMETracks{this, "MuidMETracksLocation", "MuidMETracks", "Muidco ME Tracks"};

    // helpers, managers, tools
    PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "Printer", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
    ToolHandleArray<MuonCombined::IMuonCombinedTagTool> m_muonCombinedTagTools{this, "MuonCombinedTagTools", {}};

    /// Use this tool to retrieve the last and first measurments of the ID and MS, respectively.
    PublicToolHandle<Muon::IMuonAlignmentUncertTool> m_alignUncertTool{this, "AlignmentUncertTool", ""};

    Gaudi::Property<float> m_deltaEtaPreSelection{this, "DeltaEtaPreSelection", 0.5};
    Gaudi::Property<float> m_deltaPhiPreSelection{this, "DeltaPhiPreSelection", 1};
    Gaudi::Property<float> m_ptBalance{this, "PtBalancePreSelection", -1.};
};

#endif
