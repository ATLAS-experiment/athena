/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKSTEERING_H
#define MUONTRACKSTEERING_H

#include <mutex>
#include <set>
#include <utility>


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/ToolHandle.h"
#include "Identifier/Identifier.h"
#include "MooCandidateMatchingTool.h"
#include "MooTrackBuilder.h"
#include "MuPatCandidateTool.h"
#include "MuPatPrimitives/MuPatSegment.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonHoleRecoveryTool.h"
#include "MuonRecToolInterfaces/IMuonSegmentFittingTool.h"
#include "MuonRecToolInterfaces/IMuonTrackBuilder.h"
#include "MuonRecToolInterfaces/IMuonTrackFinder.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentInOverlapResolvingTool.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonTrackSelectorTool.h"
#include "MuonTrackSteeringStrategy.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
namespace Muon {  
    class MuPatTrack;
}  // namespace Muon

namespace Muon {

    typedef std::vector<const Muon::MuonSegment*> MuonSegmentCollection;

    /**
        Implementation of an IMuonTrackFinder.

        For more details look at the mainpage of this package.
    */
    class MuonTrackSteering :  public  extends <AthAlgTool , IMuonTrackFinder> {
    public:
        typedef std::vector<MuPatTrack*> TrkVec;
        typedef std::vector<Trk::Track*> TrkCollVec;
        typedef std::vector<MuPatSegment*> SegCol;
        typedef std::vector<MuPatSegment*>::iterator SegColIt;
        /** @brief Helper container to sort the segments by station index */
        using StSegCol_t = std::array<SegCol, MuonStationIndex::toInt(MuonStationIndex::StIndex::StIndexMax)>;
        /** @brief Helper container to sort the segments by chamber index */
        using ChSegCol_t = std::array<SegCol, MuonStationIndex::toInt(MuonStationIndex::ChIndex::ChIndexMax)>;
        /** @brief  */
        using SegColVec_t = std::vector<SegCol>;
        typedef std::set<MuonStationIndex::ChIndex> ChSet;
        typedef ChSet::iterator ChIt;
        typedef ChSet::const_iterator ChCit;

        typedef std::set<MuonStationIndex::StIndex> StSet;
        typedef StSet::iterator StIt;
        typedef StSet::const_iterator StCit;

    public:
        /** default AlgTool constructor */
        MuonTrackSteering(const std::string&, const std::string&, const IInterface*);

        /** destructor */
        virtual ~MuonTrackSteering() = default;

        /** initialize method, method taken from bass-class AlgTool */
        virtual StatusCode initialize() override;

        /** @brief find tracks starting from a MuonSegmentCollection
            @param coll a reference to a MuonSegmentCollection
            @return a pointer to a vector of tracks, the ownership of the tracks is passed to the client calling the tool.
        */
       std::unique_ptr<TrackCollection> find(const EventContext& ctx, const MuonSegmentCollection& coll) const override;

    private:
        struct GarbageContainer {
            void push_back(std::unique_ptr<MuonSegment> seg) {seg_garbage.push_back(std::move(seg));}
            void push_back(std::unique_ptr<MuPatSegment> seg) {pat_garbage.push_back(std::move(seg));}
            void reserve(size_t N) {
                pat_garbage.reserve(N);
                seg_garbage.reserve(N);
            }
            std::vector<std::unique_ptr<MuPatSegment>> pat_garbage{};
            std::vector<std::unique_ptr<MuonSegment>> seg_garbage{};            
        };
        std::unique_ptr<TrackCollection> selectTracks(std::vector<std::unique_ptr<MuPatTrack>>& candidates, bool takeOwnership = true) const;

        /** actual find method */
        std::unique_ptr<TrackCollection> findTracks(const EventContext& ctx, ChSegCol_t& chamberSegments, StSegCol_t& stationSegments) const;
        bool extractSegments(const EventContext& ctx, const MuonSegmentCollection& coll, ChSegCol_t& chamberSegments, StSegCol_t& stationSegments,
                             ChSet& chambersWithSegments, StSet& stationsWithSegments, GarbageContainer& trash_bin) const;

        StatusCode decodeStrategyVector(const std::vector<std::string>& strategy);
        std::unique_ptr<const MuonTrackSteeringStrategy> decodeStrategy(const std::string& strategy) const;
        static bool decodeList(const std::string& input, std::vector<std::string>& list) ;

