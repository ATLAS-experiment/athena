/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointCalibrator.h"



#include "MdtCalibInterfaces/MdtCalibInput.h"
#include "MdtCalibInterfaces/MdtCalibOutput.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"

namespace MuonR4{
     using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
     using CalibSpacePointPtr = ISpacePointCalibrator::CalibSpacePointPtr;


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
                                                       const SpacePoint* spacePoint,
                                                       const Amg::Vector3D& posInChamb,
                                                       const Amg::Vector3D& dirInChamb,
                                                       const double timeOfArrival) const {
        
        SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
        const Amg::Vector3D& spPos{spacePoint->positionInChamber()};
        const Amg::Transform3D& locToGlob{spacePoint->chamber()->localToGlobalTrans(*gctx)};
        Amg::Vector3D chDir{spacePoint->directionInChamber()};

        // Adjust the space point position according to the external seed. But only if the space point
        // is a 1D strip
        Amg::Vector3D calibSpPos = spacePoint->dimension() == 2 ? spPos
                                 : spPos + Amg::intersect<3>(posInChamb, dirInChamb, spPos, chDir).value_or(0) * chDir;               

        CalibSpacePointPtr calibSP{};
        switch (spacePoint->type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                auto* dc = static_cast<const xAOD::MdtDriftCircle*>(spacePoint->primaryMeasurement());
                MdtCalibInput calibInput{*dc, *gctx};
                calibInput.setTrackDirection(locToGlob.linear() * dirInChamb);
                calibInput.setTimeOfFlight(timeOfArrival);
                
                Amg::Vector3D closestApproach{locToGlob*(posInChamb + Amg::intersect<3>(spPos, chDir, posInChamb, dirInChamb).value_or(0) * dirInChamb)};
                calibInput.setClosestApproach(std::move(closestApproach));
                
                /** In valid drift radius has been created */
                MdtCalibOutput calibOutput = m_mdtCalibrationTool->calibrate(ctx, calibInput);
                if (calibOutput.status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                    ATH_MSG_WARNING("Failed to create a valid hit from "<<m_idHelperSvc->toString(dc->identify())
                                    <<std::endl<<calibInput<<std::endl<<calibOutput);
                    break;                    
                }
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir));
                calibSP->setDriftRadius(calibOutput.driftRadius());
                AmgSymMatrix(2) jac{AmgSymMatrix(2)::Identity()};
                jac.col(0) = spacePoint->normalInChamber().block<2,1>(0,0).unit();
                jac.col(1) = spacePoint->directionInChamber().block<2,1>(0,0).unit();

                AmgSymMatrix(2) diagCov{AmgSymMatrix(2)::Identity()};
                diagCov(Amg::x, Amg::x) = std::pow(calibOutput.driftRadiusUncert(),2);
                diagCov(Amg::y, Amg::y) = std::pow(0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash()),2);

                AmgSymMatrix(2) cov{jac.inverse()*diagCov*jac};
                calibSP->setCovariance(std::move(cov));
                break;
           }
           case xAOD::UncalibMeasType::RpcStripType: {
                auto* strip = static_cast<const xAOD::RpcMeasurement*>(spacePoint->primaryMeasurement());
                /// Transform the space point into the local frame to calculate the propagation time towards the readout
                const Amg::Transform3D toGasGap{strip->readoutElement()->globalToLocalTrans(*gctx, strip->layerHash()) *
                                                locToGlob};
                const Amg::Vector3D lPos = toGasGap * calibSpPos;
                using EdgeSide = MuonGMR4::RpcReadoutElement::EdgeSide;
                const double signalPropDist = strip->readoutElement()->distanceToEdge(strip->layerHash(), 
                                                                                      lPos.block<2,1>(0,0),
                                                                                      EdgeSide::readOut);
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir));
                AmgSymMatrix(2) cov = spacePoint->covariance();
                /// Check
                const double timeCov = m_rpcTimeResolution*m_rpcTimeResolution 
                                     + ( spacePoint->measuresEta() ?  cov(Amg::x, Amg::x) 
                                                           : cov(Amg::y,Amg::y)) / (m_rpcSignalVelocity * m_rpcSignalVelocity) ;
                calibSP->setCovariance(std::move(cov));
                calibSP->setTimeMeasurement(strip->time() - signalPropDist / m_rpcSignalVelocity, timeCov);
                /// TODO: Use also the time of the secondary measurement...
                ATH_MSG_VERBOSE("Create rpc space point "<<m_idHelperSvc->toString(strip->identify())<<", "
                             <<" at "<<Amg::toString(calibSP->positionInChamber())<<", uncalib time: "
                             <<strip->time()<<", calib time: "<<calibSP->time());
                break;
           }
           case xAOD::UncalibMeasType::TgcStripType: {
                /// Reminder to myself, we should modify the covariance if the space point is 1D? Probably... dunno
                calibSP = std::make_unique<CalibratedSpacePoint>(spacePoint, std::move(calibSpPos), std::move(chDir));
                AmgSymMatrix(2) cov = spacePoint->covariance();
                calibSP->setCovariance(std::move(cov));
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
                                                       const double timeOfArrival) const {
        CalibSpacePointVec calibSpacePoints{};
        calibSpacePoints.reserve(spacePoints.size());
        for(const SpacePoint* spacePoint : spacePoints) {
            /// Calculate the passage of the muon reach the hit
            std::optional<double> travelledDist = std::nullopt;
            /// Closest approach of the segment to the tube
            if (spacePoint->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                travelledDist = Amg::intersect<3>(spacePoint->positionInChamber(), spacePoint->directionInChamber(), 
                                                  posInChamb, dirInChamb);
            } else {
                /// Intersect the straight line segment with a plane parallel to the xy-plane but shifted
                /// by the space point z
                const Amg::Vector3D normal = spacePoint->normalInChamber().cross(spacePoint->directionInChamber());
                travelledDist = Amg::intersect<3>(posInChamb, dirInChamb, normal, spacePoint->positionInChamber().z());
                ATH_MSG_VERBOSE("Space point "<<m_idHelperSvc->toString(spacePoint->identify())
                              <<", position "<<Amg::toString(spacePoint->positionInChamber())
                              <<", intersection: "<<Amg::toString(posInChamb + travelledDist.value_or(0)*dirInChamb));
            }
            const double hitT0 = timeOfArrival + travelledDist.value_or(0.) * m_muonPropSpeed;
            CalibSpacePointPtr hit = calibrate(ctx, spacePoint, posInChamb, dirInChamb, hitT0);
            if (hit) calibSpacePoints.push_back(std::move(hit));
        }
        return calibSpacePoints;
    }
}
