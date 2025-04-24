/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"

namespace MuonR4 {

    std::ostream& operator<<(std::ostream& ostr, const MuonR4::MsTrackSeed& seed) {
      ostr<<"Seed @"<<Amg::toString(seed.position())<<std::endl;
      for (const xAOD::MuonSegment* seg : seed.segments()) {
        ostr<<"  **** "<<detailedSegment(*seg)->msSector()->identString()<<" "
            <<Amg::toString(seg->position())<<" + "<<Amg::toString(seg->direction())
            <<", chi2: "<<(seg->chiSquared() / seg->numberDoF())
            <<", nPrec: "<<seg->nPrecisionHits()<<", nPhi: "<<seg->nPhiLayers()
            <<", nTrigEta: "<<seg->nTrigEtaLayers()<<std::endl;
      }
      return ostr;
    }
    bool MsTrackSeed::operator==(const MsTrackSeed& other) const{
      if (!compatibleSectors(msSector(), other.msSector())) {
        return false;
      }
      if (other.segments().size() != m_segments.size()){
            return false;
      }
      for (size_t s = 0; s < m_segments.size(); ++s) {
        if (m_segments[s] != other.segments()[s]) {
            return false;
        }
      }
      return true;
    }
    bool MsTrackSeed::operator<(const MsTrackSeed& other) const {      
      if (!compatibleSectors(msSector(), other.msSector()) ||
          other.segments().size() < segments().size()) {
        return false;
      }
      auto searched_itr = other.segments().begin();
      for (const xAOD::MuonSegment* seg : m_segments) {
        searched_itr = std::find(searched_itr, other.segments().end(), seg);
        if (searched_itr == other.segments().end()) {
            return false; 
        }  
      }
      return true;
    }
    MsTrackSeed::MsTrackSeed(const Location loc): m_loc{loc}{}
    MsTrackSeed::Location MsTrackSeed::location() const { return m_loc; }
    const std::unordered_set<const SpacePointBucket*>& MsTrackSeed::buckets() const { return m_buckets; }
    const std::vector<const xAOD::MuonSegment*>& MsTrackSeed::segments() const { return m_segments; }
    const std::vector<const Segment*>& MsTrackSeed::detailedSegments() const { return m_detSegments; }

    void MsTrackSeed::addSegment(const xAOD::MuonSegment* seg) {
        const Segment* recoSeg = detailedSegment(*seg);
        auto insert_itr = std::ranges::find_if(m_detSegments, [recoSeg](const Segment* added){
            return recoSeg->position().mag2() < added->position().mag2();
        });
        m_segments.insert(m_segments.begin() + std::distance(m_detSegments.begin(), insert_itr), seg);
        m_buckets.insert(recoSeg->parent()->parentBucket());
        m_detSegments.insert(insert_itr, recoSeg);
    }
    const Amg::Vector3D& MsTrackSeed::position() const { return m_pos; }
    void MsTrackSeed::setPosition(Amg::Vector3D&& pos) { m_pos = std::move(pos); }
    const MuonGMR4::SpectrometerSector* MsTrackSeed::msSector() const {
      return m_detSegments.size() ? m_detSegments.front()->msSector() : nullptr;
   }
   bool MsTrackSeed::compatibleSectors(const MuonGMR4::SpectrometerSector* secA,
                                       const MuonGMR4::SpectrometerSector* secB) {
      const unsigned int secMax = Muon::MuonStationIndex::numberOfSectors();
      return  secA->side() == secB->side() && 
      std::abs(secA->sector() - secB->sector()) % secMax <= 1;
    }
}
 