        std::vector<std::unique_ptr<MuPatTrack>> extendWithLayer(const EventContext& ctx, MuPatTrack& candidate, const SegColVec_t& segcol, unsigned int nextlayer,
                                                                 const unsigned int endlayer, int cutLevel = 0) const;
        /** @brief Find tracks starting from a good segment
            @param seedSeg the seeding MuonSegment pointer
            @param strat the current track finding strategy
            @param layer the current layer for the seed
        */
        std::vector<std::unique_ptr<MuPatTrack>> findTrackFromSeed(const EventContext& ctx, MuPatSegment& seedSeg, const MuonTrackSteeringStrategy& strat,
                                                                   const unsigned int layer, const SegColVec_t& segs) const;

        void refineTracks(const EventContext& ctx, std::vector<std::unique_ptr<MuPatTrack>>& candidates) const;

        /** @brief Resolve ambiguities among tracks for a single strategy
                   This allows a strategy-specific ambiguity solving (with some options per strategy)
            @param vector of tracks that were found
            @param strat the steering strategy
        */
        void solveAmbiguities(std::vector<std::unique_ptr<MuPatTrack>>& tracks, const MuonTrackSteeringStrategy* strat = nullptr) const;

        void combineOverlapSegments(const EventContext& ctx, std::vector<MuPatSegment*>& ch1, std::vector<MuPatSegment*>& ch2, StSegCol_t& stationSegments,
                                    StSet& stationsWithSegments, GarbageContainer& trash_bin) const;

    private:
        ServiceHandle<IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                        "Handle to the service providing the IMuonEDMHelperSvc interface"};
        PublicToolHandle<MuonEDMPrinterTool> m_printer{this, "MuonPrinterTool", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
        ToolHandle<MuPatCandidateTool> m_candidateTool{this, "MuPatCandidateTool", "Muon::MuPatCandidateTool/MuPatCandidateTool"};
        ToolHandle<IMuonTrackBuilder> m_trackBTool{this, "TrackBuilderTool", "Muon::MooTrackBuilder/MooMuonTrackBuilder"};
        ToolHandle<Trk::ITrackAmbiguityProcessorTool> m_ambiTool{this, "AmbiguityTool",
                                                                 "Trk::TrackSelectionProcessorTool/MuonAmbiProcessor"};
        ToolHandle<MooTrackBuilder> m_mooBTool{this, "MooBuilderTool", "Muon::MooTrackBuilder/MooMuonTrackBuilder"};
        ToolHandle<MooCandidateMatchingTool> m_candidateMatchingTool{this, "CandidateMatchingTool",
                                                                     "Muon::MooCandidateMatchingTool/MooCandidateMatchingTool"};
        ToolHandle<IMuonTrackRefiner> m_trackRefineTool{this, "TrackRefinementTool", "Muon::MooTrackBuilder/MooMuonTrackBuilder"};
        ToolHandle<IMuonSegmentFittingTool> m_segmentFitter{this, "MuonSegmentFittingTool",
                                                            "Muon::MuonSegmentFittingTool/MuonSegmentFittingTool"};
        ToolHandle<Muon::MuonTrackSelectorTool> m_trackSelector{this, "MuonTrackSelector",
                                                                "Muon::MuonTrackSelectorTool/MuonTrackSelectorTool"};
        ToolHandle<IMuonHoleRecoveryTool> m_muonHoleRecoverTool{this, "HoleRecoveryTool",
                                                                "Muon::MuonChamberHoleRecoveryTool/MuonChamberHoleRecoveryTool"};
        ToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{this, "TrackSummaryTool", "MuonTrackSummaryTool"};

        std::vector<std::unique_ptr<const MuonTrackSteeringStrategy>> m_strategies;
        std::vector<std::string> m_stringStrategies;

        std::array<int, 3> m_segQCut{};  //!< Required segment quality for seed, 2nd, and other segments
        bool m_outputSingleStationTracks;
        bool m_combinedSLOverlaps;
        bool m_doSummary;
        bool m_useTightMatching;
        bool m_onlyMDTSeeding;
        int m_segThreshold;
    };

}  // namespace Muon

#endif
