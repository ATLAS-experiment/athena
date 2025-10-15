/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Compile this file assuming that FP operations may trap.
// Prevents spurious FPEs in the clang build.
#include <CxxUtils/trapping_fp.h>
CXXUTILS_TRAPPING_FP;

#include <MuonPatternHelpers/SegmentLineFitter.h>
#include <MuonPatternEvent/SegmentFitterEventData.h>

#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>

#include <ActsInterop/Logger.h>
#include <ActsInterop/UnitConverters.h>
#include <ActsCalibBase/CalibrationContext.h>


#include <format>

#include <xAODMuonPrepData/sTgcMeasurement.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>


namespace MuonR4::SegmentFit{
    using namespace Acts;
    using namespace Acts::UnitLiterals;

    using Hit_t = SegmentLineFitter::Hit_t;
    using HitVec_t = SegmentLineFitter::HitVec_t;
    using Result_t = SegmentLineFitter::Result_t;

    namespace {
        /** @brief Returns whether the hit suitable for to be used in the fit
         *  @param hit: Reference to the calibrated space point of interest */
        bool isGoodHit(const MuonR4::CalibratedSpacePoint& hit) {
            using enum MuonR4::CalibratedSpacePoint::State;
            return hit.fitState() == Valid;
        }
        /** @brief Returns whether the hit is a muon precision hit */
        bool isPrecisionHit(const MuonR4::CalibratedSpacePoint& hit) {
            using enum xAOD::UncalibMeasType;
            return isGoodHit(hit) && (
                /// Valid Mdt or micromegas are always precision hits
                hit.type() == MdtDriftCircleType || hit.type() == MMClusterType ||
                /// Only consider the stgc strips a precision measurement
                (hit.type() == sTgcStripType && 
                static_cast<const xAOD::sTgcMeasurement*>(hit.spacePoint()->primaryMeasurement())->channelType() ==
                sTgcIdHelper::sTgcChannelTypes::Strip)
            );
        }
        /** @brief Counts the number of precision hits
         *  @param hits: Collection of hit participating in the segment fit */
        inline unsigned countPrecHits(const HitVec_t& hits) {
            return std::ranges::count_if(hits, [](const Hit_t& hit){
                    return isPrecisionHit(*hit);
            });
        }
        /** @brief Counts the number of hits measuring phi */
        inline unsigned countPhiHits(const HitVec_t& hits) {
            return std::ranges::count_if(hits, [](const Hit_t& hit){
                return isGoodHit(*hit) && hit->measuresPhi();
            });
        }
         /** @brief Removes the beamspot measurement from the collection of hits
          *         to avoid that the multiple beam spot constraints are put on the line
          *  @param hits: List of measuremetns from which the beamspot shall be removed*/
        inline void removeBeamSpot(HitVec_t& hits){
            hits.erase(std::remove_if(hits.begin(), hits.end(),
                        [](const Hit_t& a){
                            return a->type() == xAOD::UncalibMeasType::Other;
                        }), hits.end());
        }
        /** @brief Copy the hit vector */
        HitVec_t copy(const HitVec_t& hits) {
            HitVec_t copied{};
            copied.reserve(hits.size());
            std::ranges::transform(hits, std::back_inserter(copied), 
                                    [](const Hit_t& hit) { 
                                        return std::make_unique<CalibratedSpacePoint>(*hit);
                                    });
            return copied;
        }
        Result_t copy(const Result_t& toCopy) {
            Result_t toRet{};
            using FitPars_t = SegmentLineFitter::FitPars_t;
            static_cast<FitPars_t&>(toRet) = toCopy;
            toRet.measurements = copy(toCopy.measurements);
            return toRet;
        }
    }
 
