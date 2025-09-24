/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentFitter.h>

#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <TrkEventPrimitives/ParamDefs.h>
#include <EventPrimitives/EventPrimitivesCovarianceHelpers.h>
#include <GaudiKernel/PhysicalConstants.h>
#include <ActsInterop/LoggerUtils.h>


#include <Acts/Seeding/detail/FastStrawLineFitter.hpp>
#include <format>

namespace {
    /* Cut off value for the determinant. Hessian matrices with a determinant smaller than this 
       are considered to be invalid */
    constexpr double detCutOff = 1.e-8;
}

namespace MuonR4::SegmentFit{
    using namespace Acts;
    using namespace Acts::UnitLiterals;
    using FastFitter_t = Acts::Experimental::detail::FastStrawLineFitter;
    /** @brief Copy the indices from the upper triangle to the lower triangle
     *  @param indices: List of parameter indices to consider
     *  @param chi2Obj: Refrence to the chi2 object carrying the Hessian */
    inline void symmetrizeHessian(const std::vector<ParamDefs>& indices,
                                  SeedingAux::ChiSqWithDerivatives& chi2Obj) {
        for (const auto P : indices) {
            for (const auto P1 : indices) {
                if (P1 >= P) {
                    break;
                }
                chi2Obj.hessian(toUnderlying(P1), toUnderlying(P)) = 
                chi2Obj.hessian(toUnderlying(P), toUnderlying(P1));
            }
        }
    }
    /** @brief */
    std::string print(const SeedingAux& aux) {
        std::stringstream sstr{};
        sstr<<"residual: "<<Amg::toString(aux.residual());
        sstr<<" -- gradient:\n";
        for (const auto par : {ParamDefs::x0, ParamDefs::y0, ParamDefs::phi, ParamDefs::theta}) {
            sstr<<"        "<<SeedingAux::parName(par)<<": "<<Amg::toString(aux.gradient(par))<<"\n";
        }
        return sstr.str();
    }
    using HitType = SegmentFitResult::HitType;
    using HitVec = SegmentFitResult::HitVec;
 
