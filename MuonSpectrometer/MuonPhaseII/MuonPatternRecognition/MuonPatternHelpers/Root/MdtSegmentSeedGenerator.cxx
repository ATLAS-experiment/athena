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

#include <Acts/Utilities/Enumerate.hpp>
#include <CxxUtils/sincos.h>
#include <format>

namespace MuonR4{
    using namespace SegmentFit;
    using namespace SegmentFitHelpers;
    using HitVec = SpacePointPerLayerSplitter::HitVec;

    double driftCov(const CalibratedSpacePoint& dcHit){
        return std::visit([](const auto& cov) ->double{
            return Amg::error(cov, toInt(AxisDefs::eta));
        }, dcHit.covariance());
    }
    constexpr bool passRangeCut(const std::array<double, 2>& cutRange, const double value) {
        return cutRange[0] <= value && value <= cutRange[1];
    }
    inline Muon::MdtDriftCircleStatus dcStatus(const SpacePoint& dc) {
        const xAOD::UncalibratedMeasurement* prd = dc.primaryMeasurement();
        if (prd->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            return static_cast<const xAOD::MdtDriftCircle*>(prd)->status();
        }
        return Muon::MdtDriftCircleStatus::MdtStatusUnDefined;
    }

    std::ostream& MdtSegmentSeedGenerator::SeedSolution::print(std::ostream& ostr) const{
        ostr<<"two circle solution with ";
        ostr<<"theta: "<<theta / Gaudi::Units::deg<<" pm "<<dTheta / Gaudi::Units::deg<<", ";
        ostr<<"y0: "<<Y0<<" pm "<<dY0;
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
        
        if (std::ranges::find_if(m_hitLayers.mdtHits(), [this](const HitVec& vec){
            return vec.size() > m_cfg.busyLayerLimit;
        }) != m_hitLayers.mdtHits().end()) {
            m_cfg.startWithPattern = false;
        }
        // Set the start for the upper layer
        m_upperLayer = m_hitLayers.mdtHits().size()-1; 

        /** Check whether the first layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[m_lowerLayer].size() >m_cfg.busyLayerLimit){
            ++m_lowerLayer;
        }
        /** Check whether the lower layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[m_upperLayer].size() > m_cfg.busyLayerLimit) {
            --m_upperLayer;
        }

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const auto [layCount, layer] : Acts::enumerate(m_hitLayers.mdtHits())) { 
                sstr<<"Mdt-hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->msSector()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            for (const auto [layCount, layer] : Acts::enumerate(m_hitLayers.stripHits())) { 
                sstr<<"Hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->msSector()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
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
         if (++m_lowerHitIndex < lower.size()) {
            return;
        }
        m_lowerHitIndex=0;
        /// Move to the next hit in the upper layer
        if (++m_upperHitIndex < upper.size()) {
            return;
        }
         m_upperHitIndex = 0;
        /// All combinations of hits & lines in both layers are processed
        /// Switch to the next lowerLayer. But skip the busy ones according to the configuration
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[++m_lowerLayer].size() > m_cfg.busyLayerLimit) {
        } 
        
        if (m_lowerLayer < m_upperLayer) {
            return;
        }
        /** Abort the loop if we parsed the multi-layer boundary */
        if (m_lowerLayer >= m_hitLayers.firstLayerFrom2ndMl() && numGenerated()){
            m_lowerLayer = m_upperLayer;
            return;
        }
        m_lowerLayer = 0; 
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[--m_upperLayer].size() > m_cfg.busyLayerLimit){
    
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
                                                              m_segmentSeed->positionInChamber(),
                                                              m_segmentSeed->directionInChamber(),0.);
            found->parentBucket = m_segmentSeed->parentBucket();
            found->nMdt = std::ranges::count_if(m_segmentSeed->getHitsInMax(),
                                                [](const SpacePoint* hit){
                                                    return hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType;
                                                });
            SeedSolution patternSeed{};
            patternSeed.seedHits.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.solutionSigns.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.Y0 = m_segmentSeed->interceptY();
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
    std::optional<MdtSegmentSeedGenerator::DriftCircleSeed>  
        MdtSegmentSeedGenerator::buildSeed(const EventContext& ctx,
                                           const HoughHitType& topHit, 
                                           const HoughHitType& bottomHit, 
                                           const SignComboType& signs) {
        
        const auto* bottomPrd = static_cast<const xAOD::MdtDriftCircle*>(bottomHit->primaryMeasurement()); 
        const auto* topPrd = static_cast<const xAOD::MdtDriftCircle*>(topHit->primaryMeasurement());

        const Muon::IMuonIdHelperSvc* idHelperSvc{topHit->msSector()->idHelperSvc()};
        if (dcStatus(*bottomHit) != Muon::MdtDriftCircleStatus::MdtStatusDriftTime ||
            dcStatus(*topHit) != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                /// All other sign combinations will also get stuck -> force the measurement selector to get another combo
                m_signComboIndex = s_signCombos.size();
                return std::nullopt;
        }
        const auto&[signTop, signBot] = signs;
        double R = signBot *bottomHit->driftRadius() - signTop * topHit->driftRadius(); 
        const Amg::Vector3D& bottomPos{bottomHit->positionInChamber()};
        const Amg::Vector3D& topPos{topHit->positionInChamber()};

        const Amg::Vector3D D = topPos - bottomPos;
        const double thetaTubes = std::atan2(D.y(), D.z()); 
        const double distTubes =  std::hypot(D.y(), D.z());
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Bottom tube "<<idHelperSvc->toString(bottomHit->identify())<<" "<<Amg::toString(bottomPos)
                    <<", driftRadius: "<<bottomHit->driftRadius()<<" - top tube "<<idHelperSvc->toString(topHit->identify())
                    <<" "<<Amg::toString(topPos)<<", driftRadius: "<<topHit->driftRadius()
                    <<", tube distance: "<<Amg::toString(D));

