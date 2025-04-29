/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
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
}