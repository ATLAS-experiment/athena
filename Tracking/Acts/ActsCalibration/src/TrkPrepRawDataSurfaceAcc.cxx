/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibration/TrkPrepRawDataSurfaceAcc.h"
#include "ActsCalibration/TrkPrepRawDataCalibrator.h"
namespace ActsTrk::detail{
    TrkPrepRawDataSurfaceAcc::TrkPrepRawDataSurfaceAcc(const IActsToTrkConverterTool* trkConvTool):
    m_trkConvTool{trkConvTool} {}
    const Acts::Surface* TrkPrepRawDataSurfaceAcc::operator()(const Acts::SourceLink& sourceLink) const {
        const auto* meas = TrkPrepRawDataCalibrator::unpack(sourceLink);
        assert(m_trkConvTool != nullptr);
        return &(m_trkConvTool->trkSurfaceToActsSurface(meas->detectorElement()->surface(meas->identify())));
    }
}