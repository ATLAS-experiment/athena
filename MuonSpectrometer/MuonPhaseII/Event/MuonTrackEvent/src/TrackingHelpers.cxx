/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace{
  using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
  using PrdLinkVec_t = std::vector<PrdLink_t>;
}

namespace MuonR4{
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
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const Segment& seg,
                                                                          bool skipOutlier) {
      std::vector<const xAOD::UncalibratedMeasurement*> out{};
      out.reserve(seg.measurements().size()*2);
      for (const auto& meas : seg.measurements()){
        /// remove all the garbage
        if (skipOutlier && meas->fitState() != CalibratedSpacePoint::State::Valid) {
          continue;
        }
        // Remove the external constraints
        const SpacePoint* sp = meas->spacePoint();
        if (!sp) {
          continue;
        }
        out.emplace_back(sp->primaryMeasurement());
        if (sp->secondaryMeasurement()) {
          out.emplace_back(sp->secondaryMeasurement());
        }
      }
      return out;
    }
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const xAOD::MuonSegment& segment,
                                                                          bool skipOutlier) {
        static const SG::ConstAccessor<PrdLinkVec_t> acc_prdLinks{"prdLinks"};
        static const SG::ConstAccessor<std::vector<char>> acc_prdState{"prdState"};
        std::vector<const xAOD::UncalibratedMeasurement*> out{};
        const PrdLinkVec_t& links{acc_prdLinks(segment)};
        out.reserve(links.size());
        for (std::size_t l = 0 ; l < links.size(); ++l) {
          const PrdLink_t& link{links[l]};
          if (!skipOutlier || 
              acc_prdState(segment)[l] == Acts::toUnderlying(CalibratedSpacePoint::State::Valid)) {
            out.push_back(*link);
          
          }
        }
        return out;
    }
}