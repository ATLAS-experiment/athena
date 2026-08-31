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
#include <xAODMuonPrepData/UtilFunctions.h>


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
        inline HitVec_t copyAndSort(HitVec_t hits) {
            std::ranges::sort(hits,  [](const Hit_t& a, const Hit_t& b){
                return a->localPosition().z() < b->localPosition().z(); 
            });
            return hits;
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
            calibHits.emplace_back(std::move(beamSpotSP));
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
        if (!checkPrecHitCount(calibHits) ) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Not enough degree of freedom available. What shall be fitted?!");
            return nullptr;
        }
        if (m_cfg.visionTool) {
            Result_t preFit{};
            preFit.parameters = startPars;
            preFit.measurements = calibHits;
            auto seedCopy = convertToSegment(localToGlobal, parent, std::move(preFit));
            m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Prefit");
        }
        Result_t segFit = callLineFit(cctx, startPars, localToGlobal, std::move(calibHits));
        if (m_cfg.visionTool && segFit.converged) {
            auto seedCopy = convertToSegment(localToGlobal, parent, Result_t{segFit});
            m_cfg.visionTool->visualizeSegment(ctx, *seedCopy, "Intermediate fit"); 
        }
        if (!removeOutliers(cctx, *parent, localToGlobal,
                            segFit.converged ? segFit.parameters : startPars,
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

        if (!checkPrecHitCount(fitResult.measurements) || 
            fitResult.nIter > m_fitter.config().maxIter) {
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

        /** Check again the available DOF and number of precision hits after hit removal */
        if (!checkPrecHitCount(fitResult.measurements)) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ 
                <<": No degree of freedom available after outlier removal. n-meas: "
                <<countPrecHits(fitResult.measurements)<<std::endl<<print(fitResult.measurements));
            return false;
        }

        /** Refit the segment line without the measurement */
        Result_t newAttempt = callLineFit(cctx, startPars, localToGlobal, 
                                          std::move(fitResult.measurements));
        if (newAttempt.converged) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" The outlier removal converged.");
            newAttempt.nIter+=fitResult.nIter;
            fitResult = std::move(newAttempt);
             if (m_cfg.visionTool) {
                const EventContext& ctx{*cctx.get<const EventContext*>()};
                auto seedCopy = convertToSegment(localToGlobal, &seed, Result_t{fitResult});
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
            // move the straws to the end of the vector
            if (a->isStraw() || b->isStraw()) {
                return !a->isStraw();
            }
            // move the beam spot to the end of the vector
            if (a->type() == xAOD::UncalibMeasType::Other || 
                b->type() == xAOD::UncalibMeasType::Other) {
                return a->type() != xAOD::UncalibMeasType::Other;
            }
            // sort the strips by layer
            const unsigned lay_a = sorter.sectorLayerNum(*a->spacePoint());
            const unsigned lay_b = sorter.sectorLayerNum(*b->spacePoint());
            if (lay_a != lay_b) {
                return lay_a < lay_b;
            }
            if (a->fitState() != b->fitState()) {
                return a->fitState() == HitState::Valid;
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
                    return std::sqrt(chi2b) > m_cfg.recoveryPull;
                } else if (sTgcB->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Pad &&
                           sTgcA->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Strip) {
                    return std::sqrt(chi2a) < m_cfg.recoveryPull;
                }
            }

           /*
            * Prefer a two-coordinate RPC/TGC space point over a phi-only
            * RPC/TGC space point, provided that the two-coordinate point
            * is compatible with the recovery-pull requirement.
            */
            if (a->type() == xAOD::UncalibMeasType::RpcStripType ||
                a->type() == xAOD::UncalibMeasType::TgcStripType) {

                const bool aEtaPhi = a->measuresEta() && a->measuresPhi();
                const bool bEtaPhi = b->measuresEta() && b->measuresPhi();

                const bool aPhiOnly = !a->measuresEta() && a->measuresPhi();
                const bool bPhiOnly = !b->measuresEta() && b->measuresPhi();

                if (aPhiOnly && bEtaPhi) {
                    // Keep the 1D point first only when the 2D point
                    // is outside the recovery-pull requirement.
                    return std::sqrt(chi2b) > m_cfg.recoveryPull;
                } else if (aEtaPhi && bPhiOnly) {
                    // Put the 2D point first when it is compatible.
                    return std::sqrt(chi2a) < m_cfg.recoveryPull;
                }
            }

            return chi2a < chi2b;
        });

        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": Check for duplicate strip hits");
        /// Loop over the hits to mark the less compatible hits on the layer as outlier
        for (HitVec_t::iterator itr = hits.begin(); itr != hits.end(); ++itr) {
            const Hit_t& hit_a{*itr};
            // Straws and the beamspot are after all the strips have been passed
            if (hit_a->isStraw() || hit_a->type() == xAOD::UncalibMeasType::Other) {
                break;
            }
            if(hit_a->fitState() == HitState::Duplicate) {
                continue;
            }
            const unsigned lay_a = sorter.sectorLayerNum(*hit_a->spacePoint());
            ///
            for (HitVec_t::iterator itr2 = itr + 1; itr2 != hits.end(); ++itr2) {
                const Hit_t& hit_b{*itr2};
                if (hit_b->type() == xAOD::UncalibMeasType::Other || hit_b->isStraw()) {
                    break;
                }
                if (hit_b->fitState() == HitState::Duplicate) {
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
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Compare results -- old chi2: "<<redChi2Old<<", nDoF: "
                    <<oldResult.nDoF<<" vs. new chi2: "<<redChi2New<<", nDoF: "<<newResult.nDoF
                    <<" -- outlier removal: "<<m_cfg.outlierRemovalCut);
        if (newResult.nDoF == oldResult.nDoF) {
            //check the number of precision hits
            const std::size_t newPrecisionHits = countPrecHits(newResult.measurements);
            const std::size_t oldPrecisionHits = countPrecHits(oldResult.measurements);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Compare results -- old precHits: "<<oldPrecisionHits
                        <<" vs. new precHits: "<<newPrecisionHits);
            return (newPrecisionHits > oldPrecisionHits && redChi2New < m_cfg.outlierRemovalCut) ||
                   redChi2New < redChi2Old;
        }
        return (redChi2New < m_cfg.outlierRemovalCut && newResult.nDoF > oldResult.nDoF) ||
               (redChi2New > m_cfg.outlierRemovalCut && redChi2New < redChi2Old);
    }
    bool SegmentLineFitter::plugHoles(const Acts::CalibrationContext& cctx,
                                      const SegmentSeed& seed,
                                      const Amg::Transform3D& localToGlobal,
                                      Result_t& toRecover) const {
        /** We've the first estimator of the segment fit */
        ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__ <<": segment "<<toString(toRecover.parameters)
                        <<", chi2: "<< calcRedChi2(toRecover) <<", nDoF: "<<toRecover.nDoF
                        <<std::endl<<print(copyAndSort(toRecover.measurements)));
        /** Setup a map to replace space points if they better suite */
        std::vector<const SpacePoint*> usedSpacePoints{};
        usedSpacePoints.reserve(toRecover.measurements.size());
        for (auto& hit : toRecover.measurements) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__ <<": "<<(*hit)<<" is known");
            usedSpacePoints.push_back(hit->spacePoint());
        }

        const EventContext& ctx{*cctx.get<const EventContext*>()};
        
        const double timeOff = toRecover.parameters[toUnderlying(ParamDefs::t0)];
        HitVec_t candidateHits{};
        std::size_t recovCandidates{0};
        const auto [locPos, locDir] = makeLine(toRecover.parameters);

         /// Loop over all hits in the parent bucket
        for (const auto& hit : *seed.parentBucket()){            
            /// Hit already used in the segment fit
            if (Acts::rangeContainsValue(usedSpacePoints, hit.get())) { 
                continue;
            }
            Hit_t calibHit{};
            double pull{-1.};
            if (hit->isStraw()) {
                using namespace Acts::detail::LineHelper;
                const double dist = signedDistance(locPos, locDir, hit->localPosition(), hit->sensorDirection());
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->primaryMeasurement());
                // Check whether the tube is crossed by the hit 
                if (std::abs(dist) >= dc->readoutElement()->innerTubeRadius()) {
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
                /// a calibration is actually worth
                pull = std::sqrt(SeedingAux::chi2Term(locPos, locDir, *hit));
                if (pull > 1.1 * m_cfg.recoveryPull) {
                    continue;
                }
            }
            calibHit = m_cfg.calibrator->calibrate(ctx, hit.get(), locPos, locDir, ActsTrk::timeToActs(timeOff));
            calibHit->setChi2Term(SeedingAux::chi2Term(locPos, locDir, *calibHit));
            if (calibHit->chi2Term() <= Acts::square(m_cfg.recoveryPull)) {
                recovCandidates += calibHit->fitState() == HitState::Valid;
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
        if (!recovCandidates) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": No space point candidates for recovery were found");
            toRecover.measurements.insert(toRecover.measurements.end(), 
                                          std::make_move_iterator(candidateHits.begin()),
                                          std::make_move_iterator(candidateHits.end()));
            eraseWrongHits(toRecover);
            return toRecover.nDoF > 0;
        }
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Found "<<recovCandidates<<" space points for recovery. ");

        HitVec_t hitsForRecovery = toRecover.measurements;
        /// Remove the beamspot constraint measurement
        if (m_cfg.doBeamSpot) {
            removeBeamSpot(hitsForRecovery);
        }

        hitsForRecovery.insert(hitsForRecovery.end(), candidateHits.begin(), candidateHits.end());

        cleanStripLayers(hitsForRecovery);

        Result_t recovered = callLineFit(cctx, toRecover.parameters, localToGlobal, 
                                         std::move(hitsForRecovery));
       
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
             *  as outlier are feasable to the hole recovery*/
            unsigned recovLoop{(candidateHits.size() == recovCandidates)*m_cfg.nRecoveryLoops};
            while (++recovLoop <= m_cfg.nRecoveryLoops) {   
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Enter recovery loop "<<recovLoop<<".");
                hitsForRecovery = toRecover.measurements;
                // Remove the beamspot
                if (m_cfg.doBeamSpot) {
                    removeBeamSpot(hitsForRecovery);
                }
                // Check whether an outlier can be lifted to on-track
                for (HitVec_t::value_type& hit : hitsForRecovery) {
                    if (hit->fitState() != HitState::Outlier) {
                        continue;
                    }
                    if (hit->chi2Term() < Acts::square(m_cfg.recoveryPull)) {
                        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Try to recover outlier "<<(*hit));
                        hit->setFitState(HitState::Valid);
                        stripOutliers.push_back(hit.get());
                    } 
                }
                // Nothing to recover
                if (stripOutliers.empty()) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": No additional measurement found");
                    break;
                }
                // Ensure that only one hit per layer is fit
                cleanStripLayers(hitsForRecovery);
                // Recovery turned out to be duplicates on the same layer
                if (std::ranges::none_of(stripOutliers,[](const CalibratedSpacePoint* sp) {
                        return sp->fitState() == HitState::Valid;
                    })) {
                    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Outliers turned out to be all duplicates.");
                    break;
                }
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Start fit without the outliers.");
                stripOutliers.clear();
                recovered = callLineFit(cctx, toRecover.parameters, localToGlobal, std::move(hitsForRecovery));
                if (!betterResult(recovered, toRecover)) {
                    break;
                }
                recovered.nIter += toRecover.nIter;
                toRecover = std::move(recovered);
            }
        } else{
            for (HitVec_t::value_type& hit : candidateHits) {
                hit->setFitState(HitState::Outlier);
                toRecover.measurements.push_back(std::move(hit));
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Reject refitted segment. Append hits as outliers: "
                    <<std::endl<<print(copyAndSort(toRecover.measurements)));
        }
        eraseWrongHits(toRecover);
        return true;
    }        
    inline bool SegmentLineFitter::checkPrecHitCount(const HitVec_t& candidateHits) const {
        using namespace Muon::MuonStationIndex;

        const size_t nPrecHits = countPrecHits(candidateHits);
        if (nPrecHits < m_cfg.nPrecHitCut) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": Not enough precision hits for segment fit. "
                <<nPrecHits<<" < "<<m_cfg.nPrecHitCut);
            return false;
        }

        const auto firstHit {std::ranges::find_if(candidateHits, [](const Hit_t& hit){
            return hit->spacePoint() != nullptr;
        })};
        assert(firstHit != candidateHits.end());
        if (toStationIndex((*firstHit)->spacePoint()->msSector()->chamberIndex()) == StIndex::EI &&
            std::ranges::any_of(candidateHits, [](const Hit_t& hit){
                return xAOD::isNSW(hit->type()); })) {

            std::array<std::size_t, 3> nStrips{Acts::filledArray<std::size_t, 3>(0u)};
            std::size_t nPhiHits {0u};
            for (const Hit_t& hit : candidateHits) {
                if (!isGoodHit(*hit)) {
                    continue;
                }
                
                if (hit->type() == xAOD::UncalibMeasType::sTgcStripType) {
                    nStrips[0] += isPrecisionHit(*hit);
                    nPhiHits += hit->measuresPhi();
                    continue;
                } else if (hit->type() == xAOD::UncalibMeasType::MMClusterType) {
                    const auto* mmClust = dynamic_cast<const xAOD::MMCluster*>(hit->spacePoint()->primaryMeasurement());
                    assert(mmClust);
                    const auto& design = mmClust->readoutElement()->stripLayer(mmClust->measurementHash()).design();
                    if (!design.hasStereoAngle()) {
                        ++nStrips[0];
                    } else if (design.stereoAngle() > 0.) {
                        ++nStrips[1];
                    } else {
                        ++nStrips[2];
                    }
                }
            }
            /** Check whether there is at least one of each micromega strip type.
             *  To have a sane topology we need to have at least 2 strips from one kind. */

            std::size_t nEtaOrientations = 
                std::ranges::count_if(nStrips, [](std::size_t n){ return n > 0; });
            if (nEtaOrientations == 3u) {
                nEtaOrientations += std::ranges::any_of(  nStrips, [](std::size_t n){ return n > 1; });
            }
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<":  nHits: "<<candidateHits.size()
                <<", nPhiHits: "<<nPhiHits<<", nEtaOrientations: "<<nEtaOrientations
                <<", N X-strips: "<<nStrips[0]<<", U-strips: "<<nStrips[1]<<", V-strips: "<<nStrips[2]);

            if ( nEtaOrientations == 4u ||
                (nEtaOrientations == 3u && nPhiHits >= 1u) ||
                (nEtaOrientations == 2u && nPhiHits >= 2u)|| 
                (std::ranges::any_of(nStrips, [](std::size_t n){ return n >= 2u; }) && nPhiHits >= 2u)) {
                return true;
            }
            return false;
        }
        return true;
    }
}
