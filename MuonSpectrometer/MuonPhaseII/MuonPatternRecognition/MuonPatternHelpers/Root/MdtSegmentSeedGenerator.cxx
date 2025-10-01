/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <EventPrimitives/EventPrimitivesHelpers.h>

#include <Acts/Utilities/Enumerate.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Utilities/UnitVectors.hpp>

#include <CxxUtils/sincos.h>
#include <format>

using namespace Acts;
using namespace Acts::UnitLiterals;
namespace MuonR4::SegmentFit{
    using HitVec = SpacePointPerLayerSplitter::HitVec;
    using HitLayerVec = SpacePointPerLayerSplitter::HitLayVec;
    constexpr auto covIdx = Acts::toUnderlying(AxisDefs::etaCov);

    
    inline Muon::MdtDriftCircleStatus dcStatus(const SpacePoint& dc) {
        const xAOD::UncalibratedMeasurement* prd = dc.primaryMeasurement();
        if (prd->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            return static_cast<const xAOD::MdtDriftCircle*>(prd)->status();
        }
        return Muon::MdtDriftCircleStatus::MdtStatusUnDefined;
    }
    /** @brief Returns whether the Mdt measurement has a valid space point */
    inline bool isGoodDC(const SpacePoint& dc) {
        return dcStatus(dc) == Muon::MdtDriftCircleStatus::MdtStatusDriftTime;
    } 
    /** @brief Move to the next space point with valid drift radius
     *  @param hits: List of hits in a particular tube layer
     *  @param hitIdx: Index of the current hit inside this list */
    inline bool moveToNextHit(const HitVec& hits, std::size_t& hitIdx) {
        while(++hitIdx < hits.size() && !isGoodDC(*hits[hitIdx])) {
        }
        return hitIdx < hits.size() && isGoodDC(*hits[hitIdx]);
    }
    /** @brief Find the first good hit in a layer
     *  @param hits: List of hits in a particular tube layer
     *  @param hitIdx: Index of the current hit inside this list */
    inline bool firstGoodHit(const HitVec& hits, std::size_t& hitIdx) {
        hitIdx = 0;
        return isGoodDC(*hits[hitIdx]) || moveToNextHit(hits, hitIdx);
    }

    std::ostream& MdtSegmentSeedGenerator::SeedSolution::print(std::ostream& ostr) const{
        ostr<<"two circle solution with ";
        ostr<<"theta: "<<(theta / 1._degree) <<" pm "<<(dTheta / 1._degree)<<", ";
        ostr<<"y0: "<<y0<<" pm "<<dY0;
        return ostr;
    }
    const MdtSegmentSeedGenerator::Config& MdtSegmentSeedGenerator::config() const {
        return m_cfg;
    }

