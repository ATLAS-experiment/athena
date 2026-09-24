/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"

#include "Acts/Utilities/MathHelpers.hpp"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include <format>

using namespace Muon::MuonStationIndex;
     

namespace MuonR4 {
    std::ostream& MsTrackSeed::print(std::ostream& ostr) const {
      ostr<<"MS Track seed @"<<Amg::toString(position())<<", sector: "<<sector() << ", location: "
          << location()<<", stations: ";
      for (const auto st : m_stations) { ostr<<st<<", "; }
      ostr<<std::endl;
      for (const xAOD::MuonSegment* seg : segments()) {
        ostr<<"  **** "<< printSegment(*seg) <<std::endl;
      }
      return ostr;
    }
    std::string MsTrackSeed::toString(const Location loc) {
        switch (loc) {
          using enum Location;
          case Undefined: return "Undefined";
          case Barrel: return "Barrel";
          case Endcap: return "Endcap";
        }
        return "";
    }
    MsTrackSeed::MsTrackSeed(const Location loc, const ExpandedSector sector): 
          m_loc{loc}, m_sector{sector}{}
    MsTrackSeed::Location MsTrackSeed::location() const { return m_loc; }
    std::vector<const SpacePointBucket*> MsTrackSeed::buckets() const { 
        std::vector<const SpacePointBucket*> returnMe{};
        for (const xAOD::MuonSegment* trfMe : segments()) {
          if(const Segment* seg = detailedSegment(*trfMe); seg != nullptr){
            returnMe.push_back(seg->parent()->parentBucket());
          }
        }
        return returnMe;
    }
    
    std::span<const xAOD::MuonSegment* const> MsTrackSeed::segments() const { return m_segments; }
    void MsTrackSeed::replaceSegment(const xAOD::MuonSegment* exist,
                                     const xAOD::MuonSegment* updated) {
        std::vector<const xAOD::MuonSegment*>::iterator itr = 
              std::ranges::find(m_segments, exist);
        if (itr == m_segments.end()){
            THROW_EXCEPTION("The exisiting segment could not be found");
        }
        (*itr) = updated;
    }
    std::span<const Muon::MuonStationIndex::StIndex> MsTrackSeed::stations() const {
        return m_stations;
    }
    void MsTrackSeed::addSegment(const xAOD::MuonSegment* seg) {
        const float path = pathLength(*seg);
        auto insert_itr = std::ranges::find_if(m_segments, 
              [&](const xAOD::MuonSegment* added){
                return path < pathLength(*added);
              });
        m_segments.insert(insert_itr, seg);
        m_nMeasurements += MuonR4::nMeasurements(*seg);
        if (!Acts::rangeContainsValue(m_stations, toStationIndex(seg->chamberIndex()))) {
            m_stations.push_back(toStationIndex(seg->chamberIndex()));
        }
    }
    std::size_t MsTrackSeed::nStations() const { return m_stations.size(); }
    std::size_t MsTrackSeed::nMeasurements() const { return m_nMeasurements; }
    const Amg::Vector3D& MsTrackSeed::position() const { return m_pos; }
    void MsTrackSeed::setPosition(Amg::Vector3D&& pos) { m_pos = std::move(pos); }
    float MsTrackSeed::pathLength(const xAOD::MuonSegment& segment) const {
        if (m_segments.empty()) {
          return 0.;
        }
        const xAOD::MuonSegment& ref{*m_segments.front()};
        return (segment.x() - ref.x())* ref.px() +
               (segment.y() - ref.y())* ref.py() +
               (segment.z() - ref.z())* ref.pz();
    }

    void MsTrackSeed::prepareOverlap(std::shared_ptr<std::uint8_t> marker) {
       m_overlapMarker.emplace_back(std::move(marker));
    }
           
    void MsTrackSeed::triggerOverlapMarker() {
      std::ranges::for_each(m_overlapMarker, [](const auto& m){ (*m) = 1;});
      m_overlapMarker.clear();
    }     
    bool MsTrackSeed::hasOverlap() const {
      return std::ranges::any_of(m_overlapMarker, [](const auto& m ){ return (*m); });
    }
}
 