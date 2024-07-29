/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"

using namespace MuonR4;

double SegmentFitHelpers::chiSqTermMdt(double y0, double tanTheta, const MuonR4::HoughHitType & measurement){
    /// TODO FIXME Johannes (@jojungge / @the.minions): Fix the -1 sign flip in your algebra!!!!! !
    const double theta{std::atan(-tanTheta)};
    const Amg::Vector3D dir{0., -std::sin(theta), std::cos(theta)};
    const Amg::Vector3D x0{y0 * Amg::Vector3D::UnitY()};
    double dchi2 = std::abs((x0 - measurement->positionInChamber()).cross(dir).dot(Amg::Vector3D::UnitX())) - measurement->driftRadius();
    return dchi2*dchi2 / (measurement->uncertainty().y()*measurement->uncertainty().y()); 
}

double SegmentFitHelpers::chiSqTermStrip(double x0, double y0, double tanPhi, double tanTheta, const MuonR4::HoughHitType & measurement){
    Amg::Vector2D residual{x0 + measurement->positionInChamber().z() * tanPhi - measurement->positionInChamber().x(), 
                         y0 + measurement->positionInChamber().z() * tanTheta - measurement->positionInChamber().y()
                        }; 
    AmgSymMatrix(2) weightMatrix = measurement->covariance().inverse(); 
    return residual.dot(weightMatrix * residual); 
}
double SegmentFitHelpers::segmentChiSquare(const double* par, const std::vector<MuonR4::HoughHitType> & hits, std::vector<double> & chi2PerMeas, const ActsGeometryContext & gctx, bool doBSConstraint){

    double y0 = par[(int)MuonSegmentFitterEventData::parameterIndices::y0];
    double x0 = par[(int)MuonSegmentFitterEventData::parameterIndices::x0];
    double tanTheta = par[(int)MuonSegmentFitterEventData::parameterIndices::tanTheta];
    double tanPhi = par[(int)MuonSegmentFitterEventData::parameterIndices::tanPhi];
    double chi2{0.}, nDF{-2.};
    size_t iHit{0u},nPhi{0u}; 
    for (auto & hit : hits){
        double localchi2 = 0; 
        switch (hit->type()){
            case xAOD::UncalibMeasType::MdtDriftCircleType: 
                localchi2 = SegmentFitHelpers::chiSqTermMdt(y0,tanTheta, hit);
                ++nDF; 
                break; 
            case xAOD::UncalibMeasType::TgcStripType:
            case xAOD::UncalibMeasType::RpcStripType: 
                localchi2 = SegmentFitHelpers::chiSqTermStrip(x0,y0,tanPhi,tanTheta, hit);
                nPhi+=hit->measuresPhi(); 
                nDF+=2; 
                break; 
            case xAOD::UncalibMeasType::MMClusterType: 
                // FIXME TODO
                break; 
            case xAOD::UncalibMeasType::sTgcStripType: 
                // FIXME TODO
                break; 
            default: 
                break;
        }
        chi2PerMeas.at(iHit) = localchi2;
        chi2 += localchi2;  
        ++iHit; 
    }
    if(doBSConstraint && 0 < nPhi && nPhi < 3){
        double beamspotchi2 = SegmentFitHelpers::chiSqTermBeamspot(x0,y0,tanPhi,tanTheta, hits.front(), gctx);
        nDF+=2;
        chi2 += beamspotchi2;
    }
    return (nDF != 0 ?  chi2/nDF : chi2); 
}

double SegmentFitHelpers::chiSqTermBeamspot(double x0, double y0, double tanPhi, double tanTheta, 
                                            const MuonR4::HoughHitType & hit, const ActsGeometryContext & gctx){
    Amg::Vector3D beamSpotVector = hit->chamber()->globalToLocalTrans(gctx).translation();
    AmgSymMatrix(3) covariance (AmgSymMatrix(3)::Identity()); 
    /// placeholder for a very generous beam spot: 300mm in X,Y (tracking volume), 20000 along Z
    covariance(0,0) = 300.*300;
    covariance(1,1) = 300.*300;
    covariance(2,2) = 20000.*20000;
    AmgSymMatrix(3) jacobian =  hit->chamber()->globalToLocalTrans(gctx).linear();
    covariance = jacobian * covariance * jacobian.transpose(); 
    covariance = covariance.inverse().eval();
    Amg::MatrixX covariance2(2,2);
    covariance2 = covariance.block<2,2>(0,0);

    Amg::Vector2D residual {x0 + beamSpotVector.z() * tanPhi - beamSpotVector.x(),
                            y0 + beamSpotVector.z() * tanTheta - beamSpotVector.y()};
    return residual.dot(covariance2 * residual);
}