     MdtSegmentFitter::Config::RangeArray 
        MdtSegmentFitter::Config::defaultRanges() {
            RangeArray rng{};
            constexpr double spatRang = 10._m;
            constexpr double timeTange = 25 * Gaudi::Units::ns;
            rng[toUnderlying(ParamDefs::y0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(ParamDefs::x0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(ParamDefs::phi)] = std::array{-179._degree, 179._degree};
            rng[toUnderlying(ParamDefs::theta)] = std::array{-85._degree,  85._degree};
            rng[toUnderlying(ParamDefs::t0)] = std::array{-timeTange, timeTange};
            return rng;
    }
    MdtSegmentFitter::MdtSegmentFitter(const std::string& name, Config&& config):
        AthMessaging{name},
        m_cfg{std::move(config)}{}

    inline void MdtSegmentFitter::updateDriftSigns(const Line_t& segmentLine, 
                                                   SegmentFitResult& fitResult) const {
        for (std::unique_ptr<CalibratedSpacePoint>& meas : fitResult.calibMeasurements) {
            meas->setDriftRadius(SeedingAux::strawSign(segmentLine, *meas)*meas->driftRadius());
        }
    }

    inline bool MdtSegmentFitter::recalibrate(const EventContext& ctx,
                                              const Line_t& segmentLine,
                                              SegmentFitResult& fitResult) const{

        fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), 
                                                                  segmentLine.position(), segmentLine.direction(),
                                                                  fitResult.segmentPars[toUnderlying(ParamDefs::t0)]);
        if (!updateHitSummary(fitResult)){
            return false;
        }
        updateDriftSigns(segmentLine, fitResult);
        /// Switch off the time fit if too little degrees of freedom are left
        if (fitResult.timeFit && fitResult.nDoF <= 1) {
            fitResult.timeFit = false;
            ATH_MSG_DEBUG("Switch of the time fit because nDoF: "<<fitResult.nDoF);
            fitResult.segmentPars[toUnderlying(ParamDefs::t0)] = 0.;
            /// Recalibrate the measurements
            fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), 
                                                                     segmentLine.position(), segmentLine.direction(), 0.);
        ///
        } else if (!fitResult.timeFit && m_cfg.doTimeFit) {
            ATH_MSG_DEBUG("Somehow a measurement is on the narrow ridge of validity. Let's try if the time can be fitted now ");
            fitResult.timeFit = true;
        }

        return true;
    }
    inline bool MdtSegmentFitter::updateHitSummary(SegmentFitResult& fitResult) const {
        /// Count the phi & time measurements measurements
         using State = CalibratedSpacePoint::State;
        fitResult.nPhiMeas = fitResult.nDoF = fitResult.nTimeMeas = 0;
        for (const HitType& hit : fitResult.calibMeasurements) {
            if (hit->fitState() != State::Valid){
                continue;
            }

            fitResult.nPhiMeas+= hit->measuresPhi();
            fitResult.nDoF+= hit->measuresPhi();
            fitResult.nDoF+= hit->measuresEta();
            fitResult.nPrecMeas+= (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType);
            /// Mdts are already counted in the measures eta category. Don't count them twice
            fitResult.nDoF += (m_cfg.doTimeFit && hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType && hit->hasTime());
            fitResult.nTimeMeas+=hit->hasTime();              
        }
        if (!fitResult.nDoF) {
            ATH_MSG_VERBOSE("Measurements rejected.");
            return false;
        }
        if (!fitResult.nPhiMeas) {
            ATH_MSG_VERBOSE("No phi measurements are left.");
            fitResult.segmentPars[toUnderlying(ParamDefs::phi)] = 90. * Gaudi::Units::deg; 
        }

        fitResult.nDoF = fitResult.nDoF - 2 - (fitResult.nPhiMeas > 0 ? 2 : 0);

        return true;
    }
    void MdtSegmentFitter::centerAlongWire(SegmentFitResult& fitResult) const {
        if (fitResult.nPhiMeas) {
            ATH_MSG_VERBOSE("The segment has phi measurements. No centering needed");
        }
        double avgX{0.};
        const double nHits = fitResult.calibMeasurements.size();
        std::ranges::for_each(fitResult.calibMeasurements,[&avgX, nHits](const HitType& hit) {
            return avgX += hit->localPosition().x() / nHits;
        });
        fitResult.segmentPars[toUnderlying(ParamDefs::x0)] = avgX;
    }

    SegmentFitResult MdtSegmentFitter::fitSegment(const EventContext& ctx,
                                                  HitVec&& calibHits,
                                                  const Parameters& startPars,
                                                  const Amg::Transform3D& localToGlobal) const {

        using State = CalibratedSpacePoint::State;

        Line_t segmentLine{spatialLinePars(startPars)};

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream hitStream{};
            for (const HitType& hit : calibHits) {
                hitStream<<"       **** "<< (*hit) <<std::endl;
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Start segment fit with parameters "
                <<toString(startPars) <<", plane location: "<<Amg::toString(localToGlobal)<<std::endl<<hitStream.str());
        }

        SegmentFitResult fitResult{};
        fitResult.segmentPars = startPars;
        fitResult.timeFit = m_cfg.doTimeFit;
        fitResult.calibMeasurements = std::move(calibHits);
        if (!updateHitSummary(fitResult)) {
            ATH_MSG_WARNING(__func__<<"() - "<<__LINE__ <<": No valid segment seed parsed from the beginning.");
            return fitResult;
        }
        centerAlongWire(fitResult);

        SeedingAux::Config pullCfg{};
        pullCfg.localToGlobal = localToGlobal;
        pullCfg.useHessian = m_cfg.useSecOrderDeriv;
        pullCfg.calcAlongStrip = false;
        if (fitResult.nPhiMeas == 0u) {
            pullCfg.parsToUse={ParamDefs::y0, ParamDefs::theta};

            if(m_cfg.useFastFit && fitResult.calibMeasurements.size() == fitResult.nPrecMeas) {
                FastFitter_t::Config fastFitCfg{};
                fastFitCfg.maxIter = m_cfg.nMaxCalls;
                fastFitCfg.precCutOff = m_cfg.tolerance;
                FastFitter_t fastFitter{fastFitCfg,
                                        Acts::getDefaultLogger("MdtSegmentFitter",
                                                               ActsTrk::actsLevelVector(msg().level()))};
                SegmentFitResult::HitVec validHits{};
                validHits.reserve(fitResult.calibMeasurements.size());
                for (const auto& hit : fitResult.calibMeasurements) {
                    if (hit->fitState() == CalibratedSpacePoint::State::Valid) {
                        validHits.emplace_back(std::make_unique<CalibratedSpacePoint>(*hit));
                    }
                }
                auto fastResult = fastFitter.fit(validHits,
                                                 SeedingAux::strawSigns(segmentLine, validHits));
                if (!fastResult) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Fast Mdt fit failed ");
                    return fitResult;
                }
                fitResult.converged = true;
                fitResult.nDoF = (*fastResult).nDoF;
                fitResult.chi2 = (*fastResult).chi2;
                fitResult.nIter = (*fastResult).nIter;
                fitResult.segmentPars[toUnderlying(ParamDefs::phi)] = 90._degree;
                fitResult.segmentPars[toUnderlying(ParamDefs::theta)] = (*fastResult).theta;
                fitResult.segmentPars[toUnderlying(ParamDefs::y0)] = (*fastResult).y0;

                fitResult.segmentParErrs(toUnderlying(ParamDefs::theta),
                                         toUnderlying(ParamDefs::theta)) = Acts::square((*fastResult).dTheta);

                fitResult.segmentParErrs(toUnderlying(ParamDefs::y0),
                                         toUnderlying(ParamDefs::y0)) = Acts::square((*fastResult).dY0);
                return fitResult;
            }
        } 

        bool recalib{m_cfg.reCalibrate};
        if (!recalib) {
            updateDriftSigns(segmentLine, fitResult);
        }
        SeedingAux pullCalculator{pullCfg,
                                    Acts::getDefaultLogger("MdtSegmentFitter",
                                                    ActsTrk::actsLevelVector(msg().level()))};

        SeedingAux::ChiSqWithDerivatives currentChi2{}, prevChi2{};
        Parameters prevPars{AmgVector(5)::Zero()};
   
        unsigned int noChangeIter{0};

        constexpr auto tIdx = toUnderlying(ParamDefs::t0);

        while (!fitResult.converged && fitResult.nIter++ < m_cfg.nMaxCalls) {
            prevChi2 = currentChi2;
            ATH_MSG_DEBUG("Iteration: "<<fitResult.nIter<<" parameters: "<<toString(fitResult.segmentPars)<<", chi2: "<<currentChi2.chi2);
          
            /// First step calibrate the hits
            if (recalib && !recalibrate(ctx, segmentLine, fitResult)) {
                break;
            }
            /// Reset chi2
            currentChi2.reset();
            const double t0 = fitResult.segmentPars[tIdx];
            /** Loop over the hits to calculate the partial derivatives */
            for (HitType& hit : fitResult.calibMeasurements) {
                if (hit->fitState() != State::Valid) {
                    ATH_MSG_VERBOSE("Skip bad measurement");
                    continue;
                }
                ATH_MSG_VERBOSE("Update chi2 from measurement "<<(*hit));

                double driftV{0.}, driftA{0.};
                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType && fitResult.timeFit) {
                   if (!m_cfg.reCalibrate && fitResult.nIter > 1) {
                        /** If the time is fitted the drift radii need to be updated.
                          *  That's properly only possible with recalibration */
                        const double dSign = (hit->driftRadius() > 0 ? 1. : -1.);
                        hit = m_cfg.calibrator->calibrate(ctx, *hit, segmentLine.position(), segmentLine.direction(), t0);
                        hit->setDriftRadius(dSign*hit->driftRadius());
                    }
                    driftV = m_cfg.calibrator->driftVelocity(ctx, *hit);
                    driftA = m_cfg.calibrator->driftAcceleration(ctx, *hit);
                }

                if (fitResult.timeFit) {
                    pullCalculator.updateFullResidual(segmentLine, t0, *hit, driftV, driftA);
                } else {
                    pullCalculator.updateSpatialResidual(segmentLine, *hit);
                }
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Calculated "<<print(pullCalculator));
                pullCalculator.updateChiSq(currentChi2, hit->covariance());
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Updated chi2: "<<currentChi2.chi2
                            <<", gradient: "<<toString(currentChi2.gradient)<<", Hessian:\n"
                                <<currentChi2.hessian);
            }

            /// Check whether the gradient is already sufficiently small
            if (currentChi2.gradient.mag() < m_cfg.tolerance) {
                fitResult.converged = true;
                ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__ <<": Fit converged after "
                                <<fitResult.nIter<<" iterations with "<<fitResult.chi2);
                break;
            }
            symmetrizeHessian(pullCfg.parsToUse, currentChi2);
            ATH_MSG_DEBUG("Total chi2 & derivatives from iteration - chi2: "<<currentChi2.chi2
                          <<", gradient: "<<toString(currentChi2.gradient)<<", Hessian:\n"
                                <<currentChi2.hessian);
            /// Pure eta segment fit
            UpdateStatus paramUpdate{UpdateStatus::outOfBounds};
            if (!fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<2>(fitResult.segmentPars, prevPars, currentChi2.gradient, prevChi2.gradient, 
                                                  currentChi2.hessian); 
            } else if (!fitResult.nPhiMeas && fitResult.timeFit) {
                /// In the case that the time is fit & that there are no phi measurements -> compress matrix by swaping
                /// the time column with whaever the second column is... it's zero
                currentChi2.hessian.col(2).swap(currentChi2.hessian.col(tIdx));
                currentChi2.hessian.row(2).swap(currentChi2.hessian.row(tIdx));
                std::swap(currentChi2.gradient[2], currentChi2.gradient[tIdx]);
                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[tIdx]);
                paramUpdate = updateParameters<3>(fitResult.segmentPars, prevPars, currentChi2.gradient, 
                                                 prevChi2.gradient, currentChi2.hessian); 
           } else if (fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<4>(fitResult.segmentPars, prevPars, currentChi2.gradient, prevChi2.gradient, 
                                                  currentChi2.hessian); 
            } else if (fitResult.nPhiMeas && fitResult.timeFit) {
                paramUpdate = updateParameters<5>(fitResult.segmentPars, prevPars, currentChi2.gradient, prevChi2.gradient, 
                                                  currentChi2.hessian); 
            }
            switch (paramUpdate) {
                case UpdateStatus::noChange: {
                    if ((++noChangeIter) >= m_cfg.noMoveIter) {
                        fitResult.converged = true;
                    }
                    break;
                } case UpdateStatus::allOkay: {
                    noChangeIter = 0;
                    recalib = m_cfg.reCalibrate;
                    break;
                } case UpdateStatus::smallStep: {
                    noChangeIter = 0;
                    recalib = false;
                    break;
                } case UpdateStatus::outOfBounds:{
                    return fitResult;
                }
            }
            /// Finally update the line with the updated parameter
            segmentLine.updateParameters(spatialLinePars(fitResult.segmentPars));
        }
        /// Fit succeeded -> Make the final chores
        
        /// Subtract 1 degree of freedom to take the time into account
        fitResult.nDoF-=fitResult.timeFit;

        /// Sort the measurements by ascending z
        std::ranges::sort(fitResult.calibMeasurements, [](const HitType&a, const HitType& b){
                return a->localPosition().z() < b->localPosition().z();
        });

        /*** Remove the drift sign again */
        fitResult.chi2 =currentChi2.chi2;
        for (const HitType& hit : fitResult.calibMeasurements) {
            hit->setDriftRadius(std::abs(hit->driftRadius()));
        }
        /// Calculate the covariance of the fit
        if (!fitResult.nPhiMeas&& !fitResult.timeFit) {
            blockCovariance<2>(std::move(currentChi2.hessian), fitResult.segmentParErrs);
        } else if (!fitResult.nPhiMeas && fitResult.timeFit) {
            currentChi2.hessian.col(2).swap(currentChi2.hessian.col(tIdx));
            currentChi2.hessian.row(2).swap(currentChi2.hessian.row(tIdx));
            blockCovariance<3>(std::move(currentChi2.hessian), fitResult.segmentParErrs);
            fitResult.segmentParErrs.col(2).swap(fitResult.segmentParErrs.col(tIdx));
            fitResult.segmentParErrs.row(2).swap(fitResult.segmentParErrs.row(tIdx));
        } else if (fitResult.nPhiMeas) {
            blockCovariance<4>(std::move(currentChi2.hessian), fitResult.segmentParErrs);
        } else if (fitResult.nPhiMeas && fitResult.timeFit) {
            blockCovariance<5>(std::move(currentChi2.hessian), fitResult.segmentParErrs);
        }
        return fitResult;    
    }
    template <unsigned int nDim>
        void MdtSegmentFitter::blockCovariance(const AmgSymMatrix(5)& hessian,
                                               SegmentFit::Covariance& covariance) const {

            covariance.setIdentity();
            AmgSymMatrix(nDim) miniHessian = hessian.block<nDim, nDim>(0,0);
            if (std::abs(miniHessian.determinant()) <= detCutOff) {
                ATH_MSG_VERBOSE("Boeser mini hessian ("<<miniHessian.determinant()<<")\n"<<miniHessian
                              <<"\n\n"<<hessian);
                return;
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Hessian matrix: \n"<<hessian
                <<",\nblock Hessian:\n"<<miniHessian<<",\n determinant: "<<miniHessian.determinant());
            covariance.block<nDim,nDim>(0,0) = miniHessian.inverse().eval();
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": covariance: \n"<<covariance);
    }

    template <unsigned int nDim> 
        MdtSegmentFitter::UpdateStatus
            MdtSegmentFitter::updateParameters(Parameters& currPars, Parameters& prevPars,
                                               Parameters& currGrad, Parameters& prevGrad,
                                               const AmgSymMatrix(5)& currentHessian) const {
            
            AmgSymMatrix(nDim) miniHessian = currentHessian.block<nDim, nDim>(0, 0);
            AmgVector(nDim) miniPars = currPars.block<nDim, 1>(0, 0);
            const AmgVector(nDim) miniGrad = currGrad.block<nDim, 1>(0,0);
            const double determinant = miniHessian.determinant();
            ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__<<": Parameter update -- \ncurrenPars: "
                          <<toString(currPars)<<", \ngradient: "<<toString(currGrad)
                          <<", hessian ("<<determinant<<")"<<std::endl<<miniHessian);

            double angleUpdate{0.}, iceptUpdate{0.};
            auto updateMag = [&angleUpdate, &iceptUpdate](const AmgVector(nDim)& updateMe) {
                if constexpr( nDim == 4) {
                    angleUpdate = Acts::fastHypot( updateMe[toUnderlying(ParamDefs::theta)],
                                                   updateMe[toUnderlying(ParamDefs::phi)]);

                    iceptUpdate = Acts::fastHypot( updateMe[toUnderlying(ParamDefs::x0)],
                                                   updateMe[toUnderlying(ParamDefs::y0)]);
                } else{
                    angleUpdate = Acts::abs( updateMe[toUnderlying(ParamDefs::theta)]);
                    iceptUpdate = Acts::abs( updateMe[toUnderlying(ParamDefs::y0)]);
                }

            };

            bool validHessian{determinant > detCutOff};

            if (ATH_UNLIKELY(validHessian && determinant < 1.)) {
                Eigen::FullPivLU<AmgSymMatrix(nDim)> mFullPivLU(miniHessian);
                validHessian = mFullPivLU.isInvertible();
                if (validHessian){
                    miniHessian = mFullPivLU.inverse();
                }  
            } else if (validHessian) {
                miniHessian = miniHessian.inverse().eval();
            }
            if (validHessian) {
                prevPars.block<nDim,1>(0,0) = currPars.block<nDim,1>(0,0);
                // Update the parameters accrodingly to the hessian

                const AmgVector(nDim) updateMe =  miniHessian* miniGrad;
                miniPars -= updateMe;
                prevGrad.block<nDim,1>(0,0) = miniGrad;
                for (unsigned p = 0; p < nDim; ++p) {
                    /// Recall that for 3x3 the dimensions are [y0, theta, time]
                    if constexpr(nDim == 3) {
                        if (p ==2) {
                            currPars[toUnderlying(ParamDefs::t0)] = miniPars[p];
                        } else {
                            currPars[p] = miniPars[p];
                        }
                    } else {
                        currPars[p] = miniPars[p];
                    }
                }
                updateMag(updateMe);
    
                ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__<<": Hessian inverse:\n"<<miniHessian
                        <<"\nUpdate the parameters by -"<<Amg::toString(updateMe)
                        <<std::format(", |{:E}|, angular: |{:E}|, spatial: |{:E}|", 
                                        updateMe.norm(), angleUpdate, iceptUpdate )
                        <<" -> new parameters: "<<toString(currPars));
            } else {
                const AmgVector(nDim) gradDiff = (currGrad - prevGrad).block<nDim,1>(0,0);
                const double gradDiffMag = gradDiff.mag2();
                double denom = (gradDiffMag > std::numeric_limits<float>::epsilon() ? gradDiffMag : 1.); 
                const double gamma = std::abs((currPars - prevPars).block<nDim,1>(0,0).dot(gradDiff)) / denom;
                ATH_MSG_VERBOSE("Hessian determinant invalid. Try deepest descent - \nprev parameters: "
                             <<toString(prevPars)<<",\nprevious gradient: "<<toString(prevGrad)<<", gamma: "<<gamma);
                prevPars.block<nDim, 1>(0,0) = currPars.block<nDim, 1>(0,0);
                currPars.block<nDim, 1>(0,0) -= gamma* currGrad.block<nDim, 1>(0,0);
                prevGrad.block<nDim, 1>(0,0) = currGrad.block<nDim,1>(0,0);
                updateMag(std::abs(gamma) *  currGrad.block<nDim, 1>(0,0));
            }
            /// Check that all parameters remain within the parameter boundary window
            unsigned int nOutOfBound{0};
            for (unsigned int p = 0; p< nDim; ++p) {
                double& parValue{currPars[p]};
                if (m_cfg.ranges[p][0] > parValue || m_cfg.ranges[p][1] < parValue) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": The "<<p<<"-th parameter "
                                    <<toString(static_cast<ParamDefs>(p))<<" is out of range "<<parValue
                                    <<" allowed ["<<m_cfg.ranges[p][0]<<"-"<<m_cfg.ranges[p][1]<<"]");
                    ++nOutOfBound;
                    parValue = std::clamp(parValue, m_cfg.ranges[p][0], m_cfg.ranges[p][1]);
                }

            }
            if (nOutOfBound > m_cfg.nParsOutOfBounds) {
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Too many parameters went out of bounds");
                return UpdateStatus::outOfBounds;
            }
            if (Acts::fastHypot(angleUpdate, iceptUpdate) <= m_cfg.tolerance) {
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Parameters did not move");
                return UpdateStatus::noChange;
            }
            if (angleUpdate < m_cfg.angularCalibCutOff && iceptUpdate < m_cfg.spatialCalibCuttOff) {
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Parameters are precise enough to switch off calib");
                return UpdateStatus::smallStep;
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Parameter update good");
            return UpdateStatus::allOkay;
        }
}