    MdtSegmentSeedGenerator::~MdtSegmentSeedGenerator() = default;
    MdtSegmentSeedGenerator::MdtSegmentSeedGenerator(const std::string& name,
                                                     const SegmentSeed* segmentSeed, 
                                                     const Config& configuration):
            AthMessaging{name},
            m_cfg{configuration},
            m_segmentSeed{segmentSeed} {

        if (m_hitLayers.mdtHits().empty()) return;
        
        if (std::ranges::any_of(m_hitLayers.mdtHits(), [this](const HitVec& vec){
                return vec.size() > m_cfg.busyLayerLimit;
            })) {
            m_cfg.startWithPattern = false;
        }
        // Set the start for the upper layer
        m_upperLayer = m_hitLayers.mdtHits().size()-1; 

        /** Check whether the first layer is too busy */
        while (m_lowerLayer < m_upperLayer){
            const HitVec& lower{m_hitLayers.mdtHits()[m_lowerLayer]};
            if (lower.size() > m_cfg.busyLayerLimit || !firstGoodHit(lower, m_lowerHitIndex)) {
                ++m_lowerLayer;
            } else {
                break;
            }
            
        }
        /** Check whether the lower layer is too busy */
        while (m_lowerLayer < m_upperLayer){
         const HitVec& upper{m_hitLayers.mdtHits()[m_upperLayer]};
            if (upper.size() > m_cfg.busyLayerLimit || !firstGoodHit(upper, m_upperHitIndex)) {
                --m_upperLayer;
            } else {
                break;
            }

        }

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const auto [layCount, layer] : Acts::enumerate(m_hitLayers.mdtHits())) { 
                sstr<<"Mdt-hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<(*hit)<<std::endl;
                }
            }
            for (const auto [layCount, layer] : Acts::enumerate(m_hitLayers.stripHits())) { 
                sstr<<"Hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<(*hit)<<std::endl;
                }
            }
            ATH_MSG_VERBOSE("SeedGenerator - sorting of hits done. Mdt layers: "<<m_hitLayers.mdtHits().size()
                            <<", strip layers: "<<m_hitLayers.stripHits().size()<<std::endl<<sstr.str()<<std::endl<<std::endl);
        }
    }
    
    unsigned int MdtSegmentSeedGenerator::numGenerated() const {
        return m_nGenSeeds;
    }
    inline void MdtSegmentSeedGenerator::moveToNextCandidate() {
        const HitVec& lower = m_hitLayers.mdtHits()[m_lowerLayer];
        const HitVec& upper = m_hitLayers.mdtHits()[m_upperLayer];
        /// Vary the left-right solutions 
        if (++m_signComboIndex < s_signCombos.size()) {
            return;
        }
        m_signComboIndex = 0; 
        
        /// Move to the next hit in the lower layer
        if (moveToNextHit(lower, m_lowerHitIndex)) {
            return;
        }
        /// Ensure that the first hit in the lower layer is good for seeding
        if (firstGoodHit(lower, m_lowerHitIndex)) {
            /// --> So we can update to the next hit in the upper layer
            if (moveToNextHit(upper, m_upperHitIndex)) {
                return;
            }
        }
        /// All combinations of hits & lines in both layers are processed
        /// Switch to the next lowerLayer. But skip the busy ones according to the configuration
        while (m_lowerLayer < m_upperLayer) {
            const HitVec& nextLower{m_hitLayers.mdtHits()[++m_lowerLayer]};
            if (nextLower.size() > m_cfg.busyLayerLimit) {
                continue;
            }
            /// The layer has ineed a good hit
            if (firstGoodHit(nextLower, m_lowerHitIndex)) {
                break;
            }
        }

        if (m_lowerLayer < m_upperLayer) {
            firstGoodHit(upper, m_upperHitIndex);
            return;
        }
        /** Abort the loop if we parsed the multi-layer boundary */
        if (m_lowerLayer >= m_hitLayers.firstLayerFrom2ndMl() && numGenerated()){
            m_lowerLayer = m_upperLayer;
            return;
        }
        /** Go to the first lower layer */
        m_lowerLayer = 0; 
        do {
            /** But search at least the first valid hit there */
            const HitVec& nextLower{m_hitLayers.mdtHits()[m_lowerLayer]};
            if (nextLower.size() > m_cfg.busyLayerLimit) {
                continue;
            }
            /// The layer has ineed a good hit
            if (firstGoodHit(nextLower, m_lowerHitIndex)) {
                break;
            }
        } while (++m_lowerLayer < m_upperLayer);

        while (m_lowerLayer < m_upperLayer) {
            const HitVec& nextUpper{m_hitLayers.mdtHits()[--m_upperLayer]};
            if (nextUpper.size() > m_cfg.busyLayerLimit) {
                continue;
            }
            if (firstGoodHit(nextUpper, m_upperHitIndex)) {
                break;
            }
        }
    }
    std::optional<MdtSegmentSeedGenerator::DriftCircleSeed> 
        MdtSegmentSeedGenerator::nextSeed(const EventContext& ctx) {
        std::optional<DriftCircleSeed> found = std::nullopt; 
        if (!m_nGenSeeds && m_cfg.startWithPattern) {
            ++m_nGenSeeds;
            found = std::make_optional<DriftCircleSeed>();
            found->parameters = m_segmentSeed->parameters();
            found->measurements = m_cfg.calibrator->calibrate(ctx,
                                                              m_segmentSeed->getHitsInMax(),
                                                              m_segmentSeed->localPosition(),
                                                              m_segmentSeed->localDirection(),0.);
            found->parentBucket = m_segmentSeed->parentBucket();
            found->nMdt = std::ranges::count_if(m_segmentSeed->getHitsInMax(),
                                                [](const SpacePoint* hit){
                                                    return hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType;
                                                });
            SeedSolution patternSeed{};
            patternSeed.seedHits.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.solutionSigns.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.y0 = m_segmentSeed->interceptY();
            patternSeed.theta = m_segmentSeed->tanTheta();
            m_seenSolutions.push_back(std::move(patternSeed));
            return found;
        }
       
        while (m_lowerLayer < m_upperLayer) {
            const HitVec& lower = m_hitLayers.mdtHits().at(m_lowerLayer);
            const HitVec& upper = m_hitLayers.mdtHits().at(m_upperLayer);
            ATH_MSG_VERBOSE("Layers with hits: "<<m_hitLayers.mdtHits().size()
                            <<" -- next bottom hit: "<<m_lowerLayer<<", hit: "<<m_lowerHitIndex
                            <<" ("<<lower.size()<<"), top hit " <<m_upperLayer<<", "<<m_upperHitIndex
                            <<" ("<<upper.size()<<") - ambiguity "<<s_signCombos[m_signComboIndex]);

            found = buildSeed(ctx, upper.at(m_upperHitIndex), lower.at(m_lowerHitIndex), s_signCombos.at(m_signComboIndex));
            /// Increment for the next candidate
            moveToNextCandidate();
            /// If a candidate is built return it. Otherwise continue the process
            if (found) {
                return found;
            }
        }
        return std::nullopt; 
    }
    Line_t::ParamVector MdtSegmentSeedGenerator::constructLinePars(const double theta, const double y0) const {
        Line_t::ParamVector pars{};
        pars[Acts::toUnderlying(ParamDefs::y0)] = y0;
        pars[Acts::toUnderlying(ParamDefs::x0)] = m_segmentSeed->interceptX();     
        if (Acts::abs(m_segmentSeed->tanPhi()) > std::numeric_limits<double>::epsilon()) {
            const Amg::Vector3D dirFromTan = Acts::makeDirectionFromAxisTangents(m_segmentSeed->tanPhi(),
                                                                                 std::tan(theta));
            pars[Acts::toUnderlying(ParamDefs::phi)] = dirFromTan.phi();
            pars[Acts::toUnderlying(ParamDefs::theta)]  = dirFromTan.theta();
        } else {
            pars[Acts::toUnderlying(ParamDefs::phi)] = 90._degree;
            pars[Acts::toUnderlying(ParamDefs::theta)]  = theta;
        }
        return pars;
    }
    std::optional<MdtSegmentSeedGenerator::DriftCircleSeed>  
        MdtSegmentSeedGenerator::buildSeed(const EventContext& ctx,
                                           const HoughHitType& topHit, 
                                           const HoughHitType& bottomHit, 
                                           const SignComboType& signs) {
        
        const Muon::IMuonIdHelperSvc* idHelperSvc{topHit->msSector()->idHelperSvc()};
        if (!isGoodDC(*bottomHit) || !isGoodDC(*topHit)) {
            THROW_EXCEPTION("Bad hit detected, despite that should have been captured upstream "
                <<isGoodDC(*bottomHit)<<", "<<isGoodDC(*topHit)<<" - lowerLayer: "<<m_lowerLayer
                <<", upperLayer"<<m_upperLayer<<", "<<"upperHit: "<<m_upperHitIndex<<", lowerHit: "<<m_lowerHitIndex<<" - "
                <<m_hitLayers.mdtHits().at(m_lowerLayer).size()<<", "<<m_hitLayers.mdtHits().at(m_upperLayer).size());
        }
  
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Attempt to construct seed from "<<idHelperSvc->toString(bottomHit->identify())
                      <<" && top tube "<<idHelperSvc->toString(topHit->identify()));
        
        SeedSolution solCandidate = estimateTangentLine(*topHit, *bottomHit, signs);
        if (!solCandidate.isValid){
            ATH_MSG_VERBOSE("Estimated solution is invalid" <<solCandidate);
            return std::nullopt;
        }
        
        std::unique_ptr<CalibratedSpacePoint> calibBottom{}, calibTop{};
        /// If there's an estimate on the time of arrival use it
        const double t0 = m_segmentSeed->parameters()[Acts::toUnderlying(ParamDefs::t0)];
        if (m_cfg.recalibSeedCircles) {
            m_line.updateParameters(constructLinePars(solCandidate.theta, solCandidate.y0));
            /// Create a new line position & direction which also takes the
            /// potential phi estimates into account
            calibBottom = m_cfg.calibrator->calibrate(ctx, bottomHit, m_line.position(), m_line.direction(), t0);
            calibTop = m_cfg.calibrator->calibrate(ctx, topHit, m_line.position(), m_line.direction(), t0);
            solCandidate = estimateTangentLine(*calibTop, *calibBottom, signs);
            if (!solCandidate.isValid){
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<": Recalibrated seed turned to be invalid");
                return std::nullopt;
            }
        }

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test new "<<solCandidate<<". "<<m_seenSolutions.size());

        if (std::ranges::find_if(m_seenSolutions,
                        [&solCandidate, this] (const SeedSolution& seen) {
                            const double deltaY = std::abs(seen.y0 - solCandidate.y0);
                            const double limitY = std::hypot(seen.dY0, solCandidate.dY0);
                            const double dTheta = std::abs(seen.theta - solCandidate.theta);
                            const double limitTh = std::hypot(seen.dTheta, solCandidate.dTheta);
                            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": "<<seen
                                    <<std::format(" delta Y: {:.2f} {:} {:.2f}", deltaY, deltaY < limitY ? '<' : '>', limitY)
                                    <<std::format(" delta theta: {:.2f} {:} {:.2f}", dTheta, dTheta < limitTh ? '<' : '>', limitTh) );
                            return deltaY < limitY && dTheta < limitTh;;
                        }) != m_seenSolutions.end()){
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Reject due to similarity");
            return std::nullopt;
        }

        DriftCircleSeed candidateSeed{};
        candidateSeed.parentBucket = m_segmentSeed->parentBucket();
        
        const auto finalSeedPars = constructLinePars(solCandidate.theta,solCandidate.y0);
        m_line.updateParameters(finalSeedPars);          
        /** Collect all hits close to the seed line */
        for (const auto [layerNr,  hitsInLayer] : Acts::enumerate(m_hitLayers.mdtHits())) {
            ATH_MSG_VERBOSE( __func__<<"() "<<__LINE__<<": "<<hitsInLayer.size()<<" hits in layer "<<(layerNr +1));
            bool hadGoodHit{false};
            for (const HoughHitType testMe : hitsInLayer) {
                using namespace Acts::detail::LineHelper;
                const double distance = Acts::abs(signedDistance(testMe->localPosition(), testMe->sensorDirection(),
                                                                 m_line.position(), m_line.direction()));
                const double pull = Acts::abs(distance - testMe->driftRadius()) / std::sqrt(testMe->covariance()[covIdx]);

                const auto* re = static_cast<const xAOD::MdtDriftCircle*>(testMe->primaryMeasurement())->readoutElement();

                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->localPosition())<<", pull: "<<pull<<", distance: "<<distance);
                if (pull < m_cfg.hitPullCut && distance < re->tubeRadius()) {
                    hadGoodHit = true;
                    solCandidate.seedHits.emplace_back(testMe);
                    candidateSeed.nMdt += isGoodDC(*testMe);
                }/// what ever comes after is not matching onto the segment 
                else if (hadGoodHit) {
                    break;
                } 
            }
        }
        /** Reject seeds with too litle Mdt hit association */
        const unsigned hitCut = std::max(1.*m_cfg.nMdtHitCut, 
                                         m_cfg.nMdtLayHitCut * m_hitLayers.mdtHits().size()); 

        if (1.*candidateSeed.nMdt < hitCut) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Too few hits associated "<<candidateSeed.nMdt<<", expect: "<<hitCut<<" hits.");
            return std::nullopt;
        }
        /* Calculate the left-right signs of the used hits */
        if (m_cfg.overlapCorridor) {
           solCandidate.solutionSigns = SeedingAux::strawSigns(m_line, solCandidate.seedHits);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Circle solutions for seed "
                          <<idHelperSvc->toStringChamber(bottomHit->identify())<<" - "<<solCandidate);
            /** Last check wheather another seed with the same left-right combination hasn't already been found */
            for (unsigned int a = m_cfg.startWithPattern; a< m_seenSolutions.size() ;++a) { 
                const SeedSolution& accepted = m_seenSolutions[a];
                unsigned int nOverlap{0};
                std::vector<int> corridor = SeedingAux::strawSigns(m_line, accepted.seedHits);                
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test seed against accepted "<<accepted<<", updated signs: "<<corridor);
                /// All seed hits are of the same size
                for (unsigned int l = 0; l < accepted.seedHits.size(); ++l){
                    nOverlap += (corridor[l] == accepted.solutionSigns[l]);
                }
                /// Including the places where no seed hit was assigned. Both solutions match in terms of 
                /// left-right solutions. It's very likely that they're converging to the same segment.
                if (nOverlap == corridor.size() && accepted.seedHits.size() >= solCandidate.seedHits.size()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Same set of hits collected within the same corridor");
                    return std::nullopt;
                }
            }
        }
        candidateSeed.parameters[Acts::toUnderlying(ParamDefs::t0)] = t0;
        for (const auto p : { ParamDefs::x0, ParamDefs::theta, ParamDefs::phi, ParamDefs::y0}) {
            candidateSeed.parameters[Acts::toUnderlying(p)] = finalSeedPars[Acts::toUnderlying(p)];
        }      
        /// Calibrate the seed hits
        for (const HoughHitType& hit : solCandidate.seedHits){
            //calibBottom is nullptr after it has been moved, so...
            //cppcheck-suppress accessMoved
            if (hit == bottomHit && calibBottom) {
                candidateSeed.measurements.emplace_back(std::move(calibBottom));
            } 
            //calibTop is nullptr after it has been moved, so...
            //cppcheck-suppress accessMoved
            else if (hit == topHit && calibTop) {
                candidateSeed.measurements.emplace_back(std::move(calibTop));
            } else {
                candidateSeed.measurements.emplace_back(m_cfg.calibrator->calibrate(ctx, hit, m_line.position(), 
                                                                                    m_line.direction(), t0));
            }
        }

        /// Add the solution to the list. That we don't iterate twice over it
        m_seenSolutions.emplace_back(std::move(solCandidate));
        /** If we found a long Mdt seed, then ensure that all
         *  subsequent seeds have at least the same amount of Mdt hits. */
        if (m_cfg.tightenHitCut) {
            m_cfg.nMdtHitCut = std::max(m_cfg.nMdtHitCut, candidateSeed.nMdt);
        }
        ++m_nGenSeeds;        
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": In event "<<ctx.eventID().event_number()<<" found new seed solution "<<toString(candidateSeed.parameters));

        /** Associate strip hits */
        for (const std::vector<HoughHitType>& hitsInLayer : m_hitLayers.stripHits()) {
            HoughHitType bestHitLoc0{nullptr}, bestHitLoc1{nullptr};
            double bestPullLoc0{m_cfg.hitPullCut}, bestPullLoc1{m_cfg.hitPullCut};
            for (const HoughHitType testMe : hitsInLayer) {
                const double pull = std::sqrt(SeedingAux::chi2Term(m_line, *testMe));
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->localPosition())<<", pull: "<<pull<<".");
                if (testMe->measuresLoc0() &&  pull < bestPullLoc0) {
                    bestPullLoc0 = pull;
                    bestHitLoc0 = testMe;
                }
                if (testMe->measuresLoc1() &&  pull < bestPullLoc1) {
                    bestPullLoc1 = pull;
                    bestHitLoc1 = testMe;
                }                
            }
            if (bestHitLoc0) {
                candidateSeed.measurements.push_back(m_cfg.calibrator->calibrate(ctx, bestHitLoc0, m_line.position(),
                                                                                 m_line.direction(), t0));
            }
            if (bestHitLoc1 && bestHitLoc1 != bestHitLoc0) {
                candidateSeed.measurements.push_back(m_cfg.calibrator->calibrate(ctx, bestHitLoc1, m_line.position(),
                                                                                 m_line.direction(), t0));                
            }
           
        }
        return candidateSeed;
    }
}
