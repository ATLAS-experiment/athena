/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace{
  using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
  using PrdLinkVec_t = std::vector<PrdLink_t>;
  static const SG::ConstAccessor<PrdLinkVec_t> acc_prdLinks{"prdLinks"};
  static const SG::ConstAccessor<std::vector<char>> acc_prdState{"prdState"};
}

namespace MuonR4{

   std::string printID(const xAOD::MuonSegment& seg) {
        using namespace Muon::MuonStationIndex;
        return std::format("{:}{:}{:}{:}", chName(seg.chamberIndex()),
                                                  std::abs(seg.etaIndex()),
                                                  seg.etaIndex() > 0 ? 'A' : 'C',
                                                  seg.sector());
    }
    const Segment* detailedSegment(const xAOD::MuonSegment& seg) {
        using SegLink_t = ElementLink<SegmentContainer>;
        static const SG::ConstAccessor<SegLink_t> acc{"parentSegment"};
        if (acc.isAvailable(seg)){
            const SegLink_t& link{acc(seg)};
            if (link.isValid()){
                return *link;
            }
        }
        return nullptr;
    }
    
    std::size_t nMeasurements(const xAOD::MuonSegment& segment) {
      return acc_prdLinks.isAvailable(segment) ? acc_prdLinks(segment).size() : 0;
    }
    const xAOD::UncalibratedMeasurement* getMeasurement(const xAOD::MuonSegment& segment,
                                                        const std::size_t n) {
      if (!acc_prdLinks.isAvailable(segment)) {
          return nullptr;
      }
      assert(n < nMeasurements(segment));
      const PrdLink_t& link{acc_prdLinks(segment)[n]};
      return link.isValid() ? *link : nullptr;
    } 
    bool isOutlierMeasurement(const xAOD::MuonSegment& segment,
                              const std::size_t n) {
        if(!acc_prdState.isAvailable(segment)) {
            return false;
        }
        assert (n < acc_prdState(segment).size());
        return acc_prdState(segment)[n] != Acts::toUnderlying(CalibratedSpacePoint::State::Valid);
    }
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const xAOD::MuonSegment& segment,
                                                                          bool skipOutlier) {
        std::vector<const xAOD::UncalibratedMeasurement*> out{};
        const PrdLinkVec_t& links{acc_prdLinks(segment)};
        out.reserve(links.size());
        for (std::size_t l = 0 ; l < links.size(); ++l) {
            const PrdLink_t& link{links[l]};
            if (!skipOutlier || !isOutlierMeasurement(segment, l)) {
                out.push_back(*link);
            }
        }
        return out;
    }
}