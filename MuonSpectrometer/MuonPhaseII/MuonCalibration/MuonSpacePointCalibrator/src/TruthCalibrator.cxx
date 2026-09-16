/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthCalibrator.h"

#include "Acts/Utilities/CalibrationContext.hpp"


#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonSpacePoint/SpacePoint.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "TruthUtils/AtlasPID.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Definitions/Units.hpp"

using namespace Acts::UnitLiterals;
namespace{
    /** @brief Helper utility to craete the bound track parameters from the track state proxy */
    inline Acts::BoundTrackParameters makeBoundPars(const ActsTrk::MutableTrackContainer::TrackStateProxy& state) {
        return Acts::BoundTrackParameters{state.referenceSurface().getSharedPtr(), 
                                          state.parameters(), state.covariance(), 
                                          Acts::ParticleHypothesis::muon()};
    }

    const xAOD::MuonSimHit* getMatchingSimHit(const MuonR4::SpacePoint& sp) {
        if (const xAOD::MuonSimHit* simHit = MuonR4::getTruthMatchedHit(*sp.primaryMeasurement()); 
            simHit != nullptr) {
            return simHit;
        }
        return sp.secondaryMeasurement() ? MuonR4::getTruthMatchedHit(*sp.secondaryMeasurement())
                                         : nullptr;
    }

    Amg::Vector3D closestApproach(const Acts::GeometryContext& tgContext,
                                  const Amg::Vector3D& pos, 
                                  const Amg::Vector3D& dir,
                                  const MuonR4::SpacePoint& spacePoint) {
        
        const xAOD::MuonSimHit* simHit = getMatchingSimHit(spacePoint);
        /// If the space point has an associated sim hit and the sim hit
        /// is a muon set the closest approach to the sim hit posiiton
        if (simHit != nullptr && isMuon(simHit)) {
            const Acts::Surface& measSurf{xAOD::muonSurface(spacePoint.primaryMeasurement())};
            const Acts::Surface& secSurf{spacePoint.msSector()->surface()};
            return secSurf.localToGlobalTransform(tgContext) *
                   measSurf.localToGlobalTransform(tgContext).inverse() *
                   xAOD::toEigen(simHit->localPosition());
        }
        /// Otherwise set the measurement to the crossing of the line with the plane
        if (!spacePoint.isStraw()) {
            using namespace MuonR4::SegmentFit;
            return SeedingAux::extrapolateToPlane(pos, dir, spacePoint);
        }
        using namespace Acts::detail::LineHelper;
        /// Or to the point of closest approach along the wire
        return lineIntersect(pos, dir, spacePoint.localPosition(), 
                             spacePoint.sensorDirection()).position();
    }
}

namespace MuonR4 {
    using CalibSpacePointPtr = ISpacePointCalibrator::CalibSpacePointPtr;
    using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;

    StatusCode TruthCalibrator::initialize() {
        if (m_prdContainers.value().empty()) {
            ATH_MSG_ERROR("No prd containers configured");
            return StatusCode::SUCCESS;
        }
        ATH_MSG_DEBUG("Scheudle truth depenency on "<<m_prdContainers<<" using "<<m_simLinkDecor);
        for (const std::string& prdCont : m_prdContainers){
            m_truthLinks.emplace_back(std::format("{:}.{:}", prdCont, m_simLinkDecor.value()));
        }
        ATH_CHECK(m_truthLinks.initialize());
        ATH_CHECK(m_ctxProvider.initialize());
        return StatusCode::SUCCESS;
    }
    CalibSpacePointPtr TruthCalibrator::calibrate(const EventContext& ctx,
                                                  const SpacePoint* spacePoint,
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double /*timeDelay*/) const {
        
        CalibSpacePointPtr calibSp{std::make_unique<CalibratedSpacePoint>(spacePoint,
            closestApproach(m_ctxProvider.getGeometryContext(ctx),
                            seedPosInChamb, seedDirInChamb, *spacePoint))};

        const xAOD::MuonSimHit* simHit = getMatchingSimHit(*spacePoint);
        if (!simHit) {
            calibSp->setFitState(CalibratedSpacePoint::State::Outlier);
        }
        /// Update the drift radius accordingly
        if (spacePoint->isStraw()) {
            if (!simHit) {
                using namespace Acts::detail::LineHelper;
                calibSp->setDriftRadius(signedDistance(seedPosInChamb, seedDirInChamb, 
                                                       spacePoint->localPosition(), 
                                                       spacePoint->sensorDirection()));
            } else {
                using namespace SegmentFit;
                calibSp->setDriftRadius(spacePoint->localPosition().perp() *
                                        SeedingAux::strawSign(seedPosInChamb, seedDirInChamb, *calibSp));
            }
        }
        return calibSp;
    }
           
