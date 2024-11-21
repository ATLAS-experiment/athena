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
#include "MuonSpacePoint/UtilFunctions.h"
#include "MuonPatternHelpers/MatrixUtils.h"

namespace{
    constexpr double  c_inv = 1. / Gaudi::Units::c_light;
    constexpr auto printLvl = MSG::VERBOSE;
}


namespace MuonR4 {
    namespace SegmentFitHelpers{
        using namespace SegmentFit;
        using State = CalibratedSpacePoint::State;


        double chiSqTerm(const Amg::Vector3D& posInChamber,
                         const Amg::Vector3D& dirInChamber,
                         const MuonR4::SpacePoint& measurement,
                         MsgStream& msg) {
            double chi2{0.};
            switch (measurement.type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: 
                    chi2 = chiSqTermMdt(posInChamber, dirInChamber,
                                        measurement, msg);
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                case xAOD::UncalibMeasType::TgcStripType:
                    chi2 = chiSqTermStrip(posInChamber, dirInChamber, measurement, msg);
                    break;
                default:
                    if (msg.level() <= MSG::WARNING) {
                        msg<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" - Unsupported measurement: "
                            <<measurement.msSector()->idHelperSvc()->toString(measurement.identify())<<endmsg;
                    }
            }
            return chi2;
        }
        double chiSqTermMdt(const Amg::Vector3D& segPos,
                            const Amg::Vector3D& segDir,
                            const MuonR4::SpacePoint& mdtSP,
                            MsgStream& msg) {
                        
            Amg::Vector2D residual{Amg::Vector2D::Zero()};
    
            residual[Amg::y] = std::abs(Amg::signedDistance(mdtSP.positionInChamber(), mdtSP.directionInChamber(), segPos, segDir)) 
                             - mdtSP.driftRadius();
            const double chi2{residual.dot(mdtSP.covariance().inverse()* residual)};
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Measurement "<<mdtSP.msSector()->idHelperSvc()->toString(mdtSP.identify());
                msg<<printLvl<<", position: "<<Amg::toString(mdtSP.positionInChamber())
                             <<", driftRadius: "<<mdtSP.driftRadius()
                             <<", status: "<<static_cast<const xAOD::MdtDriftCircle*>(mdtSP.primaryMeasurement())->status()<<endmsg;
                msg<<printLvl<<"segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<","<<endmsg;
                msg<<printLvl<<"residual: "<<Amg::toString(residual) <<", covariance: "<<std::endl
                             <<Amg::toString(mdtSP.covariance())<<std::endl<<", inverse: "
                             <<std::endl<<Amg::toString(mdtSP.covariance().inverse())<<", "<<endmsg;
                msg<<printLvl<<"chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermStrip(const Amg::Vector3D& segPos,
                              const Amg::Vector3D& segDir,
                              const MuonR4::SpacePoint& stripSP,
                              MsgStream& msg){
            const Amg::Vector3D normal = stripSP.planeNormal();
            std::optional<double> travelledDist = Amg::intersect<3>(segPos, segDir, normal, normal.dot(stripSP.positionInChamber()));
            const Amg::Vector3D planeCrossing = segPos + travelledDist.value_or(0) * segDir; 

            const Amg::Vector2D residual{(planeCrossing - stripSP.positionInChamber()).block<2,1>(0,0)};
            const double chi2 = residual.dot(stripSP.covariance().inverse()* residual);
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Measurement "<<stripSP.msSector()->idHelperSvc()->toString(stripSP.identify())
                             <<", position: "<<Amg::toString(stripSP.positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: " <<std::endl<<Amg::toString(stripSP.covariance())<<", inverse: "
                             <<std::endl<<Amg::toString(stripSP.covariance().inverse())<<","<<endmsg;
                msg<<printLvl<<"chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }

        double chiSqTerm(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                         const double timeOffset, std::optional<double> arrivalTime,
                         const CalibratedSpacePoint& hit,
                         MsgStream& msg) {
            double chi2{0.};
            switch (hit.type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType:
                    chi2 = chiSqTermMdt(segPos, segDir, hit, msg); 
                    break;
                case xAOD::UncalibMeasType::RpcStripType:
                case xAOD::UncalibMeasType::TgcStripType:
                    chi2 = chiSqTermStrip(segPos, segDir, timeOffset, arrivalTime, hit, msg);
                    break; 
                case xAOD::UncalibMeasType::Other:
                    chi2 = chiSqTermBeamspot(segPos,segDir, hit, msg);
                    break;
                default:
                   if (msg.level() <= MSG::WARNING) {
                        msg<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" - Unsupported measurement: "
                            <<hit.spacePoint()->msSector()->idHelperSvc()->toString(hit.spacePoint()->identify())<<endmsg;
                    }
            }
            return chi2;
        }

        double chiSqTermMdt(const Amg::Vector3D& segPos,
                            const Amg::Vector3D& segDir,
                            const CalibratedSpacePoint& hit,
                            MsgStream& msg) {
            /// Invalid spacepoints do not contribute to the chi2
            if (hit.fitState() != State::Valid) return 0.;
            const Amg::Vector3D& hitPos{hit.positionInChamber()};
            const Amg::Vector3D& hitDir{hit.directionInChamber()};

            // Calculate the closest point along the segment to the wire 
            const Amg::Vector3D closePointSeg = segPos + Amg::intersect<3>(hitPos, hitDir, segPos, segDir).value_or(0)* segDir;
            // Closest point along the wire to the segment
            const Amg::Vector3D closePointWire = hitPos + hitDir.dot(closePointSeg - hitPos) * hitDir;
            Amg::Vector2D residual{Amg::Vector2D::Zero()};
            residual[Amg::x] = (hitPos - closePointSeg).x();
            residual[Amg::y] = (closePointWire - closePointSeg).mag() - hit.driftRadius();
            const double chi2{contract(inverse(hit.covariance()), residual)};
            if (msg.level() <= printLvl) {
                const SpacePoint* sp = hit.spacePoint();
                msg<<printLvl<<"Calibrated measurement "<<sp->msSector()->idHelperSvc()->toString(sp->identify());
                msg<<printLvl<<", position: "<<Amg::toString(hit.positionInChamber())<<", driftRadius: "<<hit.driftRadius()
                             <<", dimension: "<<sp->dimension()<<endmsg;
                msg<<printLvl<<", segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<", residual: "<<Amg::toString(residual)<<"/ "<<
                             (Amg::lineDistance<3>(hitPos, hitDir, segPos, segDir) -hit.driftRadius())<<std::endl <<", covariance: "<<std::endl
                             <<toString(hit.covariance())<<", inverse: "
                             <<std::endl<<toString(inverse(hit.covariance()))<<", "<<endmsg;
                msg<<printLvl<<"Chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermStrip(const Amg::Vector3D& segPos,
                              const Amg::Vector3D& segDir,
                              const double offsetTime,
                              std::optional<double> arrivalTime,
                              const CalibratedSpacePoint& strip,
                              MsgStream& msg) {
            
            /// Invalid spacepoints do not contribute to the chi2
            if (strip.fitState() != State::Valid) return 0.;
 
            const SpacePoint* spacePoint = strip.spacePoint();
            const Amg::Vector3D normal = spacePoint->planeNormal();
            std::optional<double> travelledDist = Amg::intersect<3>(segPos, segDir, normal, normal.dot(spacePoint->positionInChamber()));
            const Amg::Vector3D planeCrossing = segPos + travelledDist.value_or(0) * segDir;
            Amg::Vector3D residual{(planeCrossing - strip.positionInChamber())};
            if (strip.measuresTime() && arrivalTime) {
                residual[Amg::z] = strip.time() - (*arrivalTime) - travelledDist.value_or(0) * c_inv  - offsetTime;
            } else  {
                residual[Amg::z] = 0;
            }
            const double chi2 = contract(inverse(strip.covariance()), residual);
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Calibrated measurement "<<spacePoint->msSector()->idHelperSvc()->toString(spacePoint->identify())
                             <<", position: "<<Amg::toString(strip.positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: "<<std::endl<<toString(strip.covariance())<<", inverse: "
                             <<std::endl<<toString(inverse(strip.covariance()))<<endmsg;
                msg<<printLvl<<" -- final chi2 term: "<<chi2<<endmsg;
            }
            assert(chi2 >=0.);
            return chi2;
        }
        double chiSqTermBeamspot(const Amg::Vector3D& segPos,
                                 const Amg::Vector3D& segDir,
                                 const CalibratedSpacePoint& beamSpotMeas,
                                 MsgStream& msg) {

            /// Invalid spacepoints do not contribute to the chi2
            if (beamSpotMeas.fitState() != State::Valid) return 0.;
            const Amg::Vector3D planeCrossing{segPos + Amg::intersect<3>(segPos, segDir, Amg::Vector3D::UnitZ(), 
                                                                         beamSpotMeas.positionInChamber().z()).value_or(0) * segDir};

            const Amg::Vector2D residual = (planeCrossing - beamSpotMeas.positionInChamber()).block<2,1>(0,0);
            const double chi2 =  contract(inverse(beamSpotMeas.covariance()), residual);
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Print beamspot constraint "<<Amg::toString(beamSpotMeas.positionInChamber())<<endmsg;
                msg<<printLvl<<"Segment: "<<Amg::toString(segPos) <<" + x "<<Amg::toString(segDir)<<endmsg;
                msg<<printLvl<<"Plane crossing: "<<Amg::toString(planeCrossing)<<", residual: "<<Amg::toString(residual)
                             <<", covariance: "<<std::endl<<toString(beamSpotMeas.covariance())<<std::endl
                             <<", inverse: "<<std::endl<<toString(inverse(beamSpotMeas.covariance()))<<std::endl
                             <<"chi2 term: "<<chi2<<endmsg;
            }
            return chi2;
        }
        std::vector<int> driftSigns(const Amg::Vector3D& segPos,
                                    const Amg::Vector3D& segDir,
                                    const std::vector<const SpacePoint*>& uncalibHits,
                                    MsgStream& msg) {

            std::vector<int> signs{};
            signs.reserve(uncalibHits.size());
            for (const SpacePoint* sp : uncalibHits) {
                signs.push_back( sp ? driftSign(segPos,segDir, *sp, msg) : 0);                
            }            
            return signs;
        }
        std::vector<int> driftSigns(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                                    const std::vector<std::unique_ptr<CalibratedSpacePoint>>& calibHits,
                                    MsgStream& msg) {
            std::vector<int> signs{};
            signs.reserve(calibHits.size());
            for (const std::unique_ptr<CalibratedSpacePoint>& hit : calibHits) {
                signs.push_back(driftSign(segPos,segDir, *hit, msg));
            }
            return signs;
        }
        int driftSign(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                       const SpacePoint& sp, MsgStream& msg) {
            if (sp.type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
                return 0;
            }
            const double signedDist = Amg::signedDistance(segPos, segDir, sp.positionInChamber(), sp.directionInChamber());
            if (msg.level() <= printLvl) {
                msg<<printLvl<<"Hit "<<sp.msSector()->idHelperSvc()->toString(sp.identify())<<" drift radius "<<sp.driftRadius()
                                <<", signed distance: "<<signedDist<<", unsigned distance: "
                                <<Amg::lineDistance<3>(segPos, segDir, sp.positionInChamber(), sp.directionInChamber())<<endmsg;
            }
            return sign(signedDist);
        }
        int driftSign(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                      const CalibratedSpacePoint& calibHit,  MsgStream& msg) {
            if (calibHit.type() != xAOD::UncalibMeasType::MdtDriftCircleType){
                return 0;
            }
            const double signedDist = Amg::signedDistance(segPos, segDir, calibHit.positionInChamber(), calibHit.directionInChamber());
            if (msg.level() <= printLvl) {
                const SpacePoint* sp = calibHit.spacePoint();
                msg<<printLvl<<"Hit "<<sp->msSector()->idHelperSvc()->toString(sp->identify())<<" drift radius "<<calibHit.driftRadius()
                                <<", signed distance: "<<signedDist<<", unsigned distance: "
                                <<Amg::lineDistance<3>(segPos, segDir, calibHit.positionInChamber(), calibHit.directionInChamber())<<endmsg;
            }
            return sign(signedDist);
        }
        std::pair<std::vector<double>, double> postFitChi2PerMas(const SegmentFit::Parameters& segPars,
                                                                 std::optional<double> arrivalTime,
                                                                 std::vector<std::unique_ptr<CalibratedSpacePoint>>& hits,
                                                                 MsgStream& msg) {

            std::vector<double> measChi2{};
            double chi2{0.};
            const auto [segPos, segDir] = SegmentFit::makeLine(segPars);
            for (std::unique_ptr<CalibratedSpacePoint>& hit : hits) {
                if (hit->fitState() == State::Outlier){
                    hit->setFitState(State::Valid);
                    measChi2.push_back(chiSqTerm(segPos,segDir, segPars[toInt(ParamDefs::time)],arrivalTime, *hit, msg));
                    hit->setFitState(State::Outlier);
                } else {
                    measChi2.push_back(chiSqTerm(segPos,segDir, segPars[toInt(ParamDefs::time)],arrivalTime, *hit, msg));
                    chi2+= measChi2.back();
                }
            }
            if (msg.level() <= MSG::VERBOSE) {
                msg<<MSG::VERBOSE<<" Overall chi2: "<<chi2<<"."<<endmsg;
            }
            return std::make_pair(std::move(measChi2), chi2);
        }
    }
}
