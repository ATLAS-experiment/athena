/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsCalibrators/SourceLinkHash.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
#include "ActsCalibrators/TrkMeasurementCalibrator.h"

#include <cassert>
namespace ActsTrk::detail {

std::size_t sourceLinkHash(const Acts::SourceLink &slink) {
    using namespace ActsTrk::detail;
    switch(MeasurementCalibratorBase::getType(slink)) {
        using enum SourceLinkType;
        case xAODUnCalibMeas:
            return xAODUncalibMeasCalibrator::unpack(slink)->identifier();
        case TrkMeasurement: {
            const auto* rio = dynamic_cast<const Trk::RIO_OnTrack*>(TrkMeasurementCalibrator::unpack(slink));
            assert(rio != nullptr);
            return rio->identify().get_compact();
        } case TrkPrepRawData: {
            return TrkPrepRawDataCalibrator::unpack(slink)->identify().get_compact();
        } default:
            break;
    }
    return 0ul;
}

bool sourceLinkEquality(const Acts::SourceLink &a, const Acts::SourceLink &b) {
    return sourceLinkHash(a) == sourceLinkHash(b);
}

}