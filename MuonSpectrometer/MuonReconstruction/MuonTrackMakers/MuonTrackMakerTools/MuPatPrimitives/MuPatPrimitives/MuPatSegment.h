/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUPATSEGMENT_H
#define MUPATSEGMENT_H

#include <mutex>
#include <set>

#include "Identifier/Identifier.h"
#include "MuPatPrimitives/MuPatCandidateBase.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentQuality.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "TrkParameters/TrackParameters.h"

namespace Muon {

    class MuPatTrack;

    /**
        @brief segment candidate object.
        The purpose of the segment candidate is three folded:
        - provide the generic MuPatCandidateBase interface for tracks
        - keep track of tracks the segment is accociated to
        - cache additional information that cannot be stored on the track

        The following information is cached:
        - the segment quality integer calculated by the MuonSegmentSelectionTool
        - the MuonSegmentQuality
        - a AtaPlane that represents the segment and allows extrapolation
        - the index of the segment in the order list of segments in the track steering cache
        - an integer (usedInFit) showing on how many track candidates the segment is included
        - the identifier of the chamber with most hits on the segment
        - a string summarizing the segment that can be used for dumping it to screen
        - a chamber index
        - a station index
        - a flag indicating whether the segment is in the barrel or endcap
        - a flag indicating whether the segment contains MDT hits
     */

    class MuPatSegment : public MuPatCandidateBase, public Trk::ObjectCounter<MuPatSegment> {
    public:
        using ChIdx = MuonStationIndex::ChIndex;
        using StIdx = MuonStationIndex::StIndex;
        MuPatSegment() = default;
        ~MuPatSegment() = default;


        int quality{0};
        const MuonSegmentQuality* segQuality{nullptr};
        const MuonSegment* segment{nullptr};
        std::shared_ptr<const Trk::AtaPlane> segPars{nullptr};
        int segmentIndex{-1};  //!< index of segment within station
        int usedInFit{0};
        Identifier chid{0};
        std::string name{};
        ChIdx chIndex{ChIdx::ChUnknown};
        StIdx stIndex{StIdx::StUnknown};
        bool isEndcap{false};
        bool isMdt{false};  //!< true for MDT, false for CSC

        /** @brief returns first track parameters */
        const Trk::TrackParameters& entryPars() const;

        /** @brief add a new track to the segment */
        void addTrack(MuPatTrack*);

        /** @brief remove a track from the segment */
        void removeTrack(MuPatTrack*);

        /** access to the tracks the segment is associated with */
        const std::set<MuPatTrack*>& tracks() const { return m_associatedTracks; }

    private:
        std::set<MuPatTrack*> m_associatedTracks;  //<! list of associated tracks

    };  // class MuPatSegment
    
    //
    // inline member functions implementations
    //
    inline const Trk::TrackParameters& MuPatSegment::entryPars() const { return *segPars; }

    struct SortSegInfoByR {
        bool operator()(const MuPatSegment* c1, const MuPatSegment* c2) { return operator()(*c1, *c2); }
        bool operator()(const MuPatSegment& c1, const MuPatSegment& c2) {
            return c1.segment->globalPosition().perp() < c2.segment->globalPosition().perp();
        }
    };
    struct SortSegInfoByZ {
        bool operator()(const MuPatSegment* c1, const MuPatSegment* c2) { return operator()(*c1, *c2); }
        bool operator()(const MuPatSegment& c1, const MuPatSegment& c2) {
            return fabs(c1.segment->globalPosition().z()) < fabs(c2.segment->globalPosition().z());
        }
    };
    struct SortSegInfoByQuality {
        bool operator()(const MuPatSegment* c1, const MuPatSegment* c2) { return operator()(*c1, *c2); }
        bool operator()(const MuPatSegment& c1, const MuPatSegment& c2) { return c1.quality > c2.quality; }
    };

}  // namespace Muon

#endif
