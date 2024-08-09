/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"

#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GeoModelHelpers/throwExcept.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"

namespace{
    constexpr double  c_inv = 1. / Gaudi::Units::c_light;
    constexpr auto printLvl = MSG::VERBOSE;
}


namespace MuonR4 {
    namespace SegmentFitHelpers{
        using namespace SegmentFit;

        double chiSqTerm(const Amg::Vector3D& posInChamber,
                         const Amg::Vector3D& dirInChamber,
                         const MuonR4::HoughHitType& measurement,
                         MsgStream& msg) {
            double chi2{0.};
            switch (measurement->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: 
                    chi2 = chiSqTermMdt(posInChamber, dirInChamber,
                                        measurement, msg);
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                case xAOD::UncalibMeasType::TgcStripType:
                    chi2 = chiSqTermStrip(posInChamber, dirInChamber,
                                          measurement, msg);
                    break;
                default:
                    if (msg.level() <= MSG::WARNING) {
                        msg<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" - Unsupported measurement: "
                            <<measurement->chamber()->idHelperSvc()->toString(measurement->identify())<<endmsg;
                    }
            }
            return chi2;
        }
        double chiSqTermMdt(const Amg::Vector3D& segPos,
                            const Amg::Vector3D& segDir,
                            const MuonR4::HoughHitType& sp,
                            MsgStream& msg) {
                        
            Amg::Vector2D residual{Amg::Vector2D::Zero()};
    
            residual[Amg::y] = Amg::lineDistance<3>(sp->positionInChamber(), sp->directionInChamber(), 
                                                    segPos, segDir) 
                             - sp->driftRadius();
            const double chi2{residual.dot(sp->covariance().inverse()* residual)};
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Measurement "<<sp->chamber()->idHelperSvc()->toString(sp->identify());
                msg<<printLvl<<", position: "<<Amg::toString(sp->positionInChamber())
                             <<", driftRadius: "<<sp->driftRadius()
                             <<", status: "<<static_cast<const xAOD::MdtDriftCircle*>(sp->primaryMeasurement())->status()<<endmsg;
                msg<<printLvl<<"segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<","<<endmsg;
                msg<<printLvl<<"residual: "<<Amg::toString(residual) <<", covariance: "<<std::endl
                             <<Amg::toString(sp->covariance())<<std::endl<<", inverse: "
                             <<std::endl<<Amg::toString(sp->covariance().inverse())<<", "<<endmsg;
                msg<<printLvl<<"chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermStrip(const Amg::Vector3D& segPos,
                              const Amg::Vector3D& segDir,
                              const MuonR4::HoughHitType& sp,
                              MsgStream& msg){
            const Amg::Vector3D normal = sp->normalInChamber().cross(sp->directionInChamber());
            std::optional<double> travelledDist = Amg::intersect<3>(segPos, segDir, normal, sp->positionInChamber().z());
            const Amg::Vector3D planeCrossing = segPos + travelledDist.value_or(0) * segDir; 

            const Amg::Vector2D residual{(planeCrossing - sp->positionInChamber()).block<2,1>(0,0)};
            const double chi2 = residual.dot(sp->covariance().inverse()* residual);
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Measurement "<<sp->chamber()->idHelperSvc()->toString(sp->identify())
                             <<", position: "<<Amg::toString(sp->positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: " <<std::endl<<Amg::toString(sp->covariance())<<", inverse: "
                             <<std::endl<<Amg::toString(sp->covariance().inverse())<<","<<endmsg;
                msg<<printLvl<<"chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }


        double chiSqTerm(const Amg::Vector3D& segPos,
                         const Amg::Vector3D& segDir,
                         std::optional<double> timeOfArrival,
                         const CalibratedSpacePoint& hit,
                         MsgStream& msg) {
            double chi2{0.};
            switch (hit.type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType:
                    chi2 = chiSqTermMdt(segPos, segDir, hit, msg); 
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                    chi2 = chiSqTermStrip(segPos, segDir, timeOfArrival, hit, msg); 
                    break;
                case xAOD::UncalibMeasType::TgcStripType:
                    chi2 = chiSqTermStrip(segPos, segDir, std::nullopt, hit, msg); 
                    break;
                case xAOD::UncalibMeasType::Other:
                    chi2 = chiSqTermBeamspot(segPos,segDir, hit, msg);
                    break;
                default:
                   if (msg.level() <= MSG::WARNING) {
                        msg<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" - Unsupported measurement: "
                            <<hit.spacePoint()->chamber()->idHelperSvc()->toString(hit.spacePoint()->identify())<<endmsg;
                    }
            }
            return chi2;
        }

        double chiSqTermMdt(const Amg::Vector3D& segPos,
                            const Amg::Vector3D& segDir,
                            const CalibratedSpacePoint& hit,
                            MsgStream& msg) {

            Amg::Vector2D residual{Amg::Vector2D::Zero()};
    
            residual[Amg::y] = Amg::lineDistance<3>(hit.positionInChamber(), hit.directionInChamber(), 
                                                    segPos, segDir) 
                             - hit.driftRadius();
            const double chi2{residual.dot(hit.covariance().inverse()* residual)};
            if (msg.level() <= printLvl) {
                const SpacePoint* sp = hit.spacePoint();
                msg<<printLvl<<"Calibrated measurement "<<sp->chamber()->idHelperSvc()->toString(sp->identify());
                msg<<printLvl<<", position: "<<Amg::toString(hit.positionInChamber())<<", driftRadius: "<<hit.driftRadius()<<endmsg;
                msg<<printLvl<<", segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<", residual: "<<Amg::toString(residual)<<std::endl <<", covariance: "<<std::endl
                             <<Amg::toString(hit.covariance())<<", inverse: "
                             <<std::endl<<Amg::toString(hit.covariance().inverse())<<", "<<endmsg;
                msg<<printLvl<<"Chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermStrip(const Amg::Vector3D& segPos,
                              const Amg::Vector3D& segDir,
                              std::optional<double> timeOfArrival,
                              const CalibratedSpacePoint& strip,
                              MsgStream& msg) {
            const SpacePoint* sp = strip.spacePoint();
            const Amg::Vector3D normal = sp->normalInChamber().cross(sp->directionInChamber());
            std::optional<double> travelledDist = Amg::intersect<3>(segPos, segDir, normal, sp->positionInChamber().z());
            const Amg::Vector3D planeCrossing = segPos + travelledDist.value_or(0) * segDir;
            const Amg::Vector2D residual{(planeCrossing - strip.positionInChamber()).block<2,1>(0,0)};
            double chi2 = residual.dot(strip.covariance().inverse()* residual);
            double timeChi2{timeOfArrival ? std::pow( (strip.time() - (*timeOfArrival) - 
                                                      travelledDist.value_or(0) * c_inv), 2) / strip.timeCovariance() : 0.};
            chi2 += timeChi2;
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Calibrated measurement "<<sp->chamber()->idHelperSvc()->toString(sp->identify())
                             <<", position: "<<Amg::toString(strip.positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: "<<std::endl<<Amg::toString(strip.covariance())<<", inverse: "
                             <<std::endl<<Amg::toString(strip.covariance().inverse())<<endmsg;
                if (timeOfArrival){
                    msg<<printLvl<<"Time: "<<strip.time()<<", time of arrival: "
                                 <<( (*timeOfArrival) - travelledDist.value_or(0) * c_inv)<<", uncert: "
                                 <<std::sqrt(strip.timeCovariance())<<", chi2: "<<timeChi2;
                }
                msg<<printLvl<<" -- final chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermBeamspot(const Amg::Vector3D& segPos,
                                 const Amg::Vector3D& segDir,
                                 const CalibratedSpacePoint& beamSpotMeas,
                                 MsgStream& msg) {

            const Amg::Vector3D planeCrossing{segPos + Amg::intersect<3>(segPos, segDir, Amg::Vector3D::UnitZ(), 
                                                                         beamSpotMeas.positionInChamber().z()).value_or(0) * segDir};

            const Amg::Vector2D residual = (planeCrossing - beamSpotMeas.positionInChamber()).block<2,1>(0,0);
            const double chi2 = residual.dot(beamSpotMeas.covariance().inverse() * residual);
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Print beamspot constraint "<<Amg::toString(beamSpotMeas.positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: "<<std::endl<<Amg::toString(beamSpotMeas.covariance())<<std::endl
                             <<", inverse: "<<std::endl<<Amg::toString(beamSpotMeas.covariance().inverse())<<std::endl
                             <<"chi2 term: "<<chi2<<endmsg;
            }
            return chi2;
        }
    }
}