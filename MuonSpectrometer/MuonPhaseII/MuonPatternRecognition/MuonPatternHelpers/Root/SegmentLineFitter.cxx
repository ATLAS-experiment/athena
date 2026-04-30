/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Compile this file assuming that FP operations may trap.
// Prevents spurious FPEs in the clang build.
#include <CxxUtils/trapping_fp.h>
CXXUTILS_TRAPPING_FP;

#include <MuonPatternHelpers/SegmentLineFitter.h>
#include <MuonPatternEvent/SegmentFitterEventData.h>

#include <MuonSpacePoint/SpacePointHelpers.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>

#include <ActsInterop/Logger.h>
#include <ActsInterop/UnitConverters.h>
#include <ActsCalibBase/CalibrationContext.h>


#include <format>

#include <xAODMuonPrepData/sTgcMeasurement.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <xAODMuonPrepData/MMCluster.h>


namespace MuonR4::SegmentFit{
    using namespace Acts;
    using namespace Acts::UnitLiterals;

    using Hit_t = SegmentLineFitter::Hit_t;
    using HitVec_t = SegmentLineFitter::HitVec_t;
    using Result_t = SegmentLineFitter::Result_t;

    namespace {

        constexpr double calcRedChi2(const Result_t& result) {
            return result.nDoF > 0ul ? result.chi2 / result.nDoF : result.chi2;
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
            constexpr double timeTange = 50._ns;
            using enum ParamDefs;
            rng[toUnderlying(y0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(x0)] = std::array{-spatRang, spatRang};
            rng[toUnderlying(phi)] = std::array{-179._degree, 179._degree};
            rng[toUnderlying(theta)] = std::array{0._degree,  175._degree};
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
        bool appendsBS = m_cfg.doBeamSpot && countPhiHits(calibHits) > 0;
        
 
        Result_t result{};
        //check the degrees of freedom before try the fit
        if (const std::size_t nPars = m_fitter.config().parsToUse.size(); nPars > 0ul) { 
            auto dOF = m_fitter.countDoF(calibHits, m_goodHitSel);     
            if (dOF.bending + dOF.nonBending < nPars) {
                return result;
            }
            // check that there are at least two crossing stereo measurements
            if (dOF.nonBending == 0ul && nPars == 4ul){
                bool foundU{false}, foundV{false};
                for (const HitVec_t::value_type& hit : calibHits) {
                    if (hit->type() != xAOD::UncalibMeasType::MMClusterType || !isGoodHit(*hit)) {
                        continue;
                    }
                    const auto* mmClust = dynamic_cast<const xAOD::MMCluster*>(hit->spacePoint()->primaryMeasurement());
                    assert(mmClust != nullptr);
                    const auto& design = mmClust->readoutElement()->stripLayer(mmClust->layerHash()).design();
                    if (!design.hasStereoAngle()) {
                        continue;
                    }
                    if (design.stereoAngle() > 0.) {
                        foundU = true;
                    } else {
                        foundV = true;
                    }
                    if (foundU && foundV) {
                        break;
                    }
                }
                if (!foundU || !foundV) {
                    result.measurements = std::move(calibHits);
                    result.parameters = startPars;
                    return result;
                }
                if (m_cfg.doBeamSpot) {
                    appendsBS = true;
                }
            } 
        }
        if (appendsBS) {
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
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Start segment fit with parameters "
                <<toString(startPars) <<", plane location: "<<Amg::toString(localToGlobal)
                <<std::endl<<print(calibHits));

        FitOpts_t fitOpts{};
        fitOpts.calibContext = cctx;
        fitOpts.calibrator = m_cfg.calibrator;
        fitOpts.selector = m_goodHitSel;

        fitOpts.measurements = std::move(calibHits);
        fitOpts.localToGlobal = localToGlobal;
        fitOpts.startParameters = startPars;
        /// Recall that the time is not the same in Acts & Athena
        constexpr auto t0idx = toUnderlying(ParamDefs::t0);
        fitOpts.startParameters[t0idx] = ActsTrk::timeToActs(fitOpts.startParameters[t0idx]);
        /// Fit the measurements
        result = m_fitter.fit(std::move(fitOpts));
        /// Convert back to athena time units
        if (m_fitter.config().fitT0) {
            result.parameters[t0idx] = ActsTrk::timeToAthena(result.parameters[t0idx]);
            result.covariance(t0idx, t0idx) = Acts::square(ActsTrk::timeToAthena(1.)) * result.covariance(t0idx, t0idx);
            for (ParamDefs p : {ParamDefs::x0, ParamDefs::y0, ParamDefs::phi, ParamDefs::theta}) {
                auto pidx = toUnderlying(p);
                result.covariance(t0idx, pidx) = ActsTrk::timeToAthena(result.covariance(t0idx, pidx));
                result.covariance(pidx, t0idx) = ActsTrk::timeToAthena(result.covariance(pidx, t0idx));
            }
        }
        /// Cache the chi2 terms of the measurements w.r.t. the segment
        {
            const auto[segPos, segDir] = makeLine(result.parameters);
            for (Hit_t& hit : result.measurements) {
                hit->setChi2Term(SeedingAux::chi2Term(segPos, segDir, *hit));
            }
        }
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
        if (!removeOutliers(cctx, *parent, localToGlobal,
                            segFit.converged? segFit.parameters : startPars,
                            segFit)) {
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
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Create new segment "
                        <<toString(data.parameters)<<" in "<<patternSeed->msSector()->identString()
                        <<"built from:\n"<<print(data.measurements));

        auto finalSeg = std::make_unique<Segment>(std::move(globPos), std::move(globDir),
                                                  patternSeed, std::move(data.measurements),
                                                  data.chi2, data.nDoF);
        finalSeg->setCallsToConverge(data.nIter);
        finalSeg->setParUncertainties(std::move(data.covariance));
        if (m_fitter.config().fitT0) {
            finalSeg->setSegmentT0(data.parameters[toUnderlying(ParamDefs::t0)]);
        }
        return finalSeg;
    }

    bool SegmentLineFitter::removeOutliers(const Acts::CalibrationContext& cctx,
                                           const SegmentSeed& seed,
                                           const Amg::Transform3D& localToGlobal,
                                           const LinePar_t& startPars,
                                           Result_t& fitResult) const {
      

        if (countPrecHits(fitResult.measurements) < m_cfg.nPrecHitCut || fitResult.nDoF == 0
            || fitResult.nIter > m_fitter.config().maxIter) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ 
                            <<": No degree of freedom available. What shall be removed?!. nDoF: "
                            <<fitResult.nDoF<<", n-meas: "<<countPrecHits(fitResult.measurements)
                            <<std::endl<<print(fitResult.measurements));
            return false;
        }
        if (fitResult.converged && calcRedChi2(fitResult) < m_cfg.outlierRemovalCut) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": The segment "<<toString(fitResult.parameters)
                          <<" is already of good quality "<< calcRedChi2(fitResult)<<". Don't remove outliers");
            return true;
        }
        if (fitResult.nDoF == 0u){
            return false;
        }
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Segment "
                       <<toString(fitResult.parameters)<<", nIter: "<<fitResult.nIter
                       <<" is of badish quality. "<<print(fitResult.measurements)
                       <<std::endl<<"Remove worst hit");

        /** Remove a priori the beamspot constaint as it never should pose any problem and
         *  another one will be added anyway in the next iteration */        
        if (m_cfg.doBeamSpot) {
            removeBeamSpot(fitResult.measurements);
        }

        /** Next sort the measurements by ascending chi2 */
        std::ranges::sort(fitResult.measurements,
                [](const HitVec_t::value_type& a, const HitVec_t::value_type& b){
                    /// Move the outliers to the front
                    if (isGoodHit(*a) != isGoodHit(*b)) {
                        return !isGoodHit(*a);
                    }
                    return a->chi2Term() < b->chi2Term();                   
                });
        fitResult.measurements.back()->setFitState(HitState::Outlier);
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Mark "<<(*fitResult.measurements.back())<<" as outlier");

        /** Refit the segment line without the measurement */
        Result_t newAttempt = callLineFit(cctx, startPars, localToGlobal, 
                                          std::move(fitResult.measurements));
        if (newAttempt.converged) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" The outlier removal converged.");
            newAttempt.nIter+=fitResult.nIter;
            fitResult = std::move(newAttempt);
             if (m_cfg.visionTool) {
                const EventContext& ctx{*cctx.get<const EventContext*>()};
                auto seedCopy = convertToSegment(localToGlobal, &seed, copy(fitResult));
                m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Bad fit recovery");
            }
        } else {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__
                <<" Outlier removal fit did not converge. Needed iterations: "<<newAttempt.nIter);
            if (newAttempt.nIter == 0ul) {
                return false;
            }
            fitResult.nIter+=newAttempt.nIter;
            fitResult.measurements = std::move(newAttempt.measurements);
        }
        return removeOutliers(cctx, seed, localToGlobal,
                              fitResult.converged ? fitResult.parameters : startPars, 
                              fitResult);
    }

    void SegmentLineFitter::eraseWrongHits(Result_t& candidate) const {
        auto [segPos, segDir] = makeLine(candidate.parameters); 
        cleanStripLayers(candidate.measurements);
        candidate.measurements.erase(std::remove_if(candidate.measurements.begin(), 
                                                    candidate.measurements.end(),
            [&](const HitVec_t::value_type& hit){
                if (hit->fitState() == HitState::Valid) {
                    return false;
                } else if (hit->fitState() == HitState::Duplicate) {
                    return true;
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
    inline void SegmentLineFitter::cleanStripLayers(HitVec_t& hits) const {
        const SpacePointPerLayerSorter sorter{};
        /// We need to sort out strip hits on the same layer
        std::ranges::sort(hits, [&](const Hit_t&a ,const Hit_t& b){
            if (a->isStraw() || b->isStraw()) {
                return !a->isStraw();
            }
            if (a->type() == xAOD::UncalibMeasType::Other || 
                b->type() == xAOD::UncalibMeasType::Other) {
                return a->type() != xAOD::UncalibMeasType::Other;
            }
            const unsigned lay_a = sorter.sectorLayerNum(*a->spacePoint());
            const unsigned lay_b = sorter.sectorLayerNum(*b->spacePoint());
            if (lay_a != lay_b) {
                return lay_a < lay_b;
            }
            const double chi2a = a->chi2Term();
            const double chi2b = b->chi2Term();
            /* Do not accept pad hits even though they've smaller chi2
             * than the neighbouring strip */
            if (a->type() == xAOD::UncalibMeasType::sTgcStripType) {
                const auto* sTgcA = static_cast<const xAOD::sTgcMeasurement*>(a->spacePoint()->primaryMeasurement());
                const auto* sTgcB = static_cast<const xAOD::sTgcMeasurement*>(b->spacePoint()->primaryMeasurement());
                if (sTgcA->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Pad &&
                    sTgcB->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Strip) {
                    return chi2b > m_cfg.recoveryPull;
                } else if (sTgcB->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Pad &&
                           sTgcA->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Strip) {
                    return chi2a < m_cfg.recoveryPull;
                }
            }
            return chi2a < chi2b;
        });

        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Check for duplicate strip hits");
        /// Loop over the hits to mark the less compatible hits on the layer as outlier
        for (HitVec_t::iterator itr = hits.begin(); itr != hits.end(); ++itr) {
            const Hit_t& hit_a{*itr};
            if (hit_a->isStraw()){
                break;
            }
            if(hit_a->fitState() == HitState::Duplicate || 
               hit_a->type() == xAOD::UncalibMeasType::Other) {
                continue;
            }
            const unsigned lay_a = sorter.sectorLayerNum(*hit_a->spacePoint());
            ///
            for (HitVec_t::iterator itr2 = itr + 1; itr2 != hits.end(); ++itr2) {
                const Hit_t& hit_b{*itr2};
                if (hit_b->type() == xAOD::UncalibMeasType::Other ||
                    hit_b->fitState() == HitState::Duplicate) {
                    continue;
                }
                if (lay_a != sorter.sectorLayerNum(*hit_b->spacePoint())) {
                    break;
                }
                /// Both hits measure eta. They've been sorted by lower chi2 -> reject b
                if ((hit_a->measuresEta() && hit_b->measuresEta()) ||
                    (hit_a->measuresPhi() && hit_b->measuresPhi())) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Duplicate hit on same layer"<<std::endl
                        <<" -- reject: "<<(*hit_b)<<std::endl
                        <<" -- accept: "<<(*hit_a));
                    hit_b->setFitState(HitState::Duplicate);
                }
            }
        }
    }

    inline bool SegmentLineFitter::betterResult(const Result_t& newResult, const Result_t& oldResult) const {
        if (!newResult.converged) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" The new result did not converge");
            return false;
        }
        const double redChi2New = calcRedChi2(newResult);
        const double redChi2Old = calcRedChi2(oldResult);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Compare results -- old chi2: "<<redChi2Old<<", nDoF: "
                    <<oldResult.nDoF<<" vs. new chi2: "<<redChi2New<<", nDoF: "<<newResult.nDoF
                    <<" -- outlier removal: "<<m_cfg.outlierRemovalCut);
        if (newResult.nDoF == oldResult.nDoF) {
            return redChi2New < redChi2Old;
        }
        return (redChi2New < m_cfg.outlierRemovalCut && newResult.nDoF > oldResult.nDoF) ||
               (redChi2New > m_cfg.outlierRemovalCut && redChi2New < redChi2Old);
    }
    bool SegmentLineFitter::plugHoles(const Acts::CalibrationContext& cctx,
                                      const SegmentSeed& seed,
                                      const Amg::Transform3D& localToGlobal,
                                      Result_t& toRecover) const {
        /** We've the first estimator of the segment fit */
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": segment "<<toString(toRecover.parameters)
                        <<", chi2: "<< calcRedChi2(toRecover) <<", nDoF: "<<toRecover.nDoF);
        /** Setup a map to replace space points if they better suite */
        
 
        std::unordered_set<const SpacePoint*> usedSpacePoints{};
        for (auto& hit : toRecover.measurements) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": "<<(*hit)<<" is known");
            usedSpacePoints.insert(hit->spacePoint());
        }
        /** */
        const EventContext& ctx{*cctx.get<const EventContext*>()};
        
        const double timeOff = toRecover.parameters[toUnderlying(ParamDefs::t0)];
        HitVec_t candidateHits{};
        std::size_t hasCandidate{0};
        const auto [locPos, locDir] = makeLine(toRecover.parameters);

         /// Loop over all hits in the parent bucket
        for (const auto& hit : *seed.parentBucket()){            
            /// Hit already used in the segment fit
            if (usedSpacePoints.count(hit.get())){ 
                continue;
            }
            Hit_t calibHit{};
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
                        1.1*std::sqrt(hit->covariance()[toUnderlying(AxisDefs::etaCov)])){
                    continue;
                }
                /// Use the pull of the uncalibrated measurement to estimate whether 
                ///  a calibration is actually worth
                pull = std::sqrt(SeedingAux::chi2Term(locPos, locDir, *hit));
                if (pull > 1.1 * m_cfg.recoveryPull) {
                    continue;
                }
            }
            calibHit = m_cfg.calibrator->calibrate(ctx, hit.get(), locPos, locDir, ActsTrk::timeToActs(timeOff));
            calibHit->setChi2Term(SeedingAux::chi2Term(locPos, locDir, *calibHit));
            if (calibHit->chi2Term() <= Acts::square(m_cfg.recoveryPull)) {
                hasCandidate += calibHit->fitState() == HitState::Valid;
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Candidate hit for recovery "
                            <<(*calibHit));
            } else {
                calibHit->setFitState(HitState::Outlier);
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Outlier hit "
                            <<(*calibHit)<<" -> limit: "<<m_cfg.recoveryPull);
            }
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
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Found "<<hasCandidate<<" space points for recovery. ");


        HitVec_t copied = copy(toRecover.measurements);
        HitVec_t copiedCandidates = copy(candidateHits);
        /// Remove the beamspot constraint measurement
        if (m_cfg.doBeamSpot) {
            removeBeamSpot(copied);
        }

        candidateHits.insert(candidateHits.end(), 
                             std::make_move_iterator(copied.begin()), 
                             std::make_move_iterator(copied.end()));

        cleanStripLayers(candidateHits);

        Result_t recovered = callLineFit(cctx, toRecover.parameters, localToGlobal, 
                                         std::move(candidateHits));
       
        /// If the chi2 is less than 5, no outlier rejection is launched. 
        /// So also accept any recovered segment below that threshold
        if (betterResult(recovered, toRecover)) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Accept segment with recovered "
                            <<(recovered.nDoF  - toRecover.nDoF)<<" extra nDoF.");
            recovered.nIter += toRecover.nIter;
            toRecover = std::move(recovered);

            std::vector<const CalibratedSpacePoint*> stripOutliers{};
            stripOutliers.reserve(toRecover.measurements.size());
            /** Next check whether the recovery made measurements marked 
             *  as outlier feasable to the hole recovery*/
            unsigned recovLoop{(copiedCandidates.size() != hasCandidate)*m_cfg.nRecoveryLoops};
            while (++recovLoop <= m_cfg.nRecoveryLoops) {   
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Enter recovery loop "<<recovLoop<<".");
                copied = copy(toRecover.measurements);
                // Remove the beamspot
                if (m_cfg.doBeamSpot) {
                    removeBeamSpot(copied);
                }
                // Check whether an outlier can be lifted to on-track
                for (HitVec_t::value_type& copyHit : copied) {
                    if (copyHit->fitState() != HitState::Outlier) {
                        continue;
                    }
                    if (copyHit->chi2Term() < Acts::square(m_cfg.recoveryPull)) {
                        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Try to recover outlier "<<(*copyHit));
                        copyHit->setFitState(HitState::Valid);
                        stripOutliers.push_back(copyHit.get());
                    } 
                }
                // Nothing to recover
                if (stripOutliers.empty()) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": No additional measurement found");
                    break;
                }
                // Ensure that only one hit per layer is fit
                cleanStripLayers(copied);
                // Recovery turned out to be duplicates on the same layer
                if (std::ranges::none_of(stripOutliers,[](const CalibratedSpacePoint* sp){
                        return sp->fitState() == HitState::Valid;
                    })) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Outliers turned out to be duplicates.");
                    break;
                }
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Start fit without the outliers.");
                stripOutliers.clear();
                recovered = callLineFit(cctx, toRecover.parameters, localToGlobal, std::move(copied));
                if (!betterResult(recovered, toRecover)) {
                    break;
                }
                recovered.nIter += toRecover.nIter;
                toRecover = std::move(recovered);
            }
        } else{
            for (HitVec_t::value_type& hit : copiedCandidates) {
                hit->setFitState(HitState::Outlier);
                toRecover.measurements.push_back(std::move(hit));
            }
        }
        eraseWrongHits(toRecover);
        return true;
    }        
   
}
