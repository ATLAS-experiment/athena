/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"

#include "Acts/Utilities/MathHelpers.hpp"

#include <format>

namespace MuonR4 {

    std::ostream& operator<<(std::ostream& ostr, const MuonR4::MsTrackSeed& seed) {
      ostr<<"MS Track seed @"<<Amg::toString(seed.position())<<", sector: "<<seed.sector()<<std::endl;
      using namespace Muon::MuonStationIndex;
      for (const xAOD::MuonSegment* seg : seed.segments()) {
        ostr<<"  **** "<< printID(*seg)
            <<", theta: "<<(seg->direction().theta() /Gaudi::Units::degree)
            <<", phi: "<<(seg->direction().phi() /Gaudi::Units::degree)
            <<", R: "<<Acts::fastHypot(seg->x(), seg->y())
            <<", Z: "<<seg->z()<<" "<<(seg->position().theta() / Gaudi::Units::degree)
            // <<", "<<SegmentFit::toString(SegmentFit::localSegmentPars(*seg))
            <<", chi2: "<<(seg->chiSquared() / std::max(seg->numberDoF(), 1.f))
            <<", nPrec: "<<seg->nPrecisionHits()
            <<", nPhi: "<<seg->nPhiLayers()
            <<", nTrigEta: "<<seg->nTrigEtaLayers()<<std::endl;
      }
      return ostr;
    }
    MsTrackSeed::MsTrackSeed(const Location loc, const int sector): 
          m_loc{loc}, m_sector{sector}{}
    MsTrackSeed::Location MsTrackSeed::location() const { return m_loc; }
    std::vector<const SpacePointBucket*> MsTrackSeed::buckets() const { 
        std::vector<const SpacePointBucket*> returnMe{};
        for (const Segment* seg : detailedSegments()){
            returnMe.push_back(seg->parent()->parentBucket());
        }
        return returnMe;
    }
    
    const std::vector<const xAOD::MuonSegment*>& MsTrackSeed::segments() const { return m_segments; }
    std::vector<const Segment*> MsTrackSeed::detailedSegments() const { 
      std::vector<const Segment*> segs{};
      segs.reserve(segments().size());
      for (const xAOD::MuonSegment* trfMe : segments()) {
         if(const Segment* seg = detailedSegment(*trfMe); seg != nullptr){
            segs.push_back(seg);
         }
      } 
      return segs; 
    }
    void MsTrackSeed::replaceSegment(const xAOD::MuonSegment* exist,
                                     const xAOD::MuonSegment* updated) {
        std::vector<const xAOD::MuonSegment*>::iterator itr = 
              std::ranges::find(m_segments, exist);
        if (itr == m_segments.end()){
            THROW_EXCEPTION("The exisiting segment could not be found");
        }
        (*itr) = updated;
    }
    void MsTrackSeed::addSegment(const xAOD::MuonSegment* seg) {
        const float r2 = Acts::hypotSquare(seg->x(), seg->y(), seg->z());
        auto insert_itr = std::ranges::find_if(m_segments, 
              [&r2](const xAOD::MuonSegment* added){
                return r2 < Acts::hypotSquare(added->x(), added->y(), added->z());
              });
        m_segments.insert(insert_itr, seg);
    }
    const Amg::Vector3D& MsTrackSeed::position() const { return m_pos; }
    void MsTrackSeed::setPosition(Amg::Vector3D&& pos) { m_pos = std::move(pos); }
  
}
 