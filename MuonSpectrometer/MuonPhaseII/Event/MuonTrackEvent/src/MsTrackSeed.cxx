/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"
namespace MuonR4 {
    MsTrackSeed::MsTrackSeed(const Location loc): m_loc{loc}{}
    MsTrackSeed::Location MsTrackSeed::location() const { return m_loc; }
    const std::unordered_set<const SpacePointBucket*>& MsTrackSeed::buckets() const { return m_buckets; }
    const std::vector<const xAOD::MuonSegment*>& MsTrackSeed::segments() const { return m_segments; }
    const std::vector<const Segment*>& MsTrackSeed::detailedSegments() const { return m_detSegments; }

    void MsTrackSeed::addSegment(const xAOD::MuonSegment* seg) {
        m_segments.push_back(seg);
        const Segment* recoSeg = detailedSegment(*seg);
        m_buckets.insert(recoSeg->parent()->parentBucket());
        m_detSegments.push_back(recoSeg);
    }
    const Amg::Vector3D& MsTrackSeed::position() const { return m_pos; }
    void MsTrackSeed::setPosition(Amg::Vector3D&& pos) { m_pos = std::move(pos); }
    const MuonGMR4::SpectrometerSector* MsTrackSeed::msSector() const {
      return m_detSegments.size() ? m_detSegments.front()->msSector() : nullptr;
   }
}
 