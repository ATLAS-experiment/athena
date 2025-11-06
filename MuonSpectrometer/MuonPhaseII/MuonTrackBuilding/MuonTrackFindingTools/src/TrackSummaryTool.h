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

namespace MuonR4 {
    class TrackSummaryTool : public extends <AthAlgTool, ITrackSummaryTool> {
        public:
            using base_class::base_class;

            virtual StatusCode initialize() override final;

            virtual HitSummary makeSummary(const EventContext& ctx,
                                                const ConstTrack_t trackProxy) const override final;

            virtual HitSummary makeSummary(const EventContext& ctx,
                                           const MsTrackSeed& seed) const override final;
            
             virtual void copySummary(const HitSummary& summary,
                                      xAOD::IParticle& track) const override final;
        private:
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    };
}
#endif