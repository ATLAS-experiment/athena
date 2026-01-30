/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_TRACKSUMMARYTOOL_H
#define MUONTRACKFINDINGTOOLS_TRACKSUMMARYTOOL_H

#include "GeoPrimitives/GeoPrimitives.h"
////
#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonTrackEvent/HitSummary.h"
#include "MuonTrackEvent/MsTrackSeed.h"

#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"
#include "xAODTracking/TrackParticle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "TrkTrack/Track.h" 

namespace MuonR4 {
    class TrackSummaryTool : public extends <AthAlgTool, ITrackSummaryTool> {
        public:
            using base_class::base_class;

            virtual StatusCode initialize() override final;

            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const ConstTrack_t trackProxy) const override final;

            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const std::vector<const xAOD::MuonSegment*>& segments) const override final;
            
            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const Trk::Track& track) const override final;
         
            virtual void copySummary(const HitSummary& summary,
                                     const xAOD::IParticle& track) const override final;
        private:
            using Stat_t = HitSummary::Status;
            /** @brief Increments the hit summary based on the identifier extracted from the measurement
             *  @param hitId: Identifier from the measurement or the hole for which the summary needs to be incremented
             *  @param status: Contribution of the measurement towards the track fit
             *  @param prdDim: Dimension of the measurement 0 -> combined measurement (eta + phi), 2 (eta +phi) 
             * @param summary: Reference to the summary where the counts shall be incremented */
            void incrementSummary(const Identifier& hitId,
                                  const Stat_t status,
                                  const unsigned prdDim,
                                  HitSummary& summary) const;
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Toggle whether holes states shall be copied to the xAOD object */
            Gaudi::Property<bool> m_fillHoles{this, "fillHoles", true};
            /** @brief Toggle whether outlier states shall be copied to the xAOD object */
            Gaudi::Property<bool> m_fillOutliers{this, "fillOutliers", true};
            /** @brief Toggle whether the segment summary shall be recomputed */
            Gaudi::Property<bool> m_reDoSegments{this, "recomputeSegment", true};
    };
}
#endif