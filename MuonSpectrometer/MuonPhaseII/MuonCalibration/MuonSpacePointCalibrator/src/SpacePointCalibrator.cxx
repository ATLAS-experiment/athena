/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointCalibrator.h"

#include "MdtCalibInterfaces/MdtCalibInput.h"
#include "MdtCalibInterfaces/MdtCalibOutput.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonSpacePoint/UtilFunctions.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
namespace {
    constexpr double c_inv = 1./ Gaudi::Units::c_light;
}

namespace MuonR4{
     using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
     using CalibSpacePointPtr = ISpacePointCalibrator::CalibSpacePointPtr;
     using State = CalibratedSpacePoint::State;
     using namespace SegmentFit;


    SpacePointCalibrator::SpacePointCalibrator(const std::string& type, const std::string &name, const IInterface* parent) :
            base_class(type, name, parent) {}

    StatusCode SpacePointCalibrator::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_mdtCalibrationTool.retrieve(EnableTool{m_idHelperSvc->hasMDT()}));
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
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
        }
        return calibSP;
    }
            
    CalibSpacePointVec SpacePointCalibrator::calibrate(const EventContext& ctx,
                                                       CalibSpacePointVec&& spacePoints,
                                                       const Amg::Vector3D& segPos,
                                                       const Amg::Vector3D& segDir,
                                                       const double timeDelay) const {
        for (CalibSpacePointPtr& sp : spacePoints){
            sp = calibrate(ctx, *sp, segPos, segDir, timeDelay);
        }
        return spacePoints;
    }
 
    CalibSpacePointPtr SpacePointCalibrator::calibrate(const EventContext& ctx,
                                                       const SpacePoint* spacePoint,
                                                       const Amg::Vector3D& posInChamb,
                                                       const Amg::Vector3D& dirInChamb,
                                                       const double timeOffset) const {
        
        SG::ReadHandle gctx{m_geoCtxKey, ctx};
        const Amg::Vector3D& spPos{spacePoint->positionInChamber()};
        const Amg::Transform3D& locToGlob{spacePoint->msSector()->localToGlobalTrans(*gctx)};
        Amg::Vector3D chDir{spacePoint->directionInChamber()};

        // Adjust the space point position according to the external seed. But only if the space point
        // is a 1D strip
        Amg::Vector3D calibSpPos = spacePoint->dimension() == 2 ? spPos
                                 : spPos + Amg::intersect<3>(posInChamb, dirInChamb, spPos, chDir).value_or(0) * chDir;               

        CalibSpacePointPtr calibSP{};
        switch (spacePoint->type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                const Amg::Vector3D locClosestApproach = posInChamb 
                                                       + Amg::intersect<3>(spPos, chDir,
                                                                           posInChamb, dirInChamb).value_or(0) * dirInChamb;

                Amg::Vector3D closestApproach{locToGlob* locClosestApproach};
                const double timeOfArrival = closestApproach.mag() * c_inv  + timeOffset;
                AmgSymMatrix(2) jac{AmgSymMatrix(2)::Identity()};
                jac.col(0) = spacePoint->normalInChamber().block<2,1>(0,0).unit();
                jac.col(1) = spacePoint->directionInChamber().block<2,1>(0,0).unit();

                if (spacePoint->dimension() == 1) {
                    auto* dc = static_cast<const xAOD::MdtDriftCircle*>(spacePoint->primaryMeasurement());
                    MdtCalibInput calibInput{*dc, *gctx};
                    calibInput.setTrackDirection(locToGlob.linear() * dirInChamb,
                                                 dirInChamb.phi() || posInChamb[toInt(AxisDefs::phi)] );
                    calibInput.setTimeOfFlight(timeOfArrival);
                    calibInput.setClosestApproach(std::move(closestApproach));
                    ATH_MSG_VERBOSE("Parse hit calibration "<<m_idHelperSvc->toString(dc->identify())<<", "<<calibInput);
                    MdtCalibOutput calibOutput = m_mdtCalibrationTool->calibrate(ctx, calibInput);
                    ATH_MSG_VERBOSE("Returned calibration object "<<calibOutput);
                    State fitState{State::Valid};
                    AmgSymMatrix(2) diagCov{AmgSymMatrix(2)::Identity()};
                    diagCov(toInt(AxisDefs::eta), toInt(AxisDefs::eta)) = std::pow(0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash()),2);
                    /** In valid drift radius has been created */
                    if (calibOutput.status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                        <<std::endl<<calibInput<<std::endl<<calibOutput);
                        fitState =  State::FailedCalib;
                        diagCov(toInt(AxisDefs::phi), toInt(AxisDefs::phi)) = std::pow(dc->readoutElement()->innerTubeRadius(), 2);
                    } else {
                        diagCov(toInt(AxisDefs::phi), toInt(AxisDefs::phi)) = std::pow(calibOutput.driftRadiusUncert(), 2);
                    }
                    calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir), fitState);
                    calibSP->setCovariance<2>(jac.inverse()*diagCov*jac);
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

                    AmgSymMatrix(2) diagCov{AmgSymMatrix(2)::Identity()};
                    State fitState{State::Valid};
                    if (calibOutput.primaryStatus() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                        ATH_MSG_DEBUG("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                     <<std::endl<<calibOutput);
                        diagCov(toInt(AxisDefs::phi), toInt(AxisDefs::phi)) = std::pow(dc->readoutElement()->innerTubeRadius(), 2);
                        diagCov(toInt(AxisDefs::eta), toInt(AxisDefs::eta)) = std::pow(0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash()), 2);
                        fitState = State::FailedCalib;
                    } else {
                        diagCov(toInt(AxisDefs::phi), toInt(AxisDefs::phi)) = std::pow(calibOutput.uncertPrimaryR(), 2);
                        diagCov(toInt(AxisDefs::eta), toInt(AxisDefs::eta)) = std::pow(calibOutput.sigmaZ(), 2);
                    }
                    calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir), fitState);
                    calibSP->setCovariance<2>(jac.inverse()*diagCov*jac);
                    calibSP->setDriftRadius(calibOutput.primaryDriftR());
                }
                break;
           }
           case xAOD::UncalibMeasType::RpcStripType: {
                auto* strip = static_cast<const xAOD::RpcMeasurement*>(spacePoint->primaryMeasurement());

                /// Transform the space point into the local frame to calculate the propagation time towards the readout
                const Amg::Transform3D toGasGap{strip->readoutElement()->globalToLocalTrans(*gctx, strip->layerHash()) * locToGlob};
                const Amg::Vector3D lPos = toGasGap * calibSpPos;
                using EdgeSide = MuonGMR4::RpcReadoutElement::EdgeSide;
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir));
                AmgSymMatrix(3) cov{AmgSymMatrix(3)::Identity()};
                cov.block<2,2>(0, 0) = spacePoint->covariance();

                cov(2, 2) = m_rpcTimeResolution * m_rpcTimeResolution;

                const double time1 = strip->time() 
                                   - strip->readoutElement()->distanceToEdge(strip->layerHash(),
                                                                             lPos.block<2,1>(0,0),
                                                                             EdgeSide::readOut)  /m_rpcSignalVelocity;

                if (spacePoint->dimension() == 2) {                   
                    auto* strip2 = static_cast<const xAOD::RpcMeasurement*>(spacePoint->secondaryMeasurement());

                    const double time2 = strip2->time() -
                                         strip2->readoutElement()->distanceToEdge(strip2->layerHash(),
                                                                                  Eigen::Rotation2D{M_PI_2}*lPos.block<2,1>(0,0),
                                                                                  EdgeSide::readOut)/m_rpcSignalVelocity;
                    /// Average the time
                    calibSP->setTimeMeasurement(0.5*(time1 + time2));
                    /// Add the difference to the covariance though
                    cov(2,2) += std::pow(0.5*(time1 - time2), 2);
                    cov(2,1) = cov(1,2) = - strip->readoutElement()->stripPhiPitch() /m_rpcSignalVelocity / std::sqrt(12.)* std::sqrt(cov(2, 2));
                    cov(2,0) = cov(0,2) = - strip->readoutElement()->stripEtaPitch() /m_rpcSignalVelocity / std::sqrt(12.)* std::sqrt(cov(2, 2));

                } else {
                    calibSP->setTimeMeasurement(time1);
                    if (strip->measuresPhi()) {
                        cov(2,1) = cov(1,2) = - 0.5 *strip->readoutElement()->stripPhiLength() /m_rpcSignalVelocity * std::sqrt(cov(2, 2));
                    } else {
                        cov(2,0) = cov(0,2) = - 0.5 *strip->readoutElement()->stripEtaLength() /m_rpcSignalVelocity * std::sqrt(cov(2, 2));
                    }
                }
                calibSP->setCovariance<3>(std::move(cov));
                /// TODO: Use also the time of the secondary measurement...
                ATH_MSG_VERBOSE("Create rpc space point "<<m_idHelperSvc->toString(strip->identify())<<", dimension "<<spacePoint->dimension()
                                << ", at "<<Amg::toString(calibSP->positionInChamber())<<", uncalib time: "
                             <<strip->time()<<", calib time: "<<calibSP->time()<<" cov " <<toString(calibSP->covariance()));
                break;
           }
           case xAOD::UncalibMeasType::TgcStripType: {
                /// Reminder to myself, we should modify the covariance if the space point is 1D? Probably... dunno
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir));
                calibSP->setCovariance<2>(spacePoint->covariance());
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
            if (hit) calibSpacePoints.push_back(std::move(hit));
        }
        return calibSpacePoints;
    }
    double SpacePointCalibrator::driftVelocity(const EventContext& ctx,
                                               const CalibratedSpacePoint& spacePoint) const {
        if(spacePoint.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const MuonCalib::MdtFullCalibData* calibConsts = m_mdtCalibrationTool->getCalibConstants(ctx, spacePoint.spacePoint()->identify());
            const std::optional<double> driftTime = calibConsts->rtRelation->tr()->driftTime(spacePoint.driftRadius());
            return calibConsts->rtRelation->rt()->driftVelocity(driftTime.value_or(0.));
        }
        return 0.;
    }
    double SpacePointCalibrator::driftAcceleration(const EventContext& ctx,
                                                   const CalibratedSpacePoint& spacePoint) const  {
        if(spacePoint.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const MuonCalib::MdtFullCalibData* calibConsts = m_mdtCalibrationTool->getCalibConstants(ctx, spacePoint.spacePoint()->identify());
            const std::optional<double> driftTime = calibConsts->rtRelation->tr()->driftTime(spacePoint.driftRadius());
            return calibConsts->rtRelation->rt()->driftAcceleration(driftTime.value_or(0.));
        }
        return 0.;
    }
}
