/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/TrkPrepRawDataSurfaceAcc.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
namespace ActsTrk::detail{
    TrkPrepRawDataSurfaceAcc::TrkPrepRawDataSurfaceAcc(const IGeometryRealmConvTool* trkConvTool):
    m_trkConvTool{trkConvTool} {}
    const Acts::Surface* TrkPrepRawDataSurfaceAcc::operator()(const Acts::SourceLink& sourceLink) const {
        const auto* meas = TrkPrepRawDataCalibrator::unpack(sourceLink);
        assert(m_trkConvTool != nullptr);
        return m_trkConvTool->convertSurfaceToActs(meas->detectorElement()->surface(meas->identify())).get();
    }
}