/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/RefittingCalibrator.h"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

namespace ActsTrk::detail {

RefittingCalibrator::RefittingCalibrator(const ActsTrk::IActsToTrkConverterTool* convTool,
                                          const Trk::IRIO_OnTrackCreator* rotCreator):
    m_prdCalibrator{convTool, rotCreator}{}

void RefittingCalibrator::calibrate(const Acts::GeometryContext& gctx,
                                    const Acts::CalibrationContext& cctx,
                                    const Acts::SourceLink& sourceLink,
                                    MutableTrackStateProxy trackState) const {

    switch (MeasurementCalibratorBase::getType(sourceLink)) {
        using enum SourceLinkType;
        case TrkMeasurement:  
            m_measCalibrator.calibrate(gctx, cctx, sourceLink, trackState);
            break;
        case TrkPrepRawData:
            m_prdCalibrator.calibrate(gctx, cctx, sourceLink, trackState);
            break;
        case xAODUnCalibMeas:
            m_xAODCalibrator.calibrate(gctx, cctx, sourceLink, trackState);
            break;
        default:
          THROW_EXCEPTION("Unsupported source link type "<<MeasurementCalibratorBase::getType(sourceLink));
    }               
}


//##########################################################################
//                      RefittingSurfaceAccesor
//##########################################################################
RefittingSurfaceAccesor::RefittingSurfaceAccesor(const IActsToTrkConverterTool* trkConvTool,
                                                 const ITrackingGeometryTool* trackGeoTool):
      m_xAODAcc{trackGeoTool}, m_prdAcc{trkConvTool}, m_rotAcc{trkConvTool} {}

const Acts::Surface* RefittingSurfaceAccesor::operator()(const Acts::SourceLink& sourceLink) const {
      switch (MeasurementCalibratorBase::getType(sourceLink)) {
        using enum SourceLinkType;
        case TrkMeasurement: return m_rotAcc(sourceLink);
        case TrkPrepRawData: return m_prdAcc(sourceLink);
        case xAODUnCalibMeas: return m_xAODAcc(sourceLink);
        default:
          THROW_EXCEPTION("Unsupported source link type "<<MeasurementCalibratorBase::getType(sourceLink));
    }
    return nullptr;
  }


}  // namespace ActsTrk::detail
