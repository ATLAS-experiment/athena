/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurement.h"
#include "GeoModelKernel/throwExcept.h"

namespace ActsTrk::detail{
    Acts::SourceLink xAODUncalibMeasCalibrator::pack(const xAOD::UncalibratedMeasurement* meas) {
        return Acts::SourceLink{meas};
    }
    const xAOD::UncalibratedMeasurement* xAODUncalibMeasCalibrator::unpack(const Acts::SourceLink& sl) {
        SourceLink_t meas = sl.template get<SourceLink_t>();
        assert(meas != nullptr);
        return meas;
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
        THROW_EXCEPTION("Calibrator is not configured for measurement type "<<static_cast<int>(unpack(sl)->type()));
    }

    void xAODUncalibMeasCalibrator::auxillaryCalibrator(const Acts::GeometryContext& /*gctx*/,
                                                        const Acts::CalibrationContext& /*cctx*/,
                                                        const Acts::SourceLink& sl,
                                                        MutableTrackStateBackend::TrackStateProxy trackState) const {
        const xAOD::UncalibratedMeasurement* meas = unpack(sl);
        assert(meas->type() == xAOD::UncalibMeasType::Other);

        const auto* pMeas = dynamic_cast<const xAOD::AuxillaryMeasurement*>(meas);
        switch(pMeas->numDimensions()) {
            case 1:
                setState<1, MutableTrackStateBackend>(pMeas->calibProjector(), pMeas->localPosition<1>(),
                            pMeas->localCovariance<1>(), sl, trackState);
                break;
            case 2:
                setState<2, MutableTrackStateBackend>(pMeas->calibProjector(), pMeas->localPosition<2>(),
                            pMeas->localCovariance<2>(), sl, trackState);
                break;
            case 3:
                setState<3, MutableTrackStateBackend>(pMeas->calibProjector(), pMeas->localPosition<3>(),
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
        
        const CalibDelegate& delegate = m_calibrators[static_cast<int>(meas->type())];
        delegate(gctx, cctx, sl, trackState);
    }

}