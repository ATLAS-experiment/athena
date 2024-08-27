/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonSpacePointCalibrator/ISpacePointCalibrator.h>
#include <TrkEventPrimitives/ParamDefs.h>
#include <EventPrimitives/EventPrimitivesCovarianceHelpers.h>
#include <GaudiKernel/PhysicalConstants.h>
#include <MuonSpacePoint/UtilFunctions.h>

/*   * Residual strip hit:  R= (P + <P-H|D>*D -H) 
     *        dR                   
     *    --> -- dP = dP + <dP|D>*D
     *        dP
     *      
     *        dR
     *        -- dD = <P-H|D>dD + <P-H|dD>*D
     *        dD
     *
     *    chi2 = <R|1./cov^{2} | R>
     * 
     *   dchi2 = <dR | 1./cov^{2} | R> + <R | 1./cov^{2} | dR>
     */
namespace {
    constexpr double c_inv = 1. / Gaudi::Units::c_light;
    /* Cut off value for the determinant. Hessian matrices with a determinant smaller than this 
       are considered to be invalid */
    constexpr double detCutOff = 1.e-8;
}

namespace MuonR4{
    using namespace SegmentFit;
    using HitType = SegmentFitResult::HitType;
    using HitVec = SegmentFitResult::HitVec;


    MdtSegmentFitter::Config::RangeArray 
        MdtSegmentFitter::Config::defaultRanges() {
            RangeArray rng{};
            constexpr double spatRang = 10.*Gaudi::Units::m;
            constexpr double tanRange = 4;
            constexpr double timeTange = 25 * Gaudi::Units::ns;
            rng[toInt(AxisDefs::y0)] = std::array{-spatRang, spatRang};
            rng[toInt(AxisDefs::x0)] = std::array{-spatRang, spatRang};
            rng[toInt(AxisDefs::tanPhi)] = std::array{-tanRange, tanRange};
            rng[toInt(AxisDefs::tanTheta)] = std::array{-tanRange, tanRange};
            rng[toInt(AxisDefs::time)] = std::array{-timeTange, timeTange};            
            return rng;
    }
    MdtSegmentFitter::MdtSegmentFitter(const std::string& name,
                                       Config&& config):
        AthMessaging{name},
        m_cfg{std::move(config)}{}
    
    
    inline Amg::Vector3D MdtSegmentFitter::partialPlaneIntersect(const Amg::Vector3D& normal, const double offset, 
                                                                 const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                                                                 const LinePartialArray& linePartials, const AxisDefs fitPar) {
        const double normDot = normal.dot(segDir);
        switch (fitPar) {
           case AxisDefs::tanPhi:
           case AxisDefs::tanTheta:{                
                const double travelledDist = (offset - segPos.dot(normal)) / normDot;
                const double partialDist = - travelledDist / normDot * normal.dot(linePartials[toInt(fitPar)]);
                return travelledDist * linePartials[toInt(fitPar)] + partialDist * segDir;
                break;
           } case AxisDefs::y0:
             case AxisDefs::x0:
                return linePartials[toInt(fitPar)] - linePartials[toInt(fitPar)].dot(normal) / normDot * segDir;
                break;
            default:
                break;
        }        
       return Amg::Vector3D::Zero();
    }
    

    inline Amg::Vector3D MdtSegmentFitter::partialClosestApproach(const MuonR4::CalibratedSpacePoint& sp,
                                                                  const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                                                                  const LinePartialArray& linePartials, const AxisDefs fitPar) {
        
        const Amg::Vector3D& hitDir{sp.directionInChamber()};
        const Amg::Vector3D& hitPos{sp.positionInChamber()};
        
        const double dirDots = hitDir.dot(segDir);
        const double divisor = (1. - dirDots * dirDots);

        switch (fitPar) {
            case AxisDefs::tanPhi:
            case AxisDefs::tanTheta: {
                const Amg::Vector3D& segDirPartial{linePartials[toInt(fitPar)]};
                const Amg::Vector3D AminusB = hitPos - segPos;
                       
                const double AminusBdotHit = AminusB.dot(hitDir);
                const double numerator = (AminusB.dot(segDir) - AminusBdotHit * dirDots);
                const double travelledDist = numerator / divisor;
                const double partialDirDots = hitDir.dot(segDirPartial);


                const double derivativeDist = (divisor * (AminusB.dot(segDirPartial)  - AminusBdotHit* partialDirDots) 
                                            +  2. *numerator * dirDots *partialDirDots) / (divisor*divisor);
                
                return travelledDist *segDirPartial + derivativeDist * segDir;
                break;
            } 
            case AxisDefs::x0:
            case AxisDefs::y0: {
                const Amg::Vector3D AminusB = -linePartials[toInt(fitPar)];
                const double numerator = (AminusB.dot(segDir) - AminusB.dot(hitDir) * dirDots);
                const double travelledDist = numerator / divisor;
                return linePartials[toInt(fitPar)] + travelledDist * segDir;
            }
            default:
                break;            
        }
        return Amg::Vector3D::Zero();
    }
    inline void MdtSegmentFitter::updateLinePartials(const Parameters& fitPars, const bool doPhi, LinePartialArray& linePartials) const{

            
        /**          x_{0}                           1                   tanPhi
         *  segPos = y_{0}  , segDir = ------------------------------    tanTheta   ->
         *             0               std::hypot(tanPhi, tanTheta, 1)      1
         * 
         *
         *   d segDir                  1                       -tan(phi)tan(theta)
         *  ----------= --------------------------------        tan^{2}(phi) + 1
         *   dtanTheta  (tan^{2}(theta)+tan^{2}(phi) +1)           -tan(theta)
         * 
         *  ---> Note: <d segDir/dtanTheta, segDir> = 0   
         *
         *   d segDir                  1                        tan^{2}(theta) + 1
         *  ----------= --------------------------------       -tan(phi)tan(theta)
         *   dtanPhi    (tan^{2}(theta)+tan^{2}(phi) +1)           -tan(phi)
         * 
         * 
        *******************************************************************************/
        const double tanTheta = fitPars[toInt(AxisDefs::tanTheta)];
        const double tanPhi   = fitPars[toInt(AxisDefs::tanPhi)];
        const double norm  = std::hypot(tanTheta, tanPhi, 1.);
        const double denom = 1. / std::pow(norm, 3);
        linePartials[toInt(AxisDefs::tanTheta)] = denom * Amg::Vector3D{-tanPhi*tanTheta, tanPhi*tanPhi +1., -tanTheta};
        if (doPhi) {
            linePartials[toInt(AxisDefs::tanPhi)] = denom * Amg::Vector3D{tanTheta*tanTheta + 1, -tanPhi*tanTheta, -tanPhi};
        }
        ATH_MSG_VERBOSE("Directional derivatives: dDir/dtanTheta = "<<Amg::toString(linePartials[toInt(AxisDefs::tanTheta)])
                        <<", dDir/dtanPhi ="<<Amg::toString(linePartials[toInt(AxisDefs::tanPhi)]));
    }

    
    SegmentFitResult MdtSegmentFitter::fitSegment(const EventContext& ctx,
                                                  HitVec&& calibHits,
                                                  const Parameters& startPars,
                                                  const Amg::Transform3D& localToGlobal) const {

        const Muon::IMuonIdHelperSvc* idHelperSvc{calibHits[0]->spacePoint()->chamber()->idHelperSvc()};
        using State = CalibratedSpacePoint::State;

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream hitStream{};
            for (const HitType& hit : calibHits) {
                hitStream<<"       **** "<<(hit->type() != xAOD::UncalibMeasType::Other ? idHelperSvc->toString(hit->spacePoint()->identify()): "beamspot" )
                         <<" position: "<<Amg::toString(hit->positionInChamber());
                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                    hitStream<<", driftRadius: "<<hit->driftRadius();
                }
                hitStream<<", channel dir: "<<Amg::toString(hit->directionInChamber())<<std::endl;
            }
            ATH_MSG_VERBOSE("Start segment fit with parameters "<<toString(startPars)
                          <<", plane location: "<<Amg::toString(localToGlobal)<<std::endl<<hitStream.str());

        }

        SegmentFitResult fitResult{};
        fitResult.segmentPars = startPars;
        fitResult.segmentPars[toInt(AxisDefs::time)] = 0.;
        fitResult.timeFit = m_cfg.doTimeFit;
        fitResult.calibMeasurements = std::move(calibHits);
        

        Parameters gradient{AmgVector(5)::Zero()}, prevGrad{AmgVector(5)::Zero()}, prevPars{AmgVector(5)::Zero()};
        AmgSymMatrix(5) hessian{AmgSymMatrix(5)::Zero()};

        /// Cache of the partial derivatives of the line parameters w.r.t the fit parameters
        LinePartialArray linePartials{make_array<Amg::Vector3D, toInt(AxisDefs::nPars)>(Amg::Vector3D::Zero())};
        linePartials[toInt(AxisDefs::x0)] = Amg::Vector3D::UnitX();
        linePartials[toInt(AxisDefs::y0)] = Amg::Vector3D::UnitY();


        /// Partials of the residual w.r.t. the fit parameters
        LinePartialArray partialsResidual{make_array<Amg::Vector3D,toInt(AxisDefs::nPars)>(Amg::Vector3D::Zero())};
        Amg::Vector3D residual{Amg::Vector3D::Zero()};

        while (fitResult.nIter++ < m_cfg.nMaxCalls) {
            ATH_MSG_VERBOSE("Iteration: "<<fitResult.nIter<<" parameters: "<<toString(fitResult.segmentPars)<<"chi2: "<<fitResult.chi2);
            /// Define the current segment line
            const auto [segPos, segDir] = fitResult.makeLine();
            /// First step calibrate the hits
            fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                      fitResult.segmentPars[toInt(AxisDefs::time)]);
            /// Count the phi & time measurements measurements
            fitResult.nPhiMeas = fitResult.nDoF = fitResult.nTimeMeas = 0;

            for (const HitType& hit : fitResult.calibMeasurements) {
                if (hit->fitState() != State::Valid){
                    continue;
                }
                fitResult.nPhiMeas+= hit->measuresPhi();
                fitResult.nDoF+= hit->measuresPhi();
                fitResult.nDoF+= hit->measuresEta();
                /// Mdts are already counted in the measures eta category. Don't count them twice
                fitResult.nDoF += (m_cfg.doTimeFit && hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType && hit->measuresTime());
                fitResult.nTimeMeas+=hit->measuresTime();              
            }
            if (!fitResult.nDoF) {
                ATH_MSG_WARNING("TSCHUUUUUUUUSS Measurements...");
                break;
            }
            if (!fitResult.nPhiMeas){
                fitResult.segmentPars[toInt(AxisDefs::tanPhi)]=0; 
                fitResult.segmentPars[toInt(AxisDefs::x0)]=0;
            }

            fitResult.nDoF = fitResult.nDoF - 2 - (fitResult.nPhiMeas > 0 ? 2 : 0);

            /// Switch off the time fit if too little degrees of freedom are left
            if (fitResult.timeFit && fitResult.nDoF <= 1) {
                fitResult.timeFit = false;
                ATH_MSG_DEBUG("Switch of the time fit because nDoF: "<<fitResult.nDoF);
                fitResult.segmentPars[toInt(AxisDefs::time)] = 0.;
                /// Recalibrate the measurements
                fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                          fitResult.segmentPars[toInt(AxisDefs::time)]);
            ///
            } else if (!fitResult.timeFit && m_cfg.doTimeFit) {
                ATH_MSG_DEBUG("Somehow a measurement is on the narrow ridge of validity. Let's try if the time can be fitted now ");
                fitResult.timeFit = true;
            }
            /// Reset chi2, gradient & Hessian
            fitResult.chi2 = 0;
            hessian.setZero();
            gradient.setZero();
            /// Update the partial derivatives of the direction vector
            updateLinePartials(fitResult.segmentPars, fitResult.nPhiMeas > 0, linePartials);

            /** Loop over the hits to calculate the partial derivatives */
            for (const HitType& hit : fitResult.calibMeasurements) {
                if (hit->fitState() != State::Valid) {
                    continue;
                }
                const Amg::Vector3D& hitPos{hit->positionInChamber()};
                const Amg::Vector3D& hitDir{hit->directionInChamber()};
                const unsigned int dim =  hit->type() == xAOD::UncalibMeasType::Other ? 2 : hit->spacePoint()->dimension();
                /// Which parameters are affected by the residual
                const int start = toInt(dim == 2 || hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType ? AxisDefs::tanPhi : AxisDefs::tanTheta);
                switch (hit->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType: {
                        /// Calculate the closest approach to the wire along the segment 
                        const double travelledDist = Amg::intersect(hitPos, hitDir, segPos, segDir).value_or(0);
                        const Amg::Vector3D closePointSeg = segPos + travelledDist* segDir;
                        /// Closest approach along the wire to that point
                        const Amg::Vector3D closePointWire = hitPos + hitDir.dot(closePointSeg - hitPos) * hitDir;

                        const Amg::Vector3D lineConnect = (closePointWire - closePointSeg);
                        const double lineDist = lineConnect.mag();
                        /// Reset the explicit time residual
                        residual[Amg::z] = 0.;
                        residual[Amg::y] = lineDist - hit->driftRadius();
                        
                        // For twin tubes, the hit position is not updated during the calibration. Make use of 
                        // additional constraint to fit phi
                        residual[Amg::x] =  dim == 2 ? (hitPos - closePointSeg).x() : 0.;
                        /// Update the residual partial derivatives
                        for (int p = start;  p >= 0; --p) {
                            const AxisDefs par{static_cast<AxisDefs>(p)};
                            /// Derivative of the closest approach along trajectory w.r.t. fit parameter
                            const Amg::Vector3D partialClosePointOnSeg = partialClosestApproach(*hit, segPos, segDir, linePartials, par);
                            /// Propagation to the closest point along the wire
                            const Amg::Vector3D partialClosePointW = hitDir.dot(partialClosePointOnSeg) * hitDir;

                            partialsResidual[toInt(par)][Amg::x] = dim ==2 ? -partialClosePointOnSeg.x() : 0.;
                            partialsResidual[toInt(par)][Amg::y] = lineConnect.dot(partialClosePointW - partialClosePointOnSeg) / lineDist;
  
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));
                        
                        }
                        /// Calculate the time derivative
                        if (fitResult.timeFit) {
                            /// Currently there's no explicit derivative function available. solve it via the calibrator
                            constexpr double stepSize = 1.e-7;
                            constexpr AxisDefs par = AxisDefs::time;
                            HitType timeUp = m_cfg.calibrator->calibrate(ctx, hit->spacePoint(), segPos, segDir,
                                                                         fitResult.segmentPars[toInt(par)] + stepSize);
                            HitType timeDn = m_cfg.calibrator->calibrate(ctx, hit->spacePoint(), segPos, segDir,
                                                                         fitResult.segmentPars[toInt(par)] - stepSize);
                            
                            
                            partialsResidual[toInt(par)][Amg::x] = partialsResidual[toInt(par)][Amg::z] = 0;
                            /// Only update if the calibrator didn't kill the hit
                            if (timeUp->fitState() == State::Valid && timeDn->fitState() == State::Valid) {
                                partialsResidual[toInt(par)][Amg::y] = - 0.5 *(timeUp->driftRadius() - timeDn->driftRadius()) / stepSize;
                                ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                             <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="<<Amg::toString(partialsResidual[toInt(par)]));
                            } else {
                                partialsResidual[toInt(par)][Amg::y] = 0.;
                                --fitResult.nDoF;                               
                            }
                        }
                        break;
                    }
                    case xAOD::UncalibMeasType::RpcStripType: 
                    case xAOD::UncalibMeasType::TgcStripType:{
                        const Amg::Vector3D normal = hit->spacePoint()->planeNormal();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segPos 
                                                       + Amg::intersect<3>(segPos, segDir, normal, planeOffSet).value_or(0)* segDir; 

                        /// The complementary coordinate does not contribute if the measurement is 1D
                        residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);

                        if (fitResult.timeFit && hit->measuresTime()) {
                           /// need to calculate the global time of flight
                           const double totFlightDist = (localToGlobal * planeIsect).mag();
                           residual[Amg::z] = hit->time() - totFlightDist * c_inv - fitResult.segmentPars[toInt(AxisDefs::time)];
                        }

                        for (int p = start;  p >= 0; --p) {
                            const AxisDefs par{static_cast<AxisDefs>(p)};
                            partialsResidual[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet,
                                                                                                    segPos, segDir,linePartials, par).block<2,1>(0,0);

                            if (fitResult.timeFit && hit->measuresTime()) {
                                partialsResidual[toInt(par)][Amg::z] = -partialsResidual[toInt(par)].perp() * c_inv;
                            }
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));
                        
                        }
                        if (fitResult.timeFit && hit->measuresTime()) {
                           constexpr AxisDefs par = AxisDefs::time;
                           partialsResidual[toInt(par)] = -Amg::Vector3D::UnitZ();
                           ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));
                       }
                       break;
                    } case xAOD::UncalibMeasType::Other:{
                        static const Amg::Vector3D normal = Amg::Vector3D::UnitZ();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segPos 
                                                       + Amg::intersect<3>(segPos, segDir, normal, planeOffSet).value_or(0)* segDir;
                        
                       
                        residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);
                        residual[Amg::z] =0.;
                        for (int p = start;  p >= 0; --p) {
                            const AxisDefs par{static_cast<AxisDefs>(p)};
                            partialsResidual[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet,
                                                                                                    segPos, segDir,linePartials, par).block<2,1>(0,0);

                            partialsResidual[toInt(par)][Amg::z] = 0;
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));
                        
                        }
                        break;
                    } default:
                        ATH_MSG_WARNING("MdtSegmentFitter() - Unsupported measurment type" <<typeid(*hit->spacePoint()).name());
                }
    
                ATH_MSG_VERBOSE("Update derivatives for hit "<< (hit->spacePoint() ? idHelperSvc->toString(hit->spacePoint()->identify()) : "beamspot"));
                updateDerivatives(residual, partialsResidual, hit->covariance(), gradient, hessian, fitResult.chi2, 
                                  fitResult.timeFit && hit->measuresTime() ? toInt(AxisDefs::time) : start);
            }
 
            /// Loop over hits is done. Symmetrise the Hessian
            for (int k =1; k < 5 - (!fitResult.timeFit); ++k){
                for (int l = 0; l< k; ++l){
                    hessian(l,k) = hessian(k,l);
                }
            }
            /// Check whether the gradient is already sufficiently small
            if (gradient.mag() < m_cfg.tolerance) {
                fitResult.converged = true;
                ATH_MSG_VERBOSE("Fit converged after "<<fitResult.nIter<<" iterations with "<<fitResult.chi2);
                break;
            }
            ATH_MSG_VERBOSE("Chi2: "<<fitResult.chi2<<", gradient: "<<Amg::toString(gradient)<<"hessian: "<<std::endl<<hessian);
            /// Pure eta segment fit
            if (!fitResult.nPhiMeas && !fitResult.timeFit) {
                if(!updateParameters<2>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian)){
                    fitResult.chi2PerMeasurement.resize(fitResult.calibMeasurements.size(), -1);
                    return fitResult;
                }
            } else if (!fitResult.nPhiMeas && fitResult.timeFit) {
                /// In the case that the time is fit & that there are no phi measurements -> compress matrix by swaping
                /// the time column with whaever the second column is... it's zero
                hessian.col(2).swap(hessian.col(toInt(AxisDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(AxisDefs::time)));
                std::swap(gradient[2], gradient[toInt(AxisDefs::time)]);
                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(AxisDefs::time)]);
                if (!updateParameters<3>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian)) {
                    fitResult.chi2PerMeasurement.resize(fitResult.calibMeasurements.size(), -1);
                    return fitResult;
                }
                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(AxisDefs::time)]);
                hessian.col(2).swap(hessian.col(toInt(AxisDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(AxisDefs::time)));
                std::swap(gradient[2], gradient[toInt(AxisDefs::time)]);
            } else if (fitResult.nPhiMeas && !fitResult.timeFit) {
                if (!updateParameters<4>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian)){
                    fitResult.chi2PerMeasurement.resize(fitResult.calibMeasurements.size(), -1);
                    return fitResult;
                }
            } else if (fitResult.nPhiMeas && fitResult.timeFit) {
                if(!updateParameters<5>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian)){
                    fitResult.chi2PerMeasurement.resize(fitResult.calibMeasurements.size(), -1);
                    return fitResult;
                }
            } 
        }
        
        /// Subtract 1 degree of freedom to take the time into account
        fitResult.nDoF-=fitResult.timeFit;
        
        /// Calculate the chi2 per measurement
        const auto [segPos, segDir] = fitResult.makeLine();
        fitResult.chi2 =0.;
        
        std::optional<double> toF = fitResult.timeFit ? std::make_optional<double>((localToGlobal * segPos).mag() * c_inv) : std::nullopt;
        /** Sort the measurements by ascending z */
        std::ranges::stable_sort(fitResult.calibMeasurements, [](const HitType&a, const HitType& b){
                return a->positionInChamber().z() > b->positionInChamber().z();
        });
        
        auto [chi2Term, chi2]= SegmentFitHelpers::postFitChi2PerMas(fitResult.segmentPars, toF, fitResult.calibMeasurements, msg());
        fitResult.chi2PerMeasurement = std::move(chi2Term);
        fitResult.chi2 = chi2;
        /// Update the covariance
        if (!fitResult.nPhiMeas&& !fitResult.timeFit) {
            blockCovariance<2>(std::move(hessian), /* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        }else if (!fitResult.nPhiMeas && fitResult.timeFit) {
            hessian.col(2).swap(hessian.col(toInt(AxisDefs::time)));
            hessian.row(2).swap(hessian.row(toInt(AxisDefs::time)));
            blockCovariance<3>(std::move(hessian),/* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
            fitResult.segmentParErrs.col(2).swap(fitResult.segmentParErrs.col(toInt(AxisDefs::time)));
            fitResult.segmentParErrs.row(2).swap(fitResult.segmentParErrs.row(toInt(AxisDefs::time)));
        } else if (fitResult.nPhiMeas) {
            blockCovariance<4>(std::move(hessian), /* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        } else if (fitResult.nPhiMeas && fitResult.timeFit) {
            blockCovariance<5>(std::move(hessian),/* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        }
        return fitResult;    
    }

    void MdtSegmentFitter::updateDerivatives(const Amg::Vector3D& residual,
                                             const LinePartialArray& residualPartials,
                                             const MeasCov_t& measCovariance,
                                             AmgVector(5)& gradient, 
                                             AmgSymMatrix(5)& hessian,
                                             double& chi2, int startPar) const {
            
        const MeasCov_t invCov{inverse(measCovariance)};

        const Amg::Vector3D covRes = multiply(invCov, residual);
        chi2 += covRes.dot(residual);
        for (int p = startPar; p >=0 ; --p) {
            gradient[p] +=2.*covRes.dot(residualPartials[p]);
            for (int k=p; k>=0; --k) {
                hessian(p,k)+= 2.*contract(invCov, residualPartials[p], residualPartials[k]);
            }
        }
        ATH_MSG_VERBOSE("After derivative update --- chi2: "<<chi2<<"("<<covRes.dot(residual)<<"), gradient: "
                      <<toString(gradient)<<", Hessian:\n"<<hessian<<", measurement covariance\n"<<toString(invCov));
    }

    template <unsigned int nDim>
        void MdtSegmentFitter::blockCovariance(const AmgSymMatrix(5)& hessian,
                                               SegmentFit::Covariance& covariance) const {

            covariance.setIdentity();
            AmgSymMatrix(nDim) miniHessian = hessian.block<nDim, nDim>(0,0);
            if (std::abs(miniHessian.determinant()) <= detCutOff) {
                ATH_MSG_ALWAYS("Boeser mini hessian ("<<miniHessian.determinant()<<")\n"<<miniHessian<<"\n\n"<<hessian);
                return;
            }
            ATH_MSG_VERBOSE("Hessian matrix: \n"<<hessian<<",\nblock Hessian:\n"<<miniHessian<<",\n determinant: "<<miniHessian.determinant());
            covariance.block<nDim,nDim>(0,0) = miniHessian.inverse();
            ATH_MSG_VERBOSE("covariance: \n"<<covariance);
    }

    template <unsigned int nDim> 
        bool MdtSegmentFitter::updateParameters(Parameters& currPars, Parameters& prevPars,
                                                Parameters& currGrad, Parameters& prevGrad,
                                                const AmgSymMatrix(5)& currentHessian) const {
            
            AmgSymMatrix(nDim) miniHessian = currentHessian.block<nDim, nDim>(0,0);
            ATH_MSG_VERBOSE("Parameter update -- \ncurrenPars: "<<toString(currPars)<<", \ngradient: "<<toString(currGrad)
                          <<", hessian ("<<miniHessian.determinant()<<")"<<std::endl<<miniHessian);

            if (std::abs(miniHessian.determinant()) > detCutOff) {
                prevPars.block<nDim,1>(0,0) = currPars.block<nDim,1>(0,0);
                // Update the parameters accrodingly to the hessian
                currPars.block<nDim,1>(0,0) -= miniHessian.inverse()* currGrad.block<nDim, 1>(0,0);
                prevGrad.block<nDim,1>(0,0)  = currGrad.block<nDim,1>(0,0);
                ATH_MSG_VERBOSE("Hessian inverse:\n"<<miniHessian.inverse()<<"\n\nUpdate the parameters by -"
                             <<Amg::toString(miniHessian.inverse()* currGrad.block<nDim, 1>(0,0)));
            } else {
                const AmgVector(nDim) gradDiff = (currGrad - prevGrad).block<nDim,1>(0,0);
                const double gradDiffMag = gradDiff.mag2();
                const double gamma = std::abs((currPars - prevPars).block<nDim,1>(0,0).dot(gradDiff))
                                   / gradDiffMag > std::numeric_limits<double>::epsilon() ? gradDiffMag : 1.;
                ATH_MSG_VERBOSE("Hessian determinant invalid. Try deepest descent - \nprev parameters: "
                             <<toString(prevPars)<<",\nprevious gradient: "<<toString(prevGrad)<<", gamma: "<<gamma);
                prevPars.block<nDim, 1>(0,0) = currPars.block<nDim, 1>(0,0);
                currPars.block<nDim, 1>(0,0) -= gamma* currGrad.block<nDim, 1>(0,0);
                prevGrad.block<nDim, 1>(0,0) = currGrad.block<nDim,1>(0,0);
            }
            /// Check that all parameters remain within the parameter boundary window
            unsigned int nOutOfBound{0};
            for (unsigned int p =0; p< nDim; ++p) {
                if (m_cfg.ranges[p][0] > currPars[p] || m_cfg.ranges[p][1]< currPars[p]) {
                    ATH_MSG_WARNING("The "<<p<<"-th parameter "<<toString(static_cast<AxisDefs>(p))<<" is out of range "<<currPars[p]
                                    <<"["<<m_cfg.ranges[p][0]<<"-"<<m_cfg.ranges[p][1]<<"]");
                    ++nOutOfBound;
                }
                currPars[p] = std::clamp(currPars[p], m_cfg.ranges[p][0], m_cfg.ranges[p][1]);
            }
            return nOutOfBound <= m_cfg.nParsOutOfBounds;
        }
}