    CalibSpacePointPtr TruthCalibrator::calibrate(const EventContext& ctx,
                                                  const CalibratedSpacePoint& spacePoint,
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double timeDelay) const {
        if (!spacePoint.spacePoint()) {
            CalibSpacePointPtr copy{std::make_unique<CalibratedSpacePoint>(spacePoint)};
            copy->setFitState(CalibratedSpacePoint::State::Outlier);
            return copy;
        }
        return calibrate(ctx, spacePoint.spacePoint(), seedPosInChamb, seedDirInChamb, timeDelay);                                    
    }
    CalibSpacePointVec TruthCalibrator::calibrate(const EventContext& ctx,
                                                  const std::vector<const SpacePoint*>& spacePoints,
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double timeDelay) const {
        CalibSpacePointVec result{};
        std::ranges::transform(spacePoints, std::back_inserter(result), [&](const SpacePoint* sp){
            return calibrate(ctx, sp, seedPosInChamb, seedDirInChamb, timeDelay);
        });
        return result;
    }
    
    CalibSpacePointVec TruthCalibrator::calibrate(const Acts::CalibrationContext& cctx,                                            
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double timeDelay,
                                                  const CalibSpacePointVec& spacePoints) const {
        CalibSpacePointVec result{};
        /// we need to recalibrate because the fake hits fill be set to the track position
        std::ranges::transform(spacePoints,std::back_inserter(result), [&](const CalibSpacePointPtr& sp){
            return calibrate(*cctx.get<const EventContext*>(), *sp,
                             seedPosInChamb, seedDirInChamb, timeDelay);
        });
        return result;
    }
    double TruthCalibrator::driftVelocity(const Acts::CalibrationContext& /*cctx*/,
                                          const CalibratedSpacePoint& /*spacePoint*/) const{
        ATH_MSG_WARNING(__func__<<"() - Implement me ");
        return 0.;
    }
          
