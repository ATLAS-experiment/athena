/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

namespace MuonR4 {

bool isPrecisionHit(const SpacePoint& hit) {
    using enum xAOD::UncalibMeasType;
    return hit.type() == MdtDriftCircleType || hit.type() == MMClusterType ||
            (hit.type() == sTgcStripType && 
            static_cast<const xAOD::sTgcMeasurement*>(hit.primaryMeasurement())->channelType() ==
            sTgcIdHelper::sTgcChannelTypes::Strip);
}
bool isGoodHit(const CalibratedSpacePoint& hit) {
    using enum CalibratedSpacePoint::State;
    return hit.fitState() == Valid;
}
bool isPrecisionHit(const CalibratedSpacePoint& hit) {
    using enum xAOD::UncalibMeasType;
    if (hit.type() == xAOD::UncalibMeasType::Other){
        return false;
    }
    return isGoodHit(hit) && isPrecisionHit(*hit.spacePoint());
}

}