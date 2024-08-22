/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKSCORINGTOOL_H
#define MUONTRACKSCORINGTOOL_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "TrkEventPrimitives/TrackScore.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"

namespace Trk {
    class Track;
    class TrackSummary;
}  // namespace Trk

namespace Muon {

    /**Concrete implementation of the ITrackScoringTool pABC*/
    class MuonTrackScoringTool : virtual public Trk::ITrackScoringTool, public AthAlgTool {
    public:
        MuonTrackScoringTool(const std::string&, const std::string&, const IInterface*);
        virtual ~MuonTrackScoringTool() = default;
        virtual StatusCode initialize() override;
        /** check track selections independent from TrackSummary */
        virtual bool passBasicSelections( const Trk::Track& ) const override {return true;}

        /** create a score based on how good the passed track is*/
        virtual Trk::TrackScore score(const Trk::Track& track, bool checkBasicSel) const override;

        /** create a score based on how good the passed TrackSummary is*/
        virtual Trk::TrackScore simpleScore(const Trk::Track& track, const Trk::TrackSummary& trackSummary) const override;

    private:
        /**\todo make this const, once createSummary method is const*/
        ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool{this, "SumHelpTool", "Trk::TrackSummaryTool"};
        PublicToolHandle<MuonEDMPrinterTool> m_printer{this, "EDMPrinter", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
                                                       "helper to nicely print out tracks"};

        /**holds the scores assigned to each Trk::SummaryType from the track's Trk::TrackSummary*/
        std::vector<Trk::TrackScore> m_summaryTypeScore;
    };

}  // namespace Muon
#endif
