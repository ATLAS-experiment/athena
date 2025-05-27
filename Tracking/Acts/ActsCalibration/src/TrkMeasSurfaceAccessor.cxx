/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibration/TrkMeasSurfaceAccessor.h"
#include "ActsCalibration/TrkMeasurementCalibrator.h"
namespace ActsTrk::detail {
    TrkMeasSurfaceAccessor::TrkMeasSurfaceAccessor(const IActsToTrkConverterTool* trkConvTool):
        m_trkConvTool{trkConvTool} {}
    const Acts::Surface* TrkMeasSurfaceAccessor::operator()(const Acts::SourceLink& sourceLink) const {
        const auto* meas = TrkMeasurementCalibrator::unpack(sourceLink);
        assert(m_trkConvTool != nullptr);
        return &(m_trkConvTool->trkSurfaceToActsSurface(meas->associatedSurface()));
    }
}