        DriftCircleSeed candidateSeed{};
        candidateSeed.parameters = m_segmentSeed->parameters();
        candidateSeed.parentBucket = m_segmentSeed->parentBucket();
        double theta{thetaTubes - std::asin(std::clamp(R / distTubes, -1., 1.))};
        Amg::Vector3D seedDir = Amg::dirFromAngles(90.*Gaudi::Units::deg, theta);
        double Y0 = bottomPos.y()*seedDir.z() - bottomPos.z()*seedDir.y() + signBot*bottomHit->driftRadius();
        double combDriftUncert{std::sqrt(bottomPrd->driftRadiusCov() + topPrd->driftRadiusCov())};
        std::unique_ptr<CalibratedSpacePoint> calibBottom{}, calibTop{};
        if (m_cfg.recalibSeedCircles) {
            candidateSeed.parameters[toInt(ParamDefs::theta)] = theta;
            candidateSeed.parameters[toInt(ParamDefs::y0)] = Y0 / seedDir.z();
            /// Create a new line position & direction which also takes the
            /// potential phi estimates into account
            const auto [linePos, lineDir] = makeLine(candidateSeed.parameters);
            calibBottom = m_cfg.calibrator->calibrate(ctx, bottomHit, linePos, lineDir, 
                                                      candidateSeed.parameters[toInt(ParamDefs::time)]);
            calibTop = m_cfg.calibrator->calibrate(ctx, topHit, linePos, lineDir, 
                                                   candidateSeed.parameters[toInt(ParamDefs::time)]);
            R = signBot * calibBottom->driftRadius() - signTop * calibTop->driftRadius();
            /// Recalculate the seed with the calibrated parameters
            theta =  thetaTubes - std::asin(std::clamp(R / distTubes, -1., 1.));
            seedDir = Amg::dirFromAngles(90.*Gaudi::Units::deg, theta);
            Y0 = bottomPos.y()*seedDir.z() - bottomPos.z()*seedDir.y() + signBot*bottomHit->driftRadius();
            combDriftUncert = std::sqrt(driftCov(*calibBottom) + driftCov(*calibTop));
        }
        
        candidateSeed.parameters[toInt(ParamDefs::theta)] = theta;
        candidateSeed.parameters[toInt(ParamDefs::y0)] = Y0 / seedDir.z();
        /// Check that the is within the predefined window
        if (!passRangeCut(m_cfg.thetaRange, theta) || 
            !passRangeCut(m_cfg.interceptRange, candidateSeed.parameters[toInt(ParamDefs::y0)])) {
            return std::nullopt;
        }

        const Amg::Vector3D seedPos = Y0 / seedDir.z() * Amg::Vector3D::UnitY();

        assert(std::abs(topPos.y()*seedDir.z() - topPos.z() * seedDir.y() + signTop*topHit->driftRadius() - Y0) < std::numeric_limits<float>::epsilon() );
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Candidate seed theta: "<<theta<<", tanTheta: "<<(seedDir.y() / seedDir.z())<<", y0: "<<Y0/seedDir.z());

        SeedSolution solCandidate{};
        solCandidate.Y0 = seedPos[toInt(ParamDefs::y0)];
        solCandidate.theta = theta;
        /// d/dx asin(x) = 1 / sqrt(1- x*x)
        const double denomSquare =  1. - std::pow(R / distTubes, 2); 
        if (denomSquare < std::numeric_limits<double>::epsilon()){
            ATH_MSG_VERBOSE("Invalid seed, rejecting"); 
            return std::nullopt; 
        }
        solCandidate.dTheta =  combDriftUncert / std::sqrt(denomSquare) / distTubes;
        solCandidate.dY0 =  std::hypot(-bottomPos.y()*seedDir.y() + bottomPos.z()*seedDir.z(), 1.) * solCandidate.dTheta;
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test new "<<solCandidate<<". "<<m_seenSolutions.size());


        if (std::ranges::find_if(m_seenSolutions,
                        [&solCandidate, this] (const SeedSolution& seen) {
                            const double deltaY = std::abs(seen.Y0 - solCandidate.Y0);
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
        /** Collect all hits close to the seed line */
        for (const auto [layerNr,  hitsInLayer] : Acts::enumerate(m_hitLayers.mdtHits())) {
            ATH_MSG_VERBOSE( __func__<<"() "<<__LINE__<<": "<<hitsInLayer.size()<<" hits in layer "<<(layerNr +1));
            bool hadGoodHit{false};
            for (const HoughHitType testMe : hitsInLayer){
                const double distance = std::abs(Amg::signedDistance(seedPos, seedDir, testMe->positionInChamber(),
                                                                    testMe->directionInChamber()));
                const auto* re = static_cast<const xAOD::MdtDriftCircle*>(testMe->primaryMeasurement())->readoutElement();

                const double pull = std::sqrt(SegmentFitHelpers::chiSqTermMdt(seedPos, seedDir, *testMe, msg()));            
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->positionInChamber())<<", pull: "<<pull<<", distance: "<<distance);
                if (pull < m_cfg.hitPullCut && distance < re->tubeRadius()) {
                    hadGoodHit = true;
                    solCandidate.seedHits.emplace_back(testMe);
                    candidateSeed.nMdt += (dcStatus(*testMe) == Muon::MdtDriftCircleStatus::MdtStatusDriftTime);
                }/// what ever comes after is not matching onto the segment 
                else if (hadGoodHit) {
                    break;
                } 
            }
        }
        /** Reject seeds with too litle Mdt hit association */
        const unsigned hitCut = std::max(1.*m_cfg.nMdtHitCut, m_cfg.nMdtLayHitCut * m_hitLayers.mdtHits().size()); 

        if (1.*candidateSeed.nMdt < hitCut) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Too few hits associated "<<candidateSeed.nMdt<<", expect: "<<hitCut<<" hits.");
            return std::nullopt;
        }
        /* Calculate the left-right signs of the used hits */
        if (m_cfg.overlapCorridor) {
            solCandidate.solutionSigns = driftSigns(seedPos, seedDir, solCandidate.seedHits, msg());
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Circle solutions for seed "
                          <<idHelperSvc->toStringChamber(bottomHit->identify())<<" - "<<solCandidate);
            /** Last check wheather another seed with the same left-right combination hasn't already been found */
            for (unsigned int a = m_cfg.startWithPattern; a< m_seenSolutions.size() ;++a) { 
                const SeedSolution& accepted = m_seenSolutions[a];
                unsigned int nOverlap{0};
                std::vector<int> corridor = driftSigns(seedPos, seedDir, accepted.seedHits,  msg());                
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test seed against accepted "<<accepted<<", updated signs: "<<corridor);
                /// All seed hits are of the same size
                for (unsigned int l = 0; l < accepted.seedHits.size(); ++l){
                    nOverlap  += corridor[l] == accepted.solutionSigns[l];
                }
                /// Including the places where no seed hit was assigned. Both solutions match in terms of 
                /// left-right solutions. It's very likely that they're converging to the same segment.
                if (nOverlap == corridor.size() && accepted.seedHits.size() >= solCandidate.seedHits.size()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Same set of hits collected within the same corridor");
                    return std::nullopt;
                }
            }
        }
        /// Seed candidate is 
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
                candidateSeed.measurements.emplace_back(m_cfg.calibrator->calibrate(ctx, hit, seedPos, seedDir, 0.));
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
        if (m_cfg.fastSeedFit) {
            if (!m_cfg.fastSegFitWithT0) {
                fitDriftCircles(candidateSeed);
            } else {
                fitDriftCirclesWithT0(ctx, candidateSeed);
            }
        }

        // Combine the seed with the phi estimate
        {
            const Amg::Vector3D parDir = dirFromTangents(m_segmentSeed->tanPhi(), std::tan(theta));
            candidateSeed.parameters[toInt(ParamDefs::theta)] = parDir.theta();
            candidateSeed.parameters[toInt(ParamDefs::phi)] = parDir.phi();
        }

        /** Associate strip hits */
        {
            const auto [seedPos, seedDir] = makeLine(candidateSeed.parameters); 
            for (const std::vector<HoughHitType>& hitsInLayer : m_hitLayers.stripHits()) {
                HoughHitType bestHit{nullptr};
                double bestPull{m_cfg.hitPullCut};
                for (const HoughHitType testMe : hitsInLayer){
                    const double pull = std::sqrt(SegmentFitHelpers::chiSqTermStrip(seedPos, seedDir, *testMe, msg())) 
                                      / testMe->dimension();
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Test hit "<<idHelperSvc->toString(testMe->identify())
                                <<" "<<Amg::toString(testMe->positionInChamber())<<", pull: "<<pull);
                    /// Add all hits with a pull better than the threshold
                    if (pull <= bestPull) {
                        bestHit = testMe;
                        bestPull = pull;
                    }
                }
                if (!bestHit) {
                    continue;
                }
                candidateSeed.measurements.push_back(m_cfg.calibrator->calibrate(ctx,bestHit, seedPos, seedDir, 0));           
            }
        }
        return candidateSeed;
    }
    inline MdtSegmentSeedGenerator::SeedFitAuxilliaries 
        MdtSegmentSeedGenerator::estimateAuxillaries(const DriftCircleSeed& seed) const {

        SeedFitAuxilliaries aux{};
        /// Seed direction vector
        const Amg::Vector3D seedDir = Amg::dirFromAngles(90.*Gaudi::Units::deg, 
                                                         seed.parameters[toInt(ParamDefs::theta)]);
        /// y0Prime = y0 * cos(theta)
        const double y0 = seed.parameters[toInt(ParamDefs::y0)] * seedDir.z();

        aux.invCovs.reserve(seed.measurements.size());
        aux.driftSigns.reserve(seed.measurements.size());
        double norm{0.};
        /// Calculate the centre of gravity
        for (const std::unique_ptr<CalibratedSpacePoint>& hit : seed.measurements) {
            const double invCov = 1./ driftCov(*hit);
            const Amg::Vector3D& pos{hit->positionInChamber()};
            const int sign = y0  - pos.y() * seedDir.z() + pos.z()* seedDir.y() > 0 ? 1 : -1;

            aux.centerOfGrav+= invCov * pos;
            aux.invCovs.push_back(invCov);
            aux.driftSigns.push_back(sign);

            norm += invCov;
        }
        ///
        aux.covNorm = 1./ norm;
        aux.centerOfGrav *= aux.covNorm;
        /// Calculate the fit constants
        for (const auto [covIdx, hit] : Acts::enumerate(seed.measurements)) {
            const double& invCov = aux.invCovs[covIdx];
            const int& sign = aux.driftSigns[covIdx];
            const Amg::Vector3D pos = hit->positionInChamber() - aux.centerOfGrav;
            const double signedCov = invCov * sign;
            aux.T_zzyy += invCov * (std::pow(pos.z(), 2) - std::pow(pos.y(), 2));
            aux.T_yz   += invCov * pos.y()*pos.z();
            aux.T_rz   += signedCov * pos.z() * hit->driftRadius();
            aux.T_ry   += signedCov * pos.y() * hit->driftRadius();
            aux.fitY0  += signedCov * aux.covNorm * hit->driftRadius();
        }
        ATH_MSG_VERBOSE("Estimated T_zzyy: "<<aux.T_zzyy<<", T_yz: "<<aux.T_yz<<", T_rz: "<<aux.T_rz
                     <<", T_ry: "<<aux.T_ry<<", centre "<<Amg::toString(aux.centerOfGrav)<<", y0: "<<aux.fitY0
                     <<", norm: "<<aux.covNorm<<"/"<<norm);
        return aux;
    }
    void MdtSegmentSeedGenerator::fitDriftCircles(DriftCircleSeed& inSeed) const {

        const SeedFitAuxilliaries auxVars = estimateAuxillaries(inSeed);
        
        double theta = inSeed.parameters[toInt(ParamDefs::theta)];
        /// Now it's time to use the guestimate
        const double thetaGuess = std::atan2( 2.*(auxVars.T_yz - auxVars.T_rz), auxVars.T_zzyy) / 2.;

        ATH_MSG_VERBOSE("Start fast fit seed: "<<theta<<", guess: "<<thetaGuess
                    <<", y0: "<<inSeed.parameters[toInt(ParamDefs::y0)]
                    <<", fitY0: "<<auxVars.fitY0<<", centre: "<<Amg::toString(auxVars.centerOfGrav));
        //// 
        theta = thetaGuess;
        CxxUtils::sincos thetaCS{theta};
        bool converged{false};
        while (!converged && inSeed.nIter++ <= m_cfg.nMaxIter) {
            const CxxUtils::sincos twoTheta{2.*theta};
            const double thetaPrime = 0.5*auxVars.T_zzyy *twoTheta.sn - auxVars.T_yz * twoTheta.cs 
                                    - auxVars.T_rz * thetaCS.cs - auxVars.T_ry * thetaCS.sn;
            if (std::abs(thetaPrime) < m_cfg.precCutOff){
                converged = true;
                break;
            }

            const double thetaTwoPrime =  auxVars.T_zzyy * twoTheta.cs + 2.* auxVars.T_yz * twoTheta.sn 
                                       + auxVars.T_rz * thetaCS.sn - auxVars.T_ry * thetaCS.cs;
            const double update = thetaPrime / thetaTwoPrime;
            ATH_MSG_VERBOSE("Fit iteration #"<<inSeed.nIter<<" -- theta: "<<theta<<", thetaPrime: "<<thetaPrime
                        <<", thetaTwoPrime: "<<thetaTwoPrime<<" -- "<<std::format("{:.8f}", update)
                        <<" --> next theta "<<(theta - thetaPrime / thetaTwoPrime));

            if (std::abs(update) < m_cfg.precCutOff) {
                converged = true;
                break;
            }
            theta -= update;
            thetaCS = CxxUtils::sincos{theta};
        }
        if (!converged) {
           return;
        }
        double fitY0 = (auxVars.centerOfGrav.y() *thetaCS.cs - auxVars.centerOfGrav.z() * thetaCS.sn + auxVars.fitY0) / thetaCS.cs;
        ATH_MSG_VERBOSE("Drift circle fit converged within "<<inSeed.nIter<<" iterations giving "<<toString(inSeed.parameters)<<", chi2: "<<inSeed.chi2
                        <<" - theta: "<<theta / Gaudi::Units::deg<<", y0: "<<fitY0);
        inSeed.parameters[toInt(ParamDefs::theta)] = theta;
        inSeed.parameters[toInt(ParamDefs::y0)] = fitY0;
    }

    inline MdtSegmentSeedGenerator::SeedFitAuxWithT0 
        MdtSegmentSeedGenerator::estimateAuxillaries(const EventContext& ctx,
                                                     const DriftCircleSeed& seed) const {
        SeedFitAuxWithT0 aux{estimateAuxillaries(seed)};
        for (const auto [idx, hit] : Acts::enumerate(seed.measurements)){
            const double signedCov = aux.driftSigns[idx] * aux.invCovs[idx];
            const double weight = aux.covNorm * signedCov;
            const double velocity = m_cfg.calibrator->driftVelocity(ctx, *hit); 
            const double acceleration = m_cfg.calibrator->driftAcceleration(ctx, *hit);
            const Amg::Vector3D pos = hit->positionInChamber() - aux.centerOfGrav;
            aux.fitY0Prime+= weight * velocity;
            aux.fitY0TwoPrime+= weight * acceleration;

            aux.T_vz += signedCov * pos.z()*velocity;
            aux.T_vy += signedCov * pos.y()*velocity;
            aux.T_az += signedCov * pos.z()*acceleration;
            aux.T_ay += signedCov * pos.y()*acceleration;

            aux.R_vr += signedCov * hit->driftRadius() * velocity;
            aux.R_va += signedCov * hit->driftRadius() * acceleration;
            aux.R_vv += signedCov * velocity * velocity;
        }
        ATH_MSG_VERBOSE("Estimated T_vz: "<<aux.T_vz<<", T_vy: "<<aux.T_vy
                     <<", T_az: "<<aux.T_az<<", T_ay: "<<aux.T_ay<<" --- R_vr: "<<aux.R_vr
                     <<", R_va: "<<aux.R_va<<", R_vv: "<<aux.R_vv<<" -- Y0^{'}: "<<aux.fitY0Prime
                     <<", Y0^{''}: "<<aux.fitY0TwoPrime);
        return aux;
    }

    void MdtSegmentSeedGenerator::fitDriftCirclesWithT0(const EventContext& ctx, DriftCircleSeed& inSeed) const {
        ///
        SeedFitAuxWithT0 auxVars{estimateAuxillaries(ctx, inSeed)};
        

        bool converged{false};
        AmgSymMatrix(2) cov{AmgSymMatrix(2)::Zero()};
        AmgVector(2) grad{AmgVector(2)::Zero()};
        AmgVector(2) pars{inSeed.parameters[toInt(ParamDefs::theta)], 
                          inSeed.parameters[toInt(ParamDefs::time)]};
        

        while (!converged && inSeed.nIter++ <= m_cfg.nMaxIter) {

            const CxxUtils::sincos thetaCS{pars[0]};
            const CxxUtils::sincos twoTheta{2.*pars[0]};
            /// d^{2}chi^{2} / d^{2}theta
            cov(0,0) = auxVars.T_zzyy * twoTheta.cs + 2.* auxVars.T_yz * twoTheta.sn 
                     + auxVars.T_rz * thetaCS.sn - auxVars.T_ry * thetaCS.cs;

            // cov(1,0) =  cov(0,1) = auxVars.T_vz * thetaCS.cs + auxVars.T_vy * thetaCS.sn;
            cov(1,1) = - auxVars.fitY0Prime *auxVars.fitY0Prime  - auxVars.fitY0Prime * auxVars.fitY0TwoPrime 
                     - auxVars.T_az * thetaCS.sn - auxVars.T_ay * thetaCS.cs + auxVars.R_vv + auxVars.R_va;

            grad[0] =0.5*auxVars.T_zzyy *twoTheta.sn - auxVars.T_yz * twoTheta.cs 
                                    - auxVars.T_rz * thetaCS.cs - auxVars.T_ry * thetaCS.sn;
            grad[1] = auxVars.fitY0 * auxVars.fitY0Prime - auxVars.R_vr  + auxVars.T_vz * thetaCS.sn - auxVars.T_vy * thetaCS.cs;

            
            const AmgVector(2) update = cov.inverse()* grad;
            ATH_MSG_VERBOSE("Iteration: "<<inSeed.nIter<<", theta: "<<pars[0] / Gaudi::Units::deg<<", time: "<< 
                        pars[1]<<" gradient: ("<<(grad[0])<<", "<<grad[1]<<"), covariance:" 
                        <<std::endl<<Amg::toString(cov)<<std::endl<<" update: ("<<update[0] / Gaudi::Units::deg
                        <<", "<<update[1]<<").");
            pars -= update;
        }

    }
  
}
