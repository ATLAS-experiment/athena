/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurement.h"
#include "GeoModelKernel/throwExcept.h"

namespace ActsTrk::detail{
    const xAOD::UncalibratedMeasurement* xAODUncalibMeasCalibrator::unpack(const Acts::SourceLink& sl) {
        const SourceLink_t& meas = sl.template get<SourceLink_t>();
        return std::holds_alternative<const xAOD::UncalibratedMeasurement*>(meas) ?
               std::get<const xAOD::UncalibratedMeasurement*>(meas) : nullptr;
    }
    xAODUncalibMeasCalibrator::xAODUncalibMeasCalibrator() {
        for (std::size_t t =0 ; t < m_calibrators.size(); ++t) {
            const auto mType = static_cast<xAOD::UncalibMeasType>(t);
            connect<&xAODUncalibMeasCalibrator::invalidCalibrator>(mType, this);
        }
        connect<&xAODUncalibMeasCalibrator::auxillaryCalibrator>(xAOD::UncalibMeasType::Other, this);
    }

    void xAODUncalibMeasCalibrator::invalidCalibrator(const Acts::GeometryContext& /*gctx*/,
                                                      const Acts::CalibrationContext& /*cctx*/,
                                                      const Acts::SourceLink& sl,
                                                      MutableTrackStateBackend::TrackStateProxy /*trackState*/) const {
        THROW_EXCEPTION("Calibrator is not configured for measurement type "<<unpack(sl)->type());
    }

    void xAODUncalibMeasCalibrator::auxillaryCalibrator(const Acts::GeometryContext& /*gctx*/,
                                                        const Acts::CalibrationContext& /*cctx*/,
                                                        const Acts::SourceLink& sl,
                                                        MutableTrackStateBackend::TrackStateProxy trackState) const {
        const xAOD::UncalibratedMeasurement* meas = unpack(sl);
        assert(meas->type() == xAOD::UncalibMeasType::Other);

        const auto* pMeas = dynamic_cast<const xAOD::AuxiliaryMeasurement*>(meas);
        switch(pMeas->numDimensions()) {
            case 1:
                setState<1>(pMeas->calibProjector(), pMeas->localPosition<1>(),
                            pMeas->localCovariance<1>(), sl, trackState);
                break;
            case 2:
                setState<2>(pMeas->calibProjector(), pMeas->localPosition<2>(),
                            pMeas->localCovariance<2>(), sl, trackState);
                break;
            case 3:
                setState<3>(pMeas->calibProjector(), pMeas->localPosition<3>(),
                            pMeas->localCovariance<3>(), sl, trackState);
                break;
            default:
                break;
        }

    }
    void xAODUncalibMeasCalibrator::calibrate(const Acts::GeometryContext &gctx,
                                              const Acts::CalibrationContext & cctx,
                                              const Acts::SourceLink& sl,
                                              const MutableTrackStateBackend::TrackStateProxy trackState) const {
        const xAOD::UncalibratedMeasurement* meas = unpack(sl);
        assert(meas->type() != xAOD::UncalibMeasType::nTypes);
        
        const CalibDelegate& delegate = m_calibrators[Acts::toUnderlying(meas->type())];
        delegate(gctx, cctx, sl, trackState);
    }

}