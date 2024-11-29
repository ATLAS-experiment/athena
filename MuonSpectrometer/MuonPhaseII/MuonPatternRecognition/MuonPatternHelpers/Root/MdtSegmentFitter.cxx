/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <TrkEventPrimitives/ParamDefs.h>
#include <EventPrimitives/EventPrimitivesCovarianceHelpers.h>
#include <GaudiKernel/PhysicalConstants.h>
#include <MuonSpacePoint/UtilFunctions.h>
#include <CxxUtils/sincos.h>
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
            constexpr double timeTange = 25 * Gaudi::Units::ns;
            rng[toInt(ParamDefs::y0)] = std::array{-spatRang, spatRang};
            rng[toInt(ParamDefs::x0)] = std::array{-spatRang, spatRang};
            rng[toInt(ParamDefs::phi)] = std::array{-180.* Gaudi::Units::deg, 180. * Gaudi::Units::deg};
            rng[toInt(ParamDefs::theta)] = std::array{-90. * Gaudi::Units::deg,  90. * Gaudi::Units::deg};
            rng[toInt(ParamDefs::time)] = std::array{-timeTange, timeTange};            
            return rng;
    }
    MdtSegmentFitter::MdtSegmentFitter(const std::string& name, Config&& config):
        AthMessaging{name},
        m_cfg{std::move(config)}{}
    
    
    inline Amg::Vector3D MdtSegmentFitter::partialPlaneIntersect(const Amg::Vector3D& normal, const double offset, 
                                                                 const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                                                                 const LinePartialArray& linePartials, const ParamDefs fitPar) {
        const double normDot = normal.dot(segDir);
        switch (fitPar) {
           case ParamDefs::phi:
           case ParamDefs::theta:{                
                const double travelledDist = (offset - segPos.dot(normal)) / normDot;
                const double partialDist = - travelledDist / normDot * normal.dot(linePartials[toInt(fitPar)]);
                return travelledDist * linePartials[toInt(fitPar)] + partialDist * segDir;
                break;
           } case ParamDefs::y0:
             case ParamDefs::x0:
                return linePartials[toInt(fitPar)] - linePartials[toInt(fitPar)].dot(normal) / normDot * segDir;
                break;
            default:
                break;
        }        
       return Amg::Vector3D::Zero();
    }
    

    inline Amg::Vector3D MdtSegmentFitter::partialClosestApproach(const MuonR4::CalibratedSpacePoint& sp,
                                                                  const Amg::Vector3D& segPos, const Amg::Vector3D& segDir,
                                                                  const LinePartialArray& linePartials, const ParamDefs fitPar) {
        
        const Amg::Vector3D& hitDir{sp.directionInChamber()};
        const Amg::Vector3D& hitPos{sp.positionInChamber()};
        
        const double dirDots = hitDir.dot(segDir);
        const double divisor = (1. - dirDots * dirDots);

        switch (fitPar) {
            case ParamDefs::phi:
            case ParamDefs::theta: {
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
            case ParamDefs::x0:
            case ParamDefs::y0: {
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
    inline void MdtSegmentFitter::updateLinePartials(const Parameters& fitPars, LinePartialArray& linePartials) const{
        /**          x_{0}             cos (phi) sin (theta)
         *  segPos = y_{0}  , segDir = sin (phi) sin (theta)
         *             0                     cos theta
         *                          
         * 
         *   d segDir     cos (phi) cos(theta)     dSegDir     -sin(phi) sin (theta)
         *  ----------=   sin (phi) cos(theta)     -------- =   cos(phi) sin (theta)
         *   dTheta            - sin (theta)        dPhi              0
         * 
        *******************************************************************************/
        const CxxUtils::sincos theta{fitPars[toInt(ParamDefs::theta)]}, 
                               phi{fitPars[toInt(ParamDefs::phi)]};
        linePartials[toInt(ParamDefs::theta)] = Amg::Vector3D{phi.cs*theta.cs, phi.sn*theta.cs, -theta.sn};
        linePartials[toInt(ParamDefs::phi)]   =  Amg::Vector3D{-theta.sn *phi.sn, theta.sn*phi.cs, 0};
        ATH_MSG_VERBOSE("Directional derivatives: dDir/dTheta = "<<Amg::toString(linePartials[toInt(ParamDefs::theta)])
                        <<", dDir/dPhi ="<<Amg::toString(linePartials[toInt(ParamDefs::phi)]));
    }

    
    SegmentFitResult MdtSegmentFitter::fitSegment(const EventContext& ctx,
                                                  HitVec&& calibHits,
                                                  const Parameters& startPars,
                                                  const Amg::Transform3D& localToGlobal) const {

        const Muon::IMuonIdHelperSvc* idHelperSvc{calibHits[0]->spacePoint()->msSector()->idHelperSvc()};
        using State = CalibratedSpacePoint::State;

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream hitStream{};
            const auto [startPos, startDir] = makeLine(startPars);
            for (const HitType& hit : calibHits) {
                hitStream<<"       **** "<<(hit->type() != xAOD::UncalibMeasType::Other ? idHelperSvc->toString(hit->spacePoint()->identify()): "beamspot" )
                         <<" position: "<<Amg::toString(hit->positionInChamber());
                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                    hitStream<<", driftRadius: "<<SegmentFitHelpers::driftSign(startPos,startDir, *hit, msg())*hit->driftRadius() ;
                }
                hitStream<<", channel dir: "<<Amg::toString(hit->directionInChamber())<<std::endl;
            }
            ATH_MSG_VERBOSE("Start segment fit with parameters "<<toString(startPars)
                          <<", plane location: "<<Amg::toString(localToGlobal)<<std::endl<<hitStream.str());

        }

        SegmentFitResult fitResult{};
        fitResult.segmentPars = startPars;
        fitResult.timeFit = m_cfg.doTimeFit;
        fitResult.calibMeasurements = std::move(calibHits);

        Parameters gradient{AmgVector(5)::Zero()}, prevGrad{AmgVector(5)::Zero()}, prevPars{AmgVector(5)::Zero()};
        AmgSymMatrix(5) hessian{AmgSymMatrix(5)::Zero()};

        /// Cache of the partial derivatives of the line parameters w.r.t the fit parameters
        LinePartialArray linePartials{make_array<Amg::Vector3D, toInt(ParamDefs::nPars)>(Amg::Vector3D::Zero())};
        linePartials[toInt(ParamDefs::x0)] = Amg::Vector3D::UnitX();
        linePartials[toInt(ParamDefs::y0)] = Amg::Vector3D::UnitY();

        /// Partials of the residual w.r.t. the fit parameters
        LinePartialArray partialsResidual{make_array<Amg::Vector3D,toInt(ParamDefs::nPars)>(Amg::Vector3D::Zero())};
        Amg::Vector3D residual{Amg::Vector3D::Zero()};

        unsigned int noChangeIter{0};
        while (fitResult.nIter++ < m_cfg.nMaxCalls) {
            ATH_MSG_VERBOSE("Iteration: "<<fitResult.nIter<<" parameters: "<<toString(fitResult.segmentPars)<<"chi2: "<<fitResult.chi2);
            /// Define the current segment line
            const auto [segPos, segDir] = fitResult.makeLine();
            /// First step calibrate the hits
            fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                     fitResult.segmentPars[toInt(ParamDefs::time)]);
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
            if (!fitResult.nPhiMeas) {
                ATH_MSG_VERBOSE("No phi measurements are left.");
                fitResult.segmentPars[toInt(ParamDefs::phi)] = 90. * Gaudi::Units::deg; 
                fitResult.segmentPars[toInt(ParamDefs::x0)] = 0;
            }

            fitResult.nDoF = fitResult.nDoF - 2 - (fitResult.nPhiMeas > 0 ? 2 : 0);

            /// Switch off the time fit if too little degrees of freedom are left
            if (fitResult.timeFit && fitResult.nDoF <= 1) {
                fitResult.timeFit = false;
                ATH_MSG_DEBUG("Switch of the time fit because nDoF: "<<fitResult.nDoF);
                fitResult.segmentPars[toInt(ParamDefs::time)] = 0.;
                /// Recalibrate the measurements
                fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                          fitResult.segmentPars[toInt(ParamDefs::time)]);
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
            updateLinePartials(fitResult.segmentPars, linePartials);

            /** Loop over the hits to calculate the partial derivatives */
            for (const HitType& hit : fitResult.calibMeasurements) {
                if (hit->fitState() != State::Valid) {
                    continue;
                }
                const Amg::Vector3D& hitPos{hit->positionInChamber()};
                const Amg::Vector3D& hitDir{hit->directionInChamber()};
                const unsigned int dim =  hit->type() == xAOD::UncalibMeasType::Other ? 2 : hit->spacePoint()->dimension();
                /// Which parameters are affected by the residual
                const int start = toInt(fitResult.nPhiMeas ? ParamDefs::phi : ParamDefs::theta);
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
                        residual[toInt(AxisDefs::t0)] = 0.;
                        residual[toInt(AxisDefs::eta)] = lineDist - hit->driftRadius();
                        const Amg::Vector3D globApproach = (localToGlobal*closePointSeg).unit();
                        // For twin tubes, the hit position is not updated during the calibration. Make use of 
                        // additional constraint to fit phi
                        residual[toInt(AxisDefs::phi)] =  dim == 2 ? (hitPos - closePointSeg).x() : 0.;
                        /// Update the residual partial derivatives
                        const double driftV{m_cfg.calibrator->driftVelocity(ctx, *hit)};
                        for (int p = start;  p >= 0; --p) {
                            const ParamDefs par{static_cast<ParamDefs>(p)};
                            /// Derivative of the closest approach along trajectory w.r.t. fit parameter
                            const Amg::Vector3D partialClosePointOnSeg = partialClosestApproach(*hit, segPos, segDir, linePartials, par);
                            /// Propagation to the closest point along the wire
                            const Amg::Vector3D partialClosePointW = hitDir.dot(partialClosePointOnSeg) * hitDir;

                            const double dR = -driftV * c_inv * globApproach.dot(linePartials[p]);

                            partialsResidual[p][toInt(AxisDefs::phi)] = dim ==2 ? -partialClosePointOnSeg.x() : 0.;
                            partialsResidual[p][toInt(AxisDefs::eta)] = lineConnect.dot(partialClosePointW - partialClosePointOnSeg) / lineDist;
                            partialsResidual[p][toInt(AxisDefs::t0)] = 0.;
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[p])<<", drift velocity: "<<driftV
                                          <<" in terms of beta: "<<(driftV * c_inv)<<" -> dR: "<<dR);

                        }
                        /// Calculate the time derivative
                        if (fitResult.timeFit) {
                            constexpr int par = toInt(ParamDefs::time);
                            partialsResidual[par] = driftV * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                            <<", driftRadius: "<<hit->driftRadius()<<", residual "<<Amg::toString(residual)
                                            <<" w.r.t "<<toString(ParamDefs::time)<<"="<<Amg::toString(partialsResidual[par]));
                        }
                        break;
                    }
                    case xAOD::UncalibMeasType::RpcStripType: 
                    case xAOD::UncalibMeasType::TgcStripType:{
                        const Amg::Vector3D normal = hit->spacePoint()->planeNormal();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segPos + Amg::intersect<3>(segPos, segDir, normal, planeOffSet).value_or(0)* segDir; 

                        /// The complementary coordinate does not contribute if the measurement is 1D
                        residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);

                        if (fitResult.timeFit && hit->measuresTime()) {
                           /// need to calculate the global time of flight
                           const double totFlightDist = (localToGlobal * planeIsect).mag();
                           residual[toInt(AxisDefs::t0)] = hit->time() - totFlightDist * c_inv - fitResult.segmentPars[toInt(ParamDefs::time)];
                        }

                        for (int p = start;  p >= 0; --p) {
                            const ParamDefs par{static_cast<ParamDefs>(p)};
                            partialsResidual[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet,
                                                                                                    segPos, segDir,linePartials, par).block<2,1>(0,0);

                            if (fitResult.timeFit && hit->measuresTime()) {
                                partialsResidual[toInt(par)][toInt(AxisDefs::t0)] = -partialsResidual[toInt(par)].perp() * c_inv;
                            }
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" "<<
                                          Amg::toString(multiply(inverse(hit->covariance()), residual))
                                          <<" "<<std::endl<<toString(hit->covariance())<<std::endl<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));

                        }
                        if (fitResult.timeFit && hit->measuresTime()) {
                           constexpr ParamDefs par = ParamDefs::time;
                           partialsResidual[toInt(par)] = -Amg::Vector3D::Unit(toInt(AxisDefs::t0));
                           ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(partialsResidual[toInt(par)]));
                       }
                       break;
                    } case xAOD::UncalibMeasType::Other:{
                        static const Amg::Vector3D normal = Amg::Vector3D::UnitZ();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segPos + Amg::intersect<3>(segPos, segDir, normal, planeOffSet).value_or(0)* segDir;

                        residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);
                        residual[toInt(AxisDefs::t0)] =0.;
                        for (int p = start;  p >= 0; --p) {
                            const ParamDefs par{static_cast<ParamDefs>(p)};
                            partialsResidual[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet,
                                                                                                    segPos, segDir,linePartials, par).block<2,1>(0,0);

                            partialsResidual[toInt(par)][toInt(AxisDefs::t0)] = 0;
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
                                  fitResult.timeFit && hit->measuresTime() ? toInt(ParamDefs::time) : start);
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
            UpdateStatus paramUpdate{UpdateStatus::outOfBounds};
            if (!fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<2>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } else if (!fitResult.nPhiMeas && fitResult.timeFit) {
                /// In the case that the time is fit & that there are no phi measurements -> compress matrix by swaping
                /// the time column with whaever the second column is... it's zero
                hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
                std::swap(gradient[2], gradient[toInt(ParamDefs::time)]);
                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(ParamDefs::time)]);

                paramUpdate = updateParameters<3>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 

                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(ParamDefs::time)]);
                hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
                std::swap(gradient[2], gradient[toInt(ParamDefs::time)]);
            } else if (fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<4>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } else if (fitResult.nPhiMeas && fitResult.timeFit) {
                paramUpdate = updateParameters<5>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } 
            if  (paramUpdate == UpdateStatus::noChange) {
                if ((++noChangeIter) >= m_cfg.noMoveIter){
                    fitResult.converged = true;
                    break;
                }
            } else if (paramUpdate == UpdateStatus::allOkay) {
                noChangeIter = 0;
            } else if (paramUpdate == UpdateStatus::outOfBounds){
               fitResult.chi2PerMeasurement.resize(fitResult.calibMeasurements.size(), -1.); 
               return fitResult;
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
            hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
            hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
            blockCovariance<3>(std::move(hessian),/* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
            fitResult.segmentParErrs.col(2).swap(fitResult.segmentParErrs.col(toInt(ParamDefs::time)));
            fitResult.segmentParErrs.row(2).swap(fitResult.segmentParErrs.row(toInt(ParamDefs::time)));
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
                ATH_MSG_VERBOSE("Boeser mini hessian ("<<miniHessian.determinant()<<")\n"<<miniHessian<<"\n\n"<<hessian);
                return;
            }
            ATH_MSG_VERBOSE("Hessian matrix: \n"<<hessian<<",\nblock Hessian:\n"<<miniHessian<<",\n determinant: "<<miniHessian.determinant());
            covariance.block<nDim,nDim>(0,0) = miniHessian.inverse();
            ATH_MSG_VERBOSE("covariance: \n"<<covariance);
    }

    template <unsigned int nDim> 
        MdtSegmentFitter::UpdateStatus
            MdtSegmentFitter::updateParameters(Parameters& currPars, Parameters& prevPars,
                                               Parameters& currGrad, Parameters& prevGrad,
                                               const AmgSymMatrix(5)& currentHessian) const {
            
            AmgSymMatrix(nDim) miniHessian = currentHessian.block<nDim, nDim>(0,0);
            ATH_MSG_VERBOSE("Parameter update -- \ncurrenPars: "<<toString(currPars)<<", \ngradient: "<<toString(currGrad)
                          <<", hessian ("<<miniHessian.determinant()<<")"<<std::endl<<miniHessian);

            double changeMag{0.};
            if (std::abs(miniHessian.determinant()) > detCutOff) {
                prevPars.block<nDim,1>(0,0) = currPars.block<nDim,1>(0,0);
                // Update the parameters accrodingly to the hessian
                const AmgVector(nDim) updateMe =  miniHessian.inverse()* currGrad.block<nDim, 1>(0,0);
                changeMag = std::sqrt(updateMe.dot(updateMe));
                currPars.block<nDim,1>(0,0) -= updateMe;
                prevGrad.block<nDim,1>(0,0)  = currGrad.block<nDim,1>(0,0);
                ATH_MSG_VERBOSE("Hessian inverse:\n"<<miniHessian.inverse()<<"\n\nUpdate the parameters by -"
                             <<Amg::toString(updateMe));
            } else {
                const AmgVector(nDim) gradDiff = (currGrad - prevGrad).block<nDim,1>(0,0);
                const double gradDiffMag = gradDiff.mag2();
                double denom = (gradDiffMag > std::numeric_limits<float>::epsilon() ? gradDiffMag : 1.); 
                const double gamma = std::abs((currPars - prevPars).block<nDim,1>(0,0).dot(gradDiff))
                                   / denom;
                ATH_MSG_VERBOSE("Hessian determinant invalid. Try deepest descent - \nprev parameters: "
                             <<toString(prevPars)<<",\nprevious gradient: "<<toString(prevGrad)<<", gamma: "<<gamma);
                prevPars.block<nDim, 1>(0,0) = currPars.block<nDim, 1>(0,0);
                currPars.block<nDim, 1>(0,0) -= gamma* currGrad.block<nDim, 1>(0,0);
                prevGrad.block<nDim, 1>(0,0) = currGrad.block<nDim,1>(0,0);
                changeMag = std::abs(gamma) *  currGrad.block<nDim, 1>(0,0).mag(); 
            }
            /// Check that all parameters remain within the parameter boundary window
            unsigned int nOutOfBound{0};
            for (unsigned int p =0; p< nDim; ++p) {
                if (m_cfg.ranges[p][0] > currPars[p] || m_cfg.ranges[p][1]< currPars[p]) {
                    ATH_MSG_WARNING("The "<<p<<"-th parameter "<<toString(static_cast<ParamDefs>(p))<<" is out of range "<<currPars[p]
                                    <<"["<<m_cfg.ranges[p][0]<<"-"<<m_cfg.ranges[p][1]<<"]");
                    ++nOutOfBound;
                }
                currPars[p] = std::clamp(currPars[p], m_cfg.ranges[p][0], m_cfg.ranges[p][1]);
            }
            if (nOutOfBound > m_cfg.nParsOutOfBounds){
                return UpdateStatus::outOfBounds;
            }
            if (changeMag <= m_cfg.tolerance) {
                return UpdateStatus::noChange;
            }
            return UpdateStatus::allOkay;
        }
}