     SegmentLineFitter::Config::RangeArray 
        SegmentLineFitter::Config::defaultRanges() {
            RangeArray rng{};
            constexpr double spatRang = 10._m;
            constexpr double timeTange = 25._ns;
            using enum ParamDefs;
            rng[toUnderlying(y0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(x0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(phi)] = std::array{-179._degree, 179._degree};
            rng[toUnderlying(theta)] = std::array{-85._degree,  85._degree};
            rng[toUnderlying(t0)] = std::array{-timeTange, timeTange};
            return rng;
    }
    SegmentLineFitter::SegmentLineFitter(const std::string& name, Config&& config):
        AthMessaging{name},
        m_fitter{config, makeActsAthenaLogger(this, name)},
        m_cfg{config} {
        m_goodHitSel.connect<isGoodHit>();
    }
    Result_t SegmentLineFitter::callLineFit(const Acts::CalibrationContext& cctx,
                                            const Parameters& startPars,
                                            const Amg::Transform3D& localToGlobal,
                                            HitVec_t&& calibHits) const {
        /// Check whether a beamspot constraint should be appended
        if (m_cfg.doBeamSpot && countPhiHits(calibHits) > 0) {
            const Amg::Transform3D globToLoc{localToGlobal.inverse()};
            Amg::Vector3D beamSpot{globToLoc.translation()};
            Amg::Vector3D beamLine{globToLoc.linear().col(2)};
            SpacePoint::Cov_t covariance{};
            covariance[toUnderlying(AxisDefs::etaCov)] = square(m_cfg.beamSpotRadius);
            covariance[toUnderlying(AxisDefs::phiCov)] = square(m_cfg.beamSpotLength);
            /// placeholder for a very generous beam spot: 300mm in X,Y (tracking volume), 20000 along Z
            auto beamSpotSP = std::make_unique<CalibratedSpacePoint>(nullptr, std::move(beamSpot));
            beamSpotSP->setBeamDirection(std::move(beamLine));
            beamSpotSP->setCovariance(std::move(covariance));
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Beam spot constraint "
                            <<Amg::toString(beamSpotSP->localPosition())<<", "<<beamSpotSP->covariance());
            calibHits.push_back(std::move(beamSpotSP));
        }
        
        if (msgLvl(MSG::VERBOSE)) {
            const auto [pos, dir] = makeLine(startPars);
            std::stringstream hitStream{};
            for (const Hit_t& hit : calibHits) {
                hitStream<<"       **** "<< (*hit)<<", pull: "
                <<std::sqrt(SeedingAux::chi2Term(pos, dir, *hit)) <<std::endl;
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Start segment fit with parameters "
                <<toString(startPars) <<", plane location: "<<Amg::toString(localToGlobal)<<std::endl
                <<hitStream.str());
        }
        FitOpts_t fitOpts{};
        fitOpts.calibContext = cctx;
        fitOpts.calibrator = m_cfg.calibrator;
        fitOpts.selector = m_goodHitSel;
        fitOpts.measurements = std::move(calibHits);
        fitOpts.localToGlobal = localToGlobal;
        fitOpts.startParameters = startPars;
        /// Recall that the time is not the same in Acts & Athena
        fitOpts.startParameters[toUnderlying(ParamDefs::t0)] = ActsTrk::timeToActs(fitOpts.startParameters[toUnderlying(ParamDefs::t0)]);
        /// Fit the measurements
        Result_t result = m_fitter.fit(std::move(fitOpts));
        /// Convert back to athena time units
        result.parameters[toUnderlying(ParamDefs::t0)] = ActsTrk::timeToAthena(result.parameters[toUnderlying(ParamDefs::t0)]);
        return result;
    }
    std::unique_ptr<Segment>
        SegmentLineFitter::fitSegment(const EventContext& ctx,
                                      const SegmentSeed* parent,
                                      const Parameters& startPars,
                                      const Amg::Transform3D& localToGlobal,
                                      HitVec_t&& calibHits) const {
        
        const Acts::CalibrationContext cctx = ActsTrk::getCalibrationContext(ctx);
        if (m_cfg.visionTool) {
            Result_t preFit{};
            preFit.parameters = startPars;
            preFit.measurements = copy(calibHits);
            auto seedCopy = convertToSegment(localToGlobal, parent, std::move(preFit));
            m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Prefit");
        }
        Result_t segFit = callLineFit(cctx, startPars, localToGlobal, std::move(calibHits));
        if (m_cfg.visionTool && segFit.converged) {
            auto seedCopy = convertToSegment(localToGlobal, parent, copy(segFit));
            m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Intermediate fit"); 
        }
        if (!removeOutliers(cctx, *parent, localToGlobal, segFit)) {
            return nullptr;
        }          
        if (!plugHoles(cctx, *parent, localToGlobal, segFit)) {
            return nullptr;
        }
        auto finalSeg = convertToSegment(localToGlobal, parent, std::move(segFit));
        if (m_cfg.visionTool) {
            m_cfg.visionTool->visualizeSegment(ctx, *finalSeg, "Final fit");
        }
        return finalSeg;
    }
    std::unique_ptr<Segment> 
        SegmentLineFitter::convertToSegment(const Amg::Transform3D& locToGlob, 
                                            const SegmentSeed* patternSeed,
                                            Result_t&& data) const {
        const auto [locPos, locDir] = makeLine(data.parameters);
        Amg::Vector3D globPos = locToGlob * locPos;
        Amg::Vector3D globDir = locToGlob.linear()* locDir;

        std::ranges::sort(data.measurements, [](const Hit_t& a, const Hit_t& b){
            return a->localPosition().z() < b->localPosition().z(); 
        });

        auto finalSeg = std::make_unique<Segment>(std::move(globPos), std::move(globDir),
                                                  patternSeed, std::move(data.measurements),
                                                  data.chi2, data.nDoF);
        finalSeg->setCallsToConverge(data.nIter);
        finalSeg->setParUncertainties(std::move(data.covariance));
        /// TODO: Add the config retrieval to the composite space point line fitter
        if (false) {
            finalSeg->setSegmentT0(data.parameters[toUnderlying(ParamDefs::t0)]);
        }
        return finalSeg;
    }

    bool SegmentLineFitter::removeOutliers(const Acts::CalibrationContext& cctx,
                                           const SegmentSeed& seed,
                                           const Amg::Transform3D& localToGlobal,
                                           Result_t& fitResult) const {

        /// @todo Use the fitter config object once it's added to the interface
        if (fitResult.nDoF == 0 || countPrecHits(fitResult.measurements) < m_cfg.nPrecHitCut
            || fitResult.nIter > 10000) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ 
                            <<": No degree of freedom available. What shall be removed?!. nDoF: "
                            <<fitResult.nDoF<<", n-meas: "<<countPrecHits(fitResult.measurements));
            return false;
        }
        if (fitResult.converged && fitResult.chi2 / fitResult.nDoF < m_cfg.outlierRemovalCut) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": The segment "<<toString(fitResult.parameters)
                          <<" is already of good quality "<<fitResult.chi2 /fitResult.nDoF
                          <<". Don't remove outliers");
            return true;
        }
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Segment "
                       <<toString(fitResult.parameters)<<" is of badish quality. Remove worst hit");

        /** Remove a priori the beamspot constaint as it never should pose any problem and
         *  another one will be added anyway in the next iteration */        
        if (m_cfg.doBeamSpot) {
            removeBeamSpot(fitResult.measurements);
        }
        const auto [segPos, segDir] = makeLine(fitResult.parameters);
        /** Next sort the measurements by ascending chi2 */
        std::ranges::sort(fitResult.measurements,
                [&segPos, &segDir](const HitVec_t::value_type& a, const HitVec_t::value_type& b){
                    const double chiSqA = isGoodHit(*a) ? SeedingAux::chi2Term(segPos, segDir, *a) : 0.;
                    const double chiSqB = isGoodHit(*b) ? SeedingAux::chi2Term(segPos, segDir, *b) : 0.;
                    return chiSqA < chiSqB;                   
                });
        fitResult.measurements.back()->setFitState(CalibratedSpacePoint::State::Outlier);

        /** Refit the segment line without the measurement */
        Result_t newAttempt = callLineFit(cctx, fitResult.parameters, 
                                          localToGlobal, std::move(fitResult.measurements));
        if (newAttempt.converged) {
            newAttempt.nIter+=fitResult.nIter;
            fitResult = std::move(newAttempt);
             if (m_cfg.visionTool) {
                const EventContext& ctx{*cctx.get<const EventContext*>()};
                auto seedCopy = convertToSegment(localToGlobal, &seed, copy(fitResult));
                m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Bad fit recovery");
            }
        } else {
            fitResult.nIter+=newAttempt.nIter;
            fitResult.measurements = std::move(newAttempt.measurements);
        }
        return removeOutliers(cctx, seed, localToGlobal, fitResult);
    }

    void SegmentLineFitter::eraseWrongHits(Result_t& candidate) const {
        auto [segPos, segDir] = makeLine(candidate.parameters);
        candidate.measurements.erase(std::remove_if(candidate.measurements.begin(), candidate.measurements.end(),
                [&segPos, &segDir](const HitVec_t::value_type& hit){
                    if (hit->fitState() != CalibratedSpacePoint::State::Outlier) {
                        return false;
                    }
                    /** The segment has never crossed the tube */
                    if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                        const double dist = Amg::lineDistance(segPos, segDir, 
                                                              hit->localPosition(), 
                                                              hit->sensorDirection());
                        const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->spacePoint()->primaryMeasurement());
                        return dist >= dc->readoutElement()->innerTubeRadius();
                    }
                    return false;
                }), candidate.measurements.end());
    }
    bool SegmentLineFitter::plugHoles(const Acts::CalibrationContext& cctx,
                                      const SegmentSeed& seed,
                                      const Amg::Transform3D& localToGlobal,
                                      Result_t& toRecover) const {
        /** We've the first estimator of the segment fit */
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": segment "<<toString(toRecover.parameters)
                        <<", chi2: "<<toRecover.chi2 /std::max(toRecover.nDoF, 1ul)
                        <<", nDoF: "<<toRecover.nDoF);
        /** Setup a map to replace space points if they better suite */
        
        using SpPerLay_t = boost::container::small_vector<const SpacePoint* , 4>;

        std::vector<SpPerLay_t> usedSpacePoints{};
        SpacePointPerLayerSorter laySorter{};
        for (auto& hit : toRecover.measurements) {
            const SpacePoint* sp = hit->spacePoint(); 
            if (!sp) {
                continue;
            }
            const unsigned layNum = laySorter.sectorLayerNum(*sp);
            if (layNum >= usedSpacePoints.size()) {
                usedSpacePoints.resize(layNum + 1);
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Used "<<(*sp)
            <<", layerNumber: "<<layNum);

            usedSpacePoints[layNum].push_back(sp);
        }
        /** */
        const EventContext& ctx{*cctx.get<const EventContext*>()};
        
        const double timeOff = toRecover.parameters[toUnderlying(ParamDefs::t0)];
        HitVec_t candidateHits{};
        bool hasCandidate{false};
        const auto [locPos, locDir] = makeLine(toRecover.parameters);

         /// Loop over all hits in the parent bucket
        for (const auto& hit : *seed.parentBucket()){            
            /// Hit already used in the segment fit
            const unsigned layNum = laySorter.sectorLayerNum(*hit);
         
            if (layNum < usedSpacePoints.size() && 
                std::ranges::any_of(usedSpacePoints[layNum], 
                    [&hit](const SpacePoint* used){   
                        return used == hit.get();
                })) {
                continue;
            }
            std::unique_ptr<CalibratedSpacePoint> calibHit{};
            double pull{-1.};
            if (hit->isStraw()) {
                using namespace Acts::detail::LineHelper;
                const double dist = signedDistance(locPos, locDir, hit->localPosition(), hit->sensorDirection());
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->primaryMeasurement());
                // Check whether the tube is crossed by the hit 
                if (Acts::abs(dist) >= dc->readoutElement()->innerTubeRadius()) {
                    continue;
                }
            } else {
                /// If the hit is a phi measurement check at least if it can be hit by the segment
                if (!hit->measuresEta() && 
                    std::abs(hit->sensorDirection().dot(hit->localPosition() - 
                        SeedingAux::extrapolateToPlane(locPos,locDir, *hit))) >
                        std::sqrt(hit->covariance()[toUnderlying(AxisDefs::etaCov)])){
                    continue;
                }
                /// Use the pull of the uncalibrated measurement to estimate whether 
                ///  a calibration is actually worth
                pull = std::sqrt(SeedingAux::chi2Term(locPos, locDir, *hit));
                if (pull > 1.1 * m_cfg.recoveryPull ||
                    /// There are already hits of the same layer kind in the collection
                    (layNum < usedSpacePoints.size() && 
                     std::ranges::any_of(usedSpacePoints[layNum],[hit](const SpacePoint* used) {
                        return (used->measuresEta() && used->measuresEta() == hit->measuresEta()) ||
                               (used->measuresPhi() && used->measuresPhi() == hit->measuresPhi());
                    })) ) {
                    continue;
                }
            }
            calibHit = m_cfg.calibrator->calibrate(ctx, hit.get(), locPos, locDir, timeOff);
            pull = std::sqrt(SeedingAux::chi2Term(locPos, locDir, *calibHit));
            if (pull <= m_cfg.recoveryPull) {
                hasCandidate |= calibHit->fitState() == CalibratedSpacePoint::State::Valid;
            } else {
                calibHit->setFitState(CalibratedSpacePoint::State::Outlier);
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Candidate hit for recovery "
                    <<seed.msSector()->idHelperSvc()->toString(hit->identify())<<", pull: "<<pull
                    <<"layer number: "<<layNum);
            candidateHits.push_back(std::move(calibHit));                
        }
        /** No extra hit has been found */
        if (!hasCandidate) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": No space point candidates for recovery were found");
            toRecover.measurements.insert(toRecover.measurements.end(), 
                                          std::make_move_iterator(candidateHits.begin()),
                                          std::make_move_iterator(candidateHits.end()));
            eraseWrongHits(toRecover);
            return toRecover.nDoF > 0;
        }


        HitVec_t copied = copy(toRecover.measurements);
        HitVec_t copiedCandidates = copy(candidateHits);
        /// Remove the beamspot constraint measurement
        if (m_cfg.doBeamSpot) {
            removeBeamSpot(copied);
        }

        candidateHits.insert(candidateHits.end(), 
                             std::make_move_iterator(copied.begin()), 
                             std::make_move_iterator(copied.end()));

        Result_t recovered  = callLineFit(cctx, toRecover.parameters, localToGlobal, 
                                          std::move(candidateHits));
        if (!recovered.converged) {
            return false;
        }
        /** Nothing has been recovered. Just bail out */
        if (recovered.nDoF <= toRecover.nDoF) {
            for (HitVec_t::value_type& hit : copiedCandidates) {
                hit->setFitState(CalibratedSpacePoint::State::Outlier);
                toRecover.measurements.push_back(std::move(hit));
            }
            eraseWrongHits(toRecover);
            return true;
        }
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<":  Before - chi2: "<<toRecover.chi2
                      <<", nDoF "<<toRecover.nDoF<<" <=> after recovery - chi2: "
                      <<recovered.chi2<<", nDoF: "<<recovered.nDoF);
        
        double redChi2 = recovered.chi2 / std::max(recovered.nDoF, 1ul);
        /// If the chi2 is less than 5, no outlier rejection is launched. 
        /// So also accept any recovered segment below that threshold
        if (redChi2 < m_cfg.outlierRemovalCut || 
            toRecover.nDoF == 0 || redChi2 < toRecover.chi2 / toRecover.nDoF) {
            ATH_MSG_VERBOSE("Accept segment with recovered "<<(recovered.nDoF  - toRecover.nDoF)<<" hits.");
            recovered.nIter += toRecover.nIter;
            toRecover = std::move(recovered);
            /** Next check whether the recovery made measurements marked as outlier feasable 
             *  the hole recovery*/
            while (true) {
                bool runAnotherTrial = false;
                copied = copy(toRecover.measurements);
                if (m_cfg.doBeamSpot) {
                    removeBeamSpot(copied);
                }
                const auto [beforePos, beforeDir] = makeLine(toRecover.parameters);
                for (HitVec_t::value_type& copyHit : copied) {
                    if (copyHit->fitState() != CalibratedSpacePoint::State::Outlier) {
                        continue;
                    }
                    if (std::sqrt(SeedingAux::chi2Term(beforePos, beforeDir, *copyHit)) < m_cfg.recoveryPull) {
                        copyHit->setFitState(CalibratedSpacePoint::State::Valid);                  
                        runAnotherTrial = true;    
                    } 
                }
                if (!runAnotherTrial) {
                    break;
                }
                recovered = callLineFit(cctx, toRecover.parameters, localToGlobal, std::move(copied));
                if (!recovered.converged) {
                    break;
                }
                if (recovered.nDoF <= toRecover.nDoF) {
                    break;
                }
                redChi2 = recovered.chi2 / std::max(recovered.nDoF, 1ul);
                if (redChi2 <  m_cfg.outlierRemovalCut || redChi2 < toRecover.chi2 / toRecover.nDoF) {
                    recovered.nIter += toRecover.nIter;
                    toRecover = std::move(recovered);
                } else {
                    break;
                }
            }
        } else{
            for (HitVec_t::value_type& hit : copiedCandidates) {
                hit->setFitState(CalibratedSpacePoint::State::Outlier);
                toRecover.measurements.push_back(std::move(hit));
            }
        }
        eraseWrongHits(toRecover);
        return true;
    }

        
    // void SegmentLineFitter::centerAlongWire(SegmentFitResult& fitResult) const {
    //     if (fitResult.nPhiMeas) {
    //         ATH_MSG_VERBOSE("The segment has phi measurements. No centering needed");
    //     }
    //     double avgX{0.};
    //     const double nHits = fitResult.calibMeasurements.size();
    //     std::ranges::for_each(fitResult.calibMeasurements,[&avgX, nHits](const HitType& hit) {
    //         return avgX += hit->localPosition().x() / nHits;
    //     });
    //     fitResult.segmentPars[toUnderlying(ParamDefs::x0)] = avgX;
    // }
         
   
}