    void TruthCalibrator::calibrateSourceLink(const Acts::GeometryContext& tgContext,
                                              const Acts::CalibrationContext& /*cctx*/,
                                              const Acts::SourceLink& link,
                                              ActsTrk::MutableTrackStateBackend::TrackStateProxy state) const {
        const auto* uncalib = dynamic_cast<const xAOD::MuonMeasurement*>(ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(link));
        assert(uncalib != nullptr);

        const xAOD::MuonSimHit* simHit{getTruthMatchedHit(*uncalib)};
        if (!simHit && uncalib->numDimensions() == 0) {
            const auto* strip = dynamic_cast<const xAOD::CombinedMuonStrip*>(uncalib);
            simHit = getTruthMatchedHit(*strip->primaryStrip());
            if (!simHit) {
                simHit = getTruthMatchedHit(*strip->secondaryStrip());
            }
        }
        const auto boundPars{makeBoundPars(state)};
        const Acts::Surface& surface{boundPars.referenceSurface()};
        assert(surface.geometryId() == xAOD::muonSurface(uncalib).geometryId());
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Calibrate muon measurement "
            <<uncalib->readoutElement()->idHelperSvc()->toString(uncalib->identify())
            <<", @"<<surface.geometryId()
            <<", is truth "<<(simHit != nullptr)
            <<", parameters:\n"<<boundPars
            <<",\n direction: "<<Amg::toString(boundPars.direction())
            <<", momentum: "<<boundPars.absoluteMomentum());
        using ProjectorType = ActsTrk::detail::xAODUncalibMeasCalibrator::ProjectorType;
        if (simHit !=nullptr) {
            const Amg::Vector3D locPos = xAOD::toEigen(simHit->localPosition());
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Sim hit position "<<Amg::toString(locPos)
                            <<", angle: "
                            <<Amg::angle(surface.localToGlobalTransform(tgContext).linear()*xAOD::toEigen(simHit->localDirection()),
                                         boundPars.direction()) / 1._degree);
            if (uncalib->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
               if (uncalib->numDimensions() == 1) {
                setState<1>(uncalib->measuresPhi() ? 
                            ProjectorType::e1DimRotNoTime :
                            ProjectorType::e1DimNoTime, 
                            locPos.block<1,1>(uncalib->measuresPhi(),0),
                            uncalib->localCovariance<1>(), link, state);
               } else {
                setState<2>(ProjectorType::e2DimNoTime, locPos.block<2,1>(0,0),
                            uncalib->localCovariance<2>(), link, state);
               }
            } else {
                using namespace Acts::detail::LineHelper;
               const double r = signedDistance(locPos, xAOD::toEigen(simHit->localDirection()),
                                               Amg::Vector3D::Zero(),Amg::Vector3D::UnitZ());
               const double z = simHit->localPosition().z();
               if (uncalib->numDimensions() == 1) {
                setState<1>(ProjectorType::e1DimNoTime, Acts::Vector<1>{r},
                             uncalib->localCovariance<1>(), link, state);
               } else {
                setState<2>(ProjectorType::e2DimNoTime, Acts::Vector2{r, z},
                             uncalib->localCovariance<2>(), link, state);
               }
            }
        } else {
            if (uncalib->numDimensions() == 1) {
                setState<1>(ProjectorType::e1DimNoTime, state.parameters().block<1,1>(0,0),
                uncalib->localCovariance<1>(), link, state);
            } else {
                setState<2>(ProjectorType::e2DimNoTime, state.parameters().block<2,1>(0,0),
                            uncalib->localCovariance<2>(), link, state);
            }
        }
    }

    void TruthCalibrator::updateSigns(const Amg::Vector3D& /*trackPos*/,
                                      const Amg::Vector3D& /*trackDir*/,
                                      CalibSpacePointVec& /*hitsToCalib*/) const {
        /// Truth signs are correct already nothing to do
    }
    void TruthCalibrator::stampSignsOnMeasurements(const xAOD::MuonSegment& /*segment*/) const {
        /** Truth signs are correct already nothing to do */
    }
           
    double TruthCalibrator::driftRadius(const Acts::CalibrationContext& /*cctx*/,
                                        const CalibratedSpacePoint& /*spacePoint*/, 
                                        const double /*timeDelay*/) const  {
        ATH_MSG_WARNING(__func__<<"() - Implement me ");
        return 0.; 
    }
           
    double TruthCalibrator::driftVelocity(const Acts::CalibrationContext& /*cctx*/,
                                          const CalibratedSpacePoint& /*spacePoint*/, 
                                          const double /*timeDelay*/) const {
        ATH_MSG_WARNING(__func__<<"() - Implement me ");
        return 0.;
    }
    double TruthCalibrator::driftAcceleration(const Acts::CalibrationContext& /*cctx*/,
                                             const CalibratedSpacePoint& /*spacePoint*/, 
                                             const double /*timeDelay*/) const {
        ATH_MSG_WARNING(__func__<<"() - Implement me ");
        return 0.;
    }
           
    double TruthCalibrator::driftAcceleration(const Acts::CalibrationContext& /*cctx*/,
                                              const CalibratedSpacePoint& /*spacePoint*/) const {
        ATH_MSG_WARNING(__func__<<"() - Implement me ");
        return 0.;
    }
}