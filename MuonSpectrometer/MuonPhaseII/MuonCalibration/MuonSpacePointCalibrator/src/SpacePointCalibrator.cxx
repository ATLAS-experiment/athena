/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointCalibrator.h"

#include "MdtCalibInterfaces/MdtCalibInput.h"
#include "MdtCalibInterfaces/MdtCalibOutput.h"
#include "MdtCalibData/MdtFullCalibData.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"
#include "xAODMuonPrepData/sTgcStripCluster.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonTrackEvent/TrackingHelpers.h"

#include "MuonPrepRawData/NswClusteringUtils.h"

#include "ActsEvent/MultiTrajectory.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"


#include "Acts/Utilities/MathHelpers.hpp"
#include "Acts/Utilities/Enumerate.hpp"


namespace {
    constexpr double c_inv = 1./ Gaudi::Units::c_light;
    static const SG::Decorator<int> dec_trackSign{"segmentFitDriftSign"};
}

namespace MuonR4{
     using namespace Acts::UnitLiterals;
     using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
     using CalibSpacePointPtr = ISpacePointCalibrator::CalibSpacePointPtr;
     using State = CalibratedSpacePoint::State;
     using namespace SegmentFit;

    StatusCode SpacePointCalibrator::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_mdtCalibrationTool.retrieve(EnableTool{m_idHelperSvc->hasMDT()}));
        ATH_CHECK(m_nswCalibTool.retrieve(EnableTool{m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC()}));
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }

    void SpacePointCalibrator::updateSigns(const Amg::Vector3D& trackPos,
                                           const Amg::Vector3D& trackDir,
                                           CalibSpacePointVec& hitsToCalib) const {
        std::vector<int> signs = SeedingAux::strawSigns(trackPos, trackDir,
                                                        hitsToCalib);
        for (const auto [spIdx, sp]: Acts::enumerate(hitsToCalib)) {
            sp->setDriftRadius(sp->driftRadius() * signs[spIdx]);
        }
    }
    CalibSpacePointPtr SpacePointCalibrator::calibrate(const EventContext& ctx,
                                                       const CalibratedSpacePoint& spacePoint,
                                                       const Amg::Vector3D& segPos,
                                                       const Amg::Vector3D& segDir,
                                                       const double timeDelay) const {
        CalibSpacePointPtr calibSP{};
        if (spacePoint.type() != xAOD::UncalibMeasType::Other){
            calibSP = calibrate(ctx, spacePoint.spacePoint(), segPos, segDir, timeDelay);
        } else {
            calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint);
        }
        if (spacePoint.fitState() == State::Outlier) {
            calibSP->setFitState(State::Outlier);
        } else if (spacePoint.fitState() == State::Duplicate) {
            calibSP->setFitState(State::Duplicate);
        }
        return calibSP;
    }
            
    CalibSpacePointVec SpacePointCalibrator::calibrate(const Acts::CalibrationContext& cctx,
                                                       const Amg::Vector3D& segPos,
                                                       const Amg::Vector3D& segDir,
                                                       const double timeDelay,
                                                       const CalibSpacePointVec& spacePoints) const {
        CalibSpacePointVec newCalib{};
        const EventContext* ctx = cctx.get<const EventContext*>();
        newCalib.reserve(spacePoints.size());
        for (const CalibSpacePointPtr& sp : spacePoints){
            newCalib.emplace_back(calibrate(*ctx, *sp, segPos, segDir, timeDelay));
        }
        return newCalib;
    }
 
    CalibSpacePointPtr SpacePointCalibrator::calibrate(const EventContext& ctx,
                                                       const SpacePoint* spacePoint,
                                                       const Amg::Vector3D& posInChamb,
                                                       const Amg::Vector3D& dirInChamb,
                                                       const double timeOffset) const {
        
        const ActsTrk::GeometryContext* gctx{nullptr};
        if (!SG::get(gctx, m_geoCtxKey, ctx).isSuccess()) {
            return nullptr;
        }
        const Amg::Vector3D& spPos{spacePoint->localPosition()};
        const Amg::Transform3D& locToGlob{spacePoint->msSector()->localToGlobalTransform(*gctx)};
        const Amg::Vector3D& chDir{spacePoint->sensorDirection()};

        // Adjust the space point position according to the external seed. But only if the space point
        // is a 1D strip
        Amg::Vector3D calibSpPos = spacePoint->dimension() == 2 ? spPos
                                 : spPos + Amg::intersect<3>(posInChamb, dirInChamb, spPos, chDir).value_or(0) * chDir;               

        SpacePoint::Cov_t cov = spacePoint->covariance();
        CalibSpacePointPtr calibSP{};
        ATH_MSG_VERBOSE("Calibrate "<<(*spacePoint) <<" -> updated pos "<<Amg::toString(calibSpPos));
        switch (spacePoint->type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                const Amg::Vector3D locClosestApproach = posInChamb 
                                                       + Amg::intersect<3>(spPos, chDir,
                                                                           posInChamb, dirInChamb).value_or(0) * dirInChamb;

                Amg::Vector3D closestApproach{locToGlob* locClosestApproach};
                const double timeOfArrival = closestApproach.mag() * c_inv  + timeOffset;

                if (ATH_LIKELY(spacePoint->dimension() == 1)) {
                    auto* dc = static_cast<const xAOD::MdtDriftCircle*>(spacePoint->primaryMeasurement());
                    MdtCalibInput calibInput{*dc, *gctx};
                    calibInput.setTrackDirection(locToGlob.linear() * dirInChamb,
                                                 Acts::abs(dirInChamb.phi() - 90._degree) > 1.e-7 );
                    calibInput.setTimeOfFlight(timeOfArrival);
                    calibInput.setClosestApproach(std::move(closestApproach));
                    ATH_MSG_VERBOSE("Parse hit calibration "<<m_idHelperSvc->toString(dc->identify())<<", "<<calibInput);
                    MdtCalibOutput calibOutput = m_mdtCalibrationTool->calibrate(ctx, calibInput);
                    ATH_MSG_VERBOSE("Returned calibration object "<<calibOutput);
                    State fitState{State::Valid};
                    /** In valid drift radius has been created */
                    if (calibOutput.status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                        <<std::endl<<calibInput<<std::endl<<calibOutput);
                        fitState =  State::FailedCalib;
                        cov[Acts::toUnderlying(AxisDefs::etaCov)] = dc->readoutElement()->innerTubeRadius();
                    } else {
                        cov[Acts::toUnderlying(AxisDefs::etaCov)] = Acts::square(calibOutput.driftRadiusUncert());
                    }
                    calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), fitState);
                    calibSP->setCovariance(cov);
                    calibSP->setDriftRadius(calibOutput.driftRadius());
                } else {
                    auto* dc = static_cast<const xAOD::MdtTwinDriftCircle*>(spacePoint->primaryMeasurement());
                    MdtCalibInput calibInput{*dc, *gctx};
                    calibInput.setClosestApproach(closestApproach);
                    calibInput.setTimeOfFlight(timeOfArrival);

                    MdtCalibInput twinInput{dc->twinIdentify(), dc->twinAdc(),  dc->twinTdc(), dc->readoutElement(), *gctx};
                    twinInput.setClosestApproach(closestApproach);
                    twinInput.setTimeOfFlight(timeOfArrival);

                    MdtCalibTwinOutput calibOutput = m_mdtCalibrationTool->calibrateTwinTubes(ctx,
                                                                                              std::move(calibInput),
                                                                                              std::move(twinInput)); 

                    State fitState{State::Valid};
                    if (calibOutput.primaryStatus() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                     <<std::endl<<calibOutput);
                        cov[Acts::toUnderlying(AxisDefs::etaCov)] = Acts::square(dc->readoutElement()->innerTubeRadius());
                        cov[Acts::toUnderlying(AxisDefs::phiCov)] = Acts::square(0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash()));
                        fitState = State::FailedCalib;
                    } else {
                        cov[Acts::toUnderlying(AxisDefs::etaCov)] = Acts::square(calibOutput.uncertPrimaryR());
                        cov[Acts::toUnderlying(AxisDefs::phiCov)] = Acts::square(calibOutput.sigmaZ());
                    }
                    calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), fitState);
                    calibSP->setCovariance(cov);
                    calibSP->setDriftRadius(calibOutput.primaryDriftR());
                }
                break;
           }
           case xAOD::UncalibMeasType::RpcStripType: {
                auto* strip = static_cast<const xAOD::RpcMeasurement*>(spacePoint->primaryMeasurement());

                /// Transform the space point into the local frame to calculate the propagation time towards the readout
                const Amg::Transform3D toGasGap{strip->readoutElement()->globalToLocalTransform(*gctx, strip->layerHash()) * locToGlob};
                const Amg::Vector3D lPos = toGasGap * calibSpPos;
                using EdgeSide = MuonGMR4::RpcReadoutElement::EdgeSide;
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos));
        
                cov[Acts::toUnderlying(AxisDefs::timeCov)] = Acts::square(m_rpcTimeResolution);

                const double time1 = strip->time() 
                                   - strip->readoutElement()->distanceToEdge(strip->layerHash(), lPos,
                                                                             EdgeSide::readOut)  /m_rpcSignalVelocity;

                if (spacePoint->dimension() == 2) {                   
                    auto* strip2 = static_cast<const xAOD::RpcMeasurement*>(spacePoint->secondaryMeasurement());

                    const double time2 = strip2->time() -
                                         strip2->readoutElement()->distanceToEdge(strip2->layerHash(),lPos, EdgeSide::readOut)/m_rpcSignalVelocity;
                    /// Average the time
                    calibSP->setTimeMeasurement(0.5*(time1 + time2));
                    /// Add the difference to the covariance though
                    cov[Acts::toUnderlying(AxisDefs::timeCov)] += Acts::square(0.5*(time1 - time2));
                } 
                calibSP->setCovariance(cov);
                ATH_MSG_VERBOSE("Create rpc space point "<<m_idHelperSvc->toString(strip->identify())<<", dimension "<<spacePoint->dimension()
                                << ", at "<<Amg::toString(calibSP->localPosition())<<", uncalib time: "
                                <<strip->time()<<", calib time: "<<calibSP->time()<<" cov " <<calibSP->covariance());
                break;
           }
           case xAOD::UncalibMeasType::TgcStripType: {
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos));
                calibSP->setCovariance(cov);
                break;
           }
           case xAOD::UncalibMeasType::MMClusterType: {
                const xAOD::MMCluster* cluster = static_cast<const xAOD::MMCluster*>(spacePoint->primaryMeasurement());
                Amg::Vector3D globalPos{locToGlob * posInChamb};
                Amg::Vector3D globalDir{locToGlob.linear() * dirInChamb};
                
                std::pair<double, double> calibPosCov {calibrateMM(ctx, *gctx, *cluster, globalPos, globalDir)}; 
                
                ATH_MSG_DEBUG("Calibrated pos and cov" << calibPosCov.first << " " << calibPosCov.second);
                cov[Acts::toUnderlying(AxisDefs::etaCov)] = calibPosCov.second;
                Amg::Transform3D toChamberTrans{ locToGlob.inverse() * cluster->readoutElement()->localToGlobalTransform(*gctx, cluster->layerHash())};

                // since we want to take the second coordiante from the external estimate we need to transform the sp posiiton to the layer frame, replace the precission coordinate and transform back
                Amg::Vector3D calibSpPosInLayer = toChamberTrans.inverse() * calibSpPos;
                ATH_MSG_DEBUG("in layer before calibration" << Amg::toString(calibSpPosInLayer));
                calibSpPosInLayer.x() = calibPosCov.first;
                ATH_MSG_DEBUG("in layer after calibration" << Amg::toString(calibSpPosInLayer));
                calibSpPos = toChamberTrans * calibSpPosInLayer;

                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos));
                calibSP->setCovariance(cov);
                ATH_MSG_DEBUG("calibrated MM cluster "<<m_idHelperSvc->toString(cluster->identify()) << " loc x old " << cluster->localPosition<1>()[0] << " new loc x " << calibSP->localPosition()[1]  << "cov " << calibSP->covariance());
                                
                break;
           }
           case xAOD::UncalibMeasType::sTgcStripType: {
                const auto* cluster = static_cast<const xAOD::sTgcMeasurement*>(spacePoint->primaryMeasurement());

                // We do not apply any correction for pads or wire only space points
                if (cluster->channelType() != sTgcIdHelper::sTgcChannelTypes::Strip) {
                    ATH_MSG_DEBUG("Calibrating an sTGC Pad or wire " << m_idHelperSvc->toString(cluster->identify()));
                    calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos));
                    calibSP->setCovariance(cov);
                    break;
                }
                
                std::optional<double> posAlongTheStrip{std::nullopt};

                // check if the space point is a strip/wire combination and take the position along the strip from the wire measurement
                if(spacePoint->secondaryMeasurement()) {
                    const auto* secMeas = static_cast<const xAOD::sTgcMeasurement*>(spacePoint->secondaryMeasurement());
                    ATH_MSG_VERBOSE("Using secondary measurement "<< m_idHelperSvc->toString(secMeas->identify())<<" for sTGC strip cluster " << m_idHelperSvc->toString(cluster->identify()));
                    posAlongTheStrip =static_cast<double>(spacePoint->secondaryMeasurement()->localPosition<1>()[0]);
                } else {
                    ATH_MSG_VERBOSE("No secondary measurement for sTGC strip cluster " << m_idHelperSvc->toString(cluster->identify()));
                }

                Amg::Vector3D globalPos{locToGlob * posInChamb};
                Amg::Vector3D globalDir{locToGlob.linear() * dirInChamb};
                
                const auto* stripClus = static_cast<const xAOD::sTgcStripCluster*>(cluster);
                const auto [calibPos, calibCov] = calibratesTGC(ctx, *gctx, *stripClus, posAlongTheStrip, globalPos, globalDir); 
                
                ATH_MSG_DEBUG("Calibrated pos and cov" << calibPos << " " << calibCov);
                cov[Acts::toUnderlying(AxisDefs::etaCov)] = calibCov;
                Amg::Transform3D toChamberTrans{ locToGlob.inverse() * cluster->readoutElement()->localToGlobalTransform(*gctx, cluster->layerHash())};

                // since we want to take the second coordiante from the external estimate we need to transform the sp posiiton to the layer frame, replace the precission coordinate and transform back
                Amg::Vector3D calibSpPosInLayer = toChamberTrans.inverse() * calibSpPos;
                ATH_MSG_DEBUG("in layer before calibration" << Amg::toString(calibSpPosInLayer));
                calibSpPosInLayer.x() = calibPos;
                ATH_MSG_DEBUG("in layer after calibration" << Amg::toString(calibSpPosInLayer));
                calibSpPos = toChamberTrans * calibSpPosInLayer;

                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos));
                calibSP->setCovariance(cov);
                ATH_MSG_DEBUG("calibrated sTGC cluster "<<m_idHelperSvc->toString(cluster->identify()) << " loc x old " << cluster->localPosition<1>()[0] << " new loc x " << calibSP->localPosition()[1]  << "cov " << calibSP->covariance());
                break;
           }

           default:
                ATH_MSG_WARNING("Do not know how to calibrate "<<m_idHelperSvc->toString(spacePoint->identify()));        
        }
        return calibSP;
    }
    
    CalibSpacePointVec SpacePointCalibrator::calibrate(const EventContext& ctx,
                                                       const std::vector<const SpacePoint*>& spacePoints,
                                                       const Amg::Vector3D& posInChamb,
                                                       const Amg::Vector3D& dirInChamb,
                                                       const double timeOffset) const {
        CalibSpacePointVec calibSpacePoints{};
        calibSpacePoints.reserve(spacePoints.size());
        for(const SpacePoint* spacePoint : spacePoints) {
            CalibSpacePointPtr hit = calibrate(ctx, spacePoint, posInChamb, dirInChamb, timeOffset);
            if (hit) {
                calibSpacePoints.push_back(std::move(hit));
            }
        }
        return calibSpacePoints;
    }
    double SpacePointCalibrator::driftVelocity(const Acts::CalibrationContext& ctx,
                                               const CalibratedSpacePoint& spacePoint) const {
        if(spacePoint.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            
            const MuonCalib::MdtFullCalibData* calibConsts = m_mdtCalibrationTool->getCalibConstants(*ctx.get<const EventContext*>(), spacePoint.spacePoint()->identify());
            const std::optional<double> driftTime = calibConsts->rtRelation->tr()->driftTime(spacePoint.driftRadius());
            return calibConsts->rtRelation->rt()->driftVelocity(driftTime.value_or(0.));
        }
        return 0.;
    }
    double SpacePointCalibrator::driftAcceleration(const Acts::CalibrationContext& ctx,
                                                   const CalibratedSpacePoint& spacePoint) const  {
        if(spacePoint.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const MuonCalib::MdtFullCalibData* calibConsts = m_mdtCalibrationTool->getCalibConstants(*ctx.get<const EventContext*>(), spacePoint.spacePoint()->identify());
            const std::optional<double> driftTime = calibConsts->rtRelation->tr()->driftTime(spacePoint.driftRadius());
            return calibConsts->rtRelation->rt()->driftAcceleration(driftTime.value_or(0.));
        }
        return 0.;
    }

    std::pair<double, double> SpacePointCalibrator::calibrateMM(const EventContext& ctx, 
                                                                const ActsTrk::GeometryContext& gctx,
                                                                const xAOD::MMCluster& cluster, 
                                                                const Amg::Vector3D& globalPos, 
                                                                const Amg::Vector3D& globalDir) const {
        std::vector<NSWCalib::CalibratedStrip> calibClus;
        StatusCode sc =  m_nswCalibTool->calibrateClus(ctx, gctx, cluster, globalPos, calibClus);
        if(sc.isFailure()) {
            ATH_MSG_WARNING("Failed to calibrate MM cluster "<<m_idHelperSvc->toString(cluster.identify()));
            return std::make_pair(0., 0.);
        }

        Amg::Vector2D locPos{cluster.localPosition<1>()[0] * Amg::Vector2D::UnitX()};
        Amg::Vector3D locDir = Muon::NswClustering::toLocal(cluster.readoutElement()->globalToLocalTransform(gctx, cluster.layerHash()), globalDir);

        Amg::MatrixX calibCov{};
        calibCov.resize(1,1);
        calibCov(0,0) = cluster.localCovariance<1>()(0, 0);
        ATH_MSG_DEBUG("old loc pos " << locPos[0] << " old cov" << calibCov(0,0)  );

        Muon::IMMClusterBuilderTool::RIO_Author rotAuthor = m_clusterBuilderToolMM->getCalibratedClusterPosition(ctx, calibClus, locDir ,locPos, calibCov);
        if(rotAuthor == Muon::IMMClusterBuilderTool::RIO_Author::unKnownAuthor){
            THROW_EXCEPTION("Failed to calibrate MM cluster "<<m_idHelperSvc->toString(cluster.identify()));
        }
        ATH_MSG_DEBUG("new loc pos " << locPos[0] << " new cov" << calibCov(0,0)  );
        return std::make_pair(locPos[0], calibCov(0,0));
    }

    std::pair<double, double> SpacePointCalibrator::calibratesTGC(const EventContext& /*ctx*/, 
                                                                  const ActsTrk::GeometryContext& gctx, 
                                                                  const xAOD::sTgcStripCluster& cluster,
                                                                  std::optional<double> posAlongTheStrip, 
                                                                  const Amg::Vector3D& globalPos, 
                                                                  const Amg::Vector3D& /*globalDir*/) const{

        // if the second coordiante was not provided by the wire, take it from the seed track position 
        if(!posAlongTheStrip) {
            Amg::Vector3D extPosLocal =  cluster.readoutElement()->globalToLocalTransform(gctx, cluster.layerHash()) * globalPos;
            posAlongTheStrip = extPosLocal[1];
        }

        // For now just copying over the local position and covariance. Eventually this should apply corrections from B-Lines and as build geometry
        
        return std::make_pair(cluster.localPosition<1>()[0], cluster.localCovariance<1>()(0,0));
    }
    void SpacePointCalibrator::calibrateCombinedPrd(const EventContext& /*ctx*/, 
                                                    const ActsTrk::GeometryContext& /*gctx*/,
                                                    const xAOD::CombinedMuonStrip* combinedPrd,
                                                    ActsTrk::MutableTrackContainer::TrackStateProxy state) const {
        const auto sl = ActsTrk::detail::xAODUncalibMeasCalibrator::pack(combinedPrd);
        if (combinedPrd->type() == xAOD::UncalibMeasType::RpcStripType) {
            if (m_useRpcTime) {
                ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Implement me");
            }
            Amg::Vector2D cmbPos{combinedPrd->primaryStrip()->localPosition<1>()[0],
                                 combinedPrd->secondaryStrip()->localPosition<1>()[0]};
            AmgSymMatrix(2) cmbCov{AmgSymMatrix(2)::Identity()};
            cmbCov (0, 0) = combinedPrd->primaryStrip()->localCovariance<1>()(0,0);
            cmbCov (1, 1) = combinedPrd->secondaryStrip()->localCovariance<1>()(0,0);
            setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimNoTime, cmbPos, cmbCov, sl, state);

            
        } else if (combinedPrd->type() == xAOD::UncalibMeasType::TgcStripType) {
            if (m_useTgcTime) {
                ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Implement me");
            }
            const auto* wireMeas = static_cast<const xAOD::TgcStrip*>(combinedPrd->primaryStrip());
            const auto* stripMeas = static_cast<const xAOD::TgcStrip*>(combinedPrd->secondaryStrip());
            
            const auto& radialDesign = stripMeas->readoutElement()->stripLayout(stripMeas->layerHash());
            const auto& wireDesign = wireMeas->readoutElement()->wireGangLayout(wireMeas->layerHash());            
            
            const double dirDots = radialDesign.stripDir(stripMeas->channelNumber()).dot(wireDesign.stripNormal());
            /// Apply the stereo transform to the covariance
            AmgSymMatrix(2) stereoTrf{AmgSymMatrix(2)::Identity()};
            const double invDist = 1. / (1. - Acts::square(dirDots));
            stereoTrf(0, 0) = stereoTrf(1, 1) = invDist;
            stereoTrf(0, 1) = stereoTrf(1, 0) = -dirDots * invDist;

            Amg::Vector2D cmbPos{wireMeas->localPosition<1>()[0],
                                 stripMeas->localPosition<1>()[0]};
            AmgSymMatrix(2) cmbCov{AmgSymMatrix(2)::Identity()};
            cmbCov (0, 0) = wireMeas->localCovariance<1>()(0,0);
            cmbCov (1, 1) = stripMeas->localCovariance<1>()(0,0);

            setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimNoTime, cmbPos, 
                                                           stereoTrf*cmbCov*stereoTrf.transpose(), sl, state);
        } else {
            THROW_EXCEPTION("Undefined uncalibrated measurement "
                            <<m_idHelperSvc->toString(xAOD::identify(combinedPrd)));
        }
    }
    void SpacePointCalibrator::calibrateSourceLink(const Acts::GeometryContext& geoctx,
                                                   const Acts::CalibrationContext& cctx,
                                                   const Acts::SourceLink& link,
                                                   ActsTrk::MutableTrackContainer::TrackStateProxy trackState) const {
     
        /** Construct bound track parameters to fetch the global track position */
        const Acts::BoundTrackParameters trackPars{trackState.referenceSurface().getSharedPtr(), 
                                                   trackState.parameters(), trackState.covariance(), 
                                                   Acts::ParticleHypothesis::muon()};
        

        const auto* muonMeas = ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(link);
        const ActsTrk::GeometryContext* gctx = geoctx.get<const ActsTrk::GeometryContext*>();
        const EventContext* ctx = cctx.get<const EventContext*>();
        ATH_MSG_VERBOSE("Calibrate measurement "<<m_idHelperSvc->toString(xAOD::identify(muonMeas))
                     <<" @ surface "<<trackState.referenceSurface().geometryId());
        /// Only the combined muonstrip has zero dimensions
        if (muonMeas->numDimensions() == 0u) {
            calibrateCombinedPrd(*ctx, *gctx, static_cast<const xAOD::CombinedMuonStrip*>(muonMeas),
                                 trackState);
            return;

        }
        const Amg::Vector3D trackPos{trackPars.position(geoctx)};
        const Amg::Vector3D trackDir{trackPars.direction()};

        switch (muonMeas->type()){
            using enum xAOD::UncalibMeasType;
            case MdtDriftCircleType: {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(muonMeas);
                MdtCalibInput calibInput{*dc, *gctx};
                calibInput.setClosestApproach(trackPos);
                //calibInput.setTimeOfFlight(trackPars.parameters()[Acts::eBoundTime]);
                calibInput.setTrackDirection(trackDir, true);
                const double driftSign = m_MdtSignFromSegment ? 
                                         static_cast<double>(dec_trackSign(*dc)) :
                                         Acts::copySign(1.,trackPars.parameters()[Acts::eBoundLoc0]);

                /** Vast majority of the measurements are ordinary drift tubes */
                if (ATH_LIKELY(muonMeas->numDimensions() == 1)) {
                    MdtCalibOutput calibOutput = m_mdtCalibrationTool->calibrate(*ctx, calibInput);
                    ATH_MSG_VERBOSE("Returned calibration object "<<calibOutput);
                    AmgVector(1) pos{AmgVector(1)::Zero()};
                    AmgSymMatrix(1) cov{AmgSymMatrix(1)::Identity()};
                    /** Invalid drift radius has been created */
                    if (calibOutput.status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                        <<std::endl<<calibInput<<std::endl<<calibOutput);
                        cov(Acts::eBoundLoc0,Acts::eBoundLoc0) = std::pow(dc->readoutElement()->innerTubeRadius(), 2);
                    } else {
                        pos[Acts::eBoundLoc0] = driftSign*calibOutput.driftRadius();
                        cov(Acts::eBoundLoc0, Acts::eBoundLoc0) = std::pow(calibOutput.driftRadiusUncert(), 2);
                    }
                    setState<1, ActsTrk::MutableTrackStateBackend>(ProjectorType::e1DimNoTime, pos, cov, link, trackState);
                } 
                /** Twin tube case */
                else {
                    const auto* twinDC = static_cast<const xAOD::MdtTwinDriftCircle*>(muonMeas);
                    MdtCalibInput twinInput{twinDC->twinIdentify(), twinDC->twinAdc(),  twinDC->twinTdc(), twinDC->readoutElement(), *gctx};
                    twinInput.setClosestApproach(trackPos);
                    twinInput.setTimeOfFlight(trackPars.parameters()[Acts::eBoundTime]);

                    MdtCalibTwinOutput calibOutput = m_mdtCalibrationTool->calibrateTwinTubes(*ctx,
                                                                                              std::move(calibInput),
                                                                                              std::move(twinInput)); 
                    Amg::Vector2D locPos{Amg::Vector2D::Zero()};
                    AmgSymMatrix(2) locCov{AmgSymMatrix(2)::Identity()};
                    if (calibOutput.primaryStatus() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                     <<std::endl<<calibOutput);
                        locCov(Acts::eBoundLoc0, Acts::eBoundLoc0) = std::pow(dc->readoutElement()->innerTubeRadius(), 2);
                        locCov(Acts::eBoundLoc1, Acts::eBoundLoc1) = std::pow(0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash()), 2);
                    } else {
                        locCov(Acts::eBoundLoc0, Acts::eBoundLoc0) = std::pow(calibOutput.uncertPrimaryR(), 2);
                        locCov(Acts::eBoundLoc1, Acts::eBoundLoc1) = std::pow(calibOutput.sigmaZ(), 2);
                        locPos[Acts::eBoundLoc0] = driftSign*calibOutput.primaryDriftR();
                        locPos[Acts::eBoundLoc1] = calibOutput.locZ();
                    }
                    setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimNoTime, locPos, locCov, link, trackState);
                }
                break;
            } case RpcStripType: {
                const auto* rpcClust = static_cast<const xAOD::RpcMeasurement*>(muonMeas);
                /** Legacy BM / BO chambers */
                if (ATH_LIKELY(rpcClust->numDimensions() == 1)) {

                    if (!m_useRpcTime) {
                        const auto proj = rpcClust->measuresPhi() ? ProjectorType::e1DimRotNoTime
                                                                  : ProjectorType::e1DimNoTime;
                        setState<1, ActsTrk::MutableTrackStateBackend>(proj, 
                                                                       rpcClust->localPosition<1>(), 
                                                                       rpcClust->localCovariance<1>(), link, trackState);
                    } else {
                        AmgVector(2) measPars{AmgVector(2)::Zero()};
                        AmgSymMatrix(2) measCov{AmgSymMatrix(2)::Identity()};
                        measPars[0] = rpcClust->localPosition<1>()[0];
                        measCov(0,0) = rpcClust->localCovariance<1>()(0, 0);
                        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"Please fix me using the ActsInterops package");
                        measCov(1,1) = std::pow(m_rpcTimeResolution, 2);
                        const auto proj = rpcClust->measuresPhi() ? ProjectorType::e1DimRotWithTime
                                                                  : ProjectorType::e1DimWithTime;
                        setState<2, ActsTrk::MutableTrackStateBackend>(proj, 
                                                                       measPars, measCov, link, trackState);
                    }
                } 
                /** BI clusters */
                else {
                    if (!m_useRpcTime) {
                        setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimNoTime, 
                                                                       rpcClust->localPosition<2>(), 
                                                                       rpcClust->localCovariance<2>(), link, trackState);
                    } else {
                        AmgVector(3) measPars{AmgVector(3)::Zero()};
                        AmgSymMatrix(3) measCov{AmgSymMatrix(3)::Identity()};
                        measPars.block<2,1>(0,0) = xAOD::toEigen(rpcClust->localPosition<2>());
                        measCov.block<2,2>(0,0) = xAOD::toEigen(rpcClust->localCovariance<2>());
                        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"Please fix me using the ActsInterops package");
                        measCov(2,2) = std::pow(m_rpcTimeResolution, 2);
                        setState<3, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimWithTime, 
                                                                       measPars, measCov, link, trackState);
                    }
                }
                break;
            } case TgcStripType: {
                const auto* tgcClust = static_cast<const xAOD::TgcStrip*>(muonMeas);
                if (!m_useTgcTime) {
                    const auto proj = tgcClust->measuresPhi() ? ProjectorType::e1DimRotNoTime
                                                              : ProjectorType::e1DimNoTime;
                    setState<1, ActsTrk::MutableTrackStateBackend>(proj, 
                                                                   tgcClust->localPosition<1>(), 
                                                                   tgcClust->localCovariance<1>(), link, trackState);
                    } else {
                        ATH_MSG_WARNING("Tgc time calibration to be implemented...");
                    }
                break;
            } 
            case MMClusterType: {
                const auto* mmClust = static_cast<const xAOD::MMCluster*>(muonMeas);
                std::pair<double, double> calibPosCov{calibrateMM(*ctx,* gctx, *mmClust, trackPos, trackDir)};
                AmgVector(1) pos{AmgVector(1)(calibPosCov.first)};
                AmgSymMatrix(1) cov{AmgSymMatrix(1)(calibPosCov.second)};

                setState<1, ActsTrk::MutableTrackStateBackend>(ProjectorType::e1DimNoTime, 
                                                               pos, cov, link, trackState);
                break;
            } case sTgcStripType: {
                const auto* stgcClust = static_cast<const xAOD::sTgcMeasurement*>(muonMeas);

                if(stgcClust->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                        setState<1, ActsTrk::MutableTrackStateBackend>(ProjectorType::e1DimNoTime, 
                                                                       muonMeas->localPosition<1>(), 
                                                                       muonMeas->localCovariance<1>(), link, trackState);
                } else if (stgcClust->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad) {
                        setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e2DimNoTime, 
                                                                       stgcClust->localPosition<2>(), 
                                                                       stgcClust->localCovariance<2>(), link, trackState);
                } else { // strips
                    const auto stgCluster = static_cast<const xAOD::sTgcStripCluster*>(muonMeas);
                    std::pair<double, double> calibPosCov{calibratesTGC(*ctx, *gctx, *stgCluster, std::nullopt, trackPos, trackDir)};
                    if(!m_usesTgcTime) {
                        AmgVector(1) pos{calibPosCov.first};
                        AmgSymMatrix(1) cov{calibPosCov.second};
                        setState<1, ActsTrk::MutableTrackStateBackend>(ProjectorType::e1DimNoTime, 
                                                                       pos, cov, link, trackState);
                    } else {
                        ATH_MSG_WARNING("sTGC time calibration to be implemented...");
                        AmgVector(2) pos{AmgVector(2)::Zero()};
                        AmgSymMatrix(2) cov{AmgSymMatrix(2)::Zero()};
                        pos[0] = calibPosCov.first;
                        pos[1] = stgCluster->time();
                        cov(0,0) = calibPosCov.second;
                        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"Please fix me using the ActsInterops package");
                        cov(1,1) = std::pow(25 /*ns*/, 2);

                        setState<2, ActsTrk::MutableTrackStateBackend>(ProjectorType::e1DimWithTime, 
                                                                       pos, cov, link, trackState);
                    }
                }
                break;
            } default: {
                THROW_EXCEPTION("The parsed measurement is not a muon measurement. Please check.");
            }
        }
    }
    void SpacePointCalibrator::stampSignsOnMeasurements(const xAOD::MuonSegment& segment) const {
        const auto [segPos, segLine] = makeLine(localSegmentPars(segment));
        const Segment* detSeg = MuonR4::detailedSegment(segment);
        for (const auto& meas : detSeg->measurements()) {
            if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType){
                dec_trackSign(*meas->spacePoint()->primaryMeasurement()) =
                    SeedingAux::strawSign(segPos, segLine, *meas);
            }
        }
    }
}
