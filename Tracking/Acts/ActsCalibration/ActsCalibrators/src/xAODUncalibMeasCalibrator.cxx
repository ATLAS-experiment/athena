/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsInterop/UnitConverters.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurement.h"
#include "xAODMuonPrepData/MuonMeasurement.h"

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

    void xAODUncalibMeasCalibrator::invalidCalibrator(const Acts::GeometryContext& /*tgContext*/,
                                                      const Acts::CalibrationContext& /*calContext*/,
                                                      const Acts::SourceLink& sl,
                                                      MutableTrackStateBackend::TrackStateProxy /*trackState*/) const {
        THROW_EXCEPTION("Calibrator is not configured for measurement type "<<unpack(sl)->type());
    }

    void xAODUncalibMeasCalibrator::passThrough(const Acts::GeometryContext& tgContext,
                                                const Acts::CalibrationContext& calContext,
                                                const Acts::SourceLink& sl,
                                                MutableTrackStateBackend::TrackStateProxy trackState) const{
        const xAOD::UncalibratedMeasurement* meas = unpack(sl);
        Acts::SourceLink fillSl{sl};
        assert(meas != nullptr);
        switch(meas->type()) {
            using enum xAOD::UncalibMeasType;
            case PixelClusterType: {
                setState<2>(ProjectorType::e2DimNoTime, meas->localPosition<2>(), meas->localCovariance<2>(), fillSl, trackState);
                break;
            } case StripClusterType:{
                auto projector = trackState.referenceSurface().bounds().type() == Acts::SurfaceBounds::eAnnulus ? 
                                  ProjectorType::e1DimRotNoTime : ProjectorType::e1DimNoTime;
                setState<1>(projector, meas->localPosition<1>(), meas->localCovariance<1>(), fillSl, trackState);
                break;
            } case MdtDriftCircleType:
              case MMClusterType: {
                setState<1>(ProjectorType::e1DimNoTime, meas->localPosition<1>(), meas->localCovariance<1>(), fillSl, trackState);
                break;
            } case RpcStripType:
              case TgcStripType:
              case sTgcStripType: {
                if (meas->numDimensions() == 2 || 
                    meas->numDimensions() == 0) {
                    setState<2>(ProjectorType::e2DimNoTime, meas->localPosition<2>(), meas->localCovariance<2>(), fillSl, trackState);
                } else {
                  const auto* mMeas = static_cast<const xAOD::MuonMeasurement*>(meas);
                  auto projector = mMeas->measuresPhi() ?  ProjectorType::e1DimRotNoTime : ProjectorType::e1DimNoTime;
                  setState<1>(projector, meas->localPosition<1>(), meas->localCovariance<1>(), fillSl, trackState);
                }
                break;
            } case HGTDClusterType: {
                xAOD::MeasVector<3> pos = meas->localPosition<3>();
                xAOD::MeasMatrix<3> cov = meas->localCovariance<3>();
                pos[2] = ActsTrk::timeToActs(pos[2]);
                cov(2,2) = ActsTrk::timeCovToActs(cov(2, 2));
                setState<3>(ProjectorType::e2DimWithTime, pos, cov, fillSl, trackState);
                break;
            } case Other :{
                auxillaryCalibrator(tgContext, calContext, sl, trackState);
                break;
            } default:
                THROW_EXCEPTION("Unsupported type "<<meas->type())
        }
    }

    void xAODUncalibMeasCalibrator::auxillaryCalibrator(const Acts::GeometryContext& /*tgContext*/,
                                                        const Acts::CalibrationContext& /*calContext*/,
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
    void xAODUncalibMeasCalibrator::calibrate(const Acts::GeometryContext &tgContext,
                                              const Acts::CalibrationContext& calContext,
                                              const Acts::SourceLink& sl,
                                              const MutableTrackStateBackend::TrackStateProxy trackState) const {
        const xAOD::UncalibratedMeasurement* meas = unpack(sl);
        assert(meas->type() != xAOD::UncalibMeasType::nTypes);
        
        const CalibDelegate& delegate = m_calibrators[Acts::toUnderlying(meas->type())];
        delegate(tgContext, calContext, sl, trackState);
    }

}