/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_SEGMENTSELECTIONTOOL_H
#define MUONTRACKFINDINGTOOLS_SEGMENTSELECTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPatternEvent/Segment.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"


namespace MuonR4 {
    class SegmentSelectionTool : public extends <AthAlgTool, ISegmentSelectionTool> {
        public:
            using base_class::base_class;
            using HitSummary = Segment::HitSummary;

            virtual StatusCode initialize() override final;

            virtual bool passSeedingQuality(const EventContext& ctx,
                                            const Segment& segment) const override final;

            virtual bool passTrackQuality(const EventContext& ctx,
                                          const Segment& segment) const override final;

            virtual bool compatibleForTrack(const EventContext& ctx,
                                            const Segment& segA,
                                            const Segment& segB) const override final;
        private:
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Cut on minimum number of Mdt hits to consider the segment for seeding  */
            Gaudi::Property<unsigned> m_nMdtSeedHitCut{this, "minMdtSeedHits" , 4};
            /** @brief Cut on minimum number of Mdt hits to consider the segment for tracking */
            Gaudi::Property<unsigned> m_nMdtMinHitCut{this, "minMdtHits" , 3};
            /** @brief Minimum number of Nsw hits */
            Gaudi::Property<unsigned> m_nNswMinHitCut{this, "minNswHits", 5};
            /** @brief Cont on maximum number of outlies to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nMdtSeedOutlierCut{ this, "maxMdtOutliers", 15};
            /** @brief Minimum number of Rpc phi hits in BI to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nRpcPhiSeedHitCutBI{ this, "minRpcPhiSeedHitsBI", 2};
            /** @brief Minimum number of Rpc phi hits in BM to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nRpcPhiSeedHitCutBM{ this, "minRpcPhiSeedHitsBM", 2};
            /** @brief Minimum number of Rpc phi hits in BO to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nRpcPhiSeedHitCutBO{ this, "minRpcPhiSeedHitsBO", 1};
            /** @brief Minimum number of Tgc phi hits in EI to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nTgcPhiSeedHitCutEI{this, "minTgcPhiSeedHitsEI", 1};
            /** @brief Minimum number of Tgc phi hits in EM to consider the segment for seeding */
            Gaudi::Property<unsigned> m_nTgcPhiSeedHitCutEM{this, "minTgcPhiSeedHitsEM", 2};

            const Muon::MuonSectorMapping m_sectorMap{};
    };
}
#endif
