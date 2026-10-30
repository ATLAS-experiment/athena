/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FitterTool.h"

#include "GeoModelKernel/throwExcept.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsEvent/TrackContainerUtils.h"
#include "ActsInterop/Logger.h"

using namespace ActsTrk::detail;
namespace MuonR4 {
    StatusCode FitterTool::initialize() {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_muonCalibrator.retrieve());
        m_unalibMeasSurfAcc = xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()};
        for (const xAOD::UncalibMeasType type : xAOD::muonMeasurementTypes()) {
            m_uncalibMeasCalibrator.connect<&MuonR4::ISpacePointCalibrator::calibrateSourceLink>(type, m_muonCalibrator.get());
        }
        
        m_extensions.surfaceAccessor.connect<&ActsTrk::detail::xAODUncalibMeasSurfAcc::operator()>(&m_unalibMeasSurfAcc);
        m_extensions.calibrator.connect<&ActsTrk::detail::xAODUncalibMeasCalibrator::calibrate>(&m_uncalibMeasCalibrator);
        auto logger = makeActsAthenaLogger(this, "MuonTrackFit");

        Acts::Navigator::Config navConfig{m_trackingGeometrySvc->trackingGeometry()};
        Acts::Navigator navigator(std::move(navConfig), logger->cloneWithSuffix("Navigator"));
        if (!m_doStraightLineFit) {
            CurvedPropagator_t::Stepper stepper{std::make_shared<ATLASMagneticFieldWrapper>()};
            CurvedPropagator_t propagator{std::move(stepper), 
                                         std::move(navigator), logger->cloneWithSuffix("Prop")};
            m_fitter = CurvedFitter_t{std::move(propagator), logger->cloneWithSuffix("Fitter")};
        } else {
           StraightPropagator_t propagator{Acts::StraightLineStepper{}, 
                                            std::move(navigator), logger->cloneWithSuffix("Prop")};
            m_fitter = StraightFitter_t{std::move(propagator), logger->cloneWithSuffix("Fitter")};
        }
        return StatusCode::SUCCESS;
    }
    StatusCode FitterTool::fit(const EventContext& /*ctx*/,
                               const ActsTrk::TrackContainer::ConstTrackProxy& /*track*/,          
                               ActsTrk::MutableTrackContainer& /*trackContainer*/,
	                           const Acts::PerigeeSurface& /*pSurface*/) const {
        ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The method is not implemented");
        return StatusCode::FAILURE;
    }
    std::unique_ptr<ActsTrk::MutableTrackContainer> 
        FitterTool::fit(const ActsTrk::Seed & /*seed*/,
                        const Acts::BoundTrackParameters&/* initialParams*/,
                        const Acts::GeometryContext& /*tgContext*/,
                        const Acts::MagneticFieldContext& /*mfContext*/,
                        const Acts::CalibrationContext& /*calContext*/,
                        const Acts::Surface& /*targetSurface*/) const {
        THROW_EXCEPTION("Method is not implemented");
        return nullptr;
    }
    std::unique_ptr<ActsTrk::MutableTrackContainer> 
        FitterTool::fit(const std::vector<const xAOD::UncalibratedMeasurement* > & measList,
                        const Acts::BoundTrackParameters& initialParams,
                        const Acts::GeometryContext& tgContext,
                        const Acts::MagneticFieldContext& mfContext,
                        const Acts::CalibrationContext& calContext,
                        const Acts::Surface* targetSurface) const {
        std::vector<Acts::SourceLink> sourceLinks{};
        std::ranges::transform(measList, std::back_inserter(sourceLinks),
                               [](const xAOD::UncalibratedMeasurement* m ){
                                    return ActsTrk::detail::xAODUncalibMeasCalibrator::pack(m);
                               });
        return fit (sourceLinks, initialParams, tgContext, mfContext, calContext, targetSurface);

    }
    std::unique_ptr<ActsTrk::MutableTrackContainer> 
        FitterTool::fit(const std::vector<Acts::SourceLink>& sourceLinks,
                        const Acts::BoundTrackParameters& initialParams,
                        const Acts::GeometryContext& tgContext,
                        const Acts::MagneticFieldContext& mfContext,
                        const Acts::CalibrationContext& calContext,
                        const Acts::Surface* /*targetSurface*/) const {

        Acts::PropagatorPlainOptions propagationOption{tgContext, mfContext};
        propagationOption.maxSteps = m_doMaxSteps;
        propagationOption.maxTargetSkipping = m_maxSurfaces;
    
        Gx2FitterOptions_t options{tgContext, mfContext, calContext, 
                                    m_extensions, std::move(propagationOption)}; 
                                    
        options.doMultipleScattering = m_doScattering;
        options.doEnergyLoss = m_doEnergyLoss;
        options.nUpdateMax = m_maxIter;
        auto tracks = std::make_unique<ActsTrk::MutableTrackContainer>(ActsTrk::MutableTrackBackend{},
                                                                       ActsTrk::MutableTrackStateBackend{});

        bool ok{false};
        std::visit([&](const auto& fitter) {
            using Fitter_t = std::decay_t<decltype(fitter)>;
            if constexpr(std::is_same_v<Fitter_t, std::monostate>) {
                ATH_MSG_WARNING("The fitter is uninitalized. That should not happen");
            } else {
                ok = fitter.fit(sourceLinks.begin(), sourceLinks.end(), 
                                initialParams, options, *tracks).ok();
            }
        }, m_fitter);
        if (!ok) {
            ATH_MSG_VERBOSE("Fit failed");
            return nullptr;
        }
        ActsTrk::TrackContainerUtils::addFitterTypeProperty(*tracks);
        for (auto trkProxy : *tracks) {
            ActsTrk::TrackContainerUtils::setFitterType(trkProxy, xAOD::TrackFitter::GlobalChi2Fitter);
        }
        return tracks;
    }
}