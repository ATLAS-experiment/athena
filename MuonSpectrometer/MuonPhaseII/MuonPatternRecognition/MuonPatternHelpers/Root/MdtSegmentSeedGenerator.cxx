/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonSpacePointCalibrator/ISpacePointCalibrator.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>


namespace MuonR4{
    using namespace SegmentFit;
    using namespace SegmentFitHelpers;
    using HitVec = SpacePointPerLayerSorter::HitVec;

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
       
        if (msgLvl(MSG::DEBUG)) {
            std::stringstream sstr{};
            unsigned int layCount{0};
            for (const HitVec& layer : m_hitLayers.mdtHits()) { 
                sstr<<"Mdt-hits in layer "<<(++layCount)<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->chamber()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            layCount = 0;
            for (const HitVec& layer : m_hitLayers.stripHits()) { 
                sstr<<"Hits in layer "<<(++layCount)<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->chamber()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            ATH_MSG_DEBUG("SeedGenerator - sorting of hits done. Mdt layers: "<<m_hitLayers.mdtHits().size()
                         <<", strip layers: "<<m_hitLayers.stripHits().size()<<std::endl<<sstr.str());
        }
    }
    
    unsigned int MdtSegmentSeedGenerator::numGenerated() const {
        return m_nGenSeeds;
    }
    
    inline void MdtSegmentSeedGenerator::moveToNextCandidate() {
        const HitVec& lower = m_hitLayers.mdtHits()[m_lowerLayer];
        const HitVec& upper = m_hitLayers.mdtHits()[m_upperLayer];
        /// All 8 sign combinations are tried. 
        if (++m_signComboIndex >= s_signCombos.size()){
            m_signComboIndex = 0; 
            /// Get the next lower hit & check whether boundary is exceeded
            if (++m_lowerHitIndex >= lower.size()){
                m_lowerHitIndex=0;
                /// Same for the hit in the upper layer
                if (++m_upperHitIndex >= upper.size()) {
                    m_upperHitIndex = 0;
                    /// All combinations of hits & lines in both layers are processed
                    /// Switch to the next lowerLayer. But skip the busy ones. For now  place a cut on 3 hits
                    while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[++m_lowerLayer].size() > m_cfg.busyLayerLimit){
                    } 
                    if (m_lowerLayer >= m_upperLayer){
                        m_lowerLayer = 0; 
                        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[--m_upperLayer].size() > m_cfg.busyLayerLimit){
                        }
                    }
                }
            }
        }
    }
    std::optional<SegmentSeed> MdtSegmentSeedGenerator::nextSeed(const EventContext& ctx) {
        std::optional<SegmentSeed> found = std::nullopt; 
        if (!m_nGenSeeds && m_cfg.startWithPattern) {
            ++m_nGenSeeds;
            found = *m_segmentSeed;
            SeedSolution patternSeed{};
            patternSeed.seedHits.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.solutionSigns.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.Y0 = m_segmentSeed->interceptY();
            patternSeed.tanTheta = m_segmentSeed->tanTheta();
            m_seenSolutions.push_back(std::move(patternSeed));
            return found;
        }
       
        while (m_lowerLayer < m_upperLayer) {
            const HitVec& lower = m_hitLayers.mdtHits().at(m_lowerLayer);
            const HitVec& upper = m_hitLayers.mdtHits().at(m_upperLayer);
            ATH_MSG_VERBOSE("Layers with hits: "<<m_hitLayers.mdtHits().size()
                            <<" -- next bottom hit: "<<m_lowerLayer<<", hit: "<<m_lowerHitIndex
                            <<" ("<<lower.size()<<"), topHit " <<m_upperLayer<<", "<<m_upperHitIndex
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
    /* Calculate the line parameters, y_{0}  & tanTheta from 2 measurements:
     * 
     *      y_{0} = T_{y1} - tanTheta * T_{z1} +- r_{1} * sqrt( 1+ tanTheta^{2})
     *      y_{0} = T_{y2} - tanTheta * T_{z2} +- r_{2} * sqrt( 1+ tanTheta^{2})
     * 
     * --> 0 = T_{y1} - T_{y2} - tanTheta*[T_{z1} - T_{z2}] + (+-r_{1} - +-r_{2})*sqrt( 1+ tanTheta^{2})
     * -->     T_{y2} - T_{y1} - tanTheta*[T_{z2} - T_{z1}] = (+-r_{2} - +-r_{1})*sqrt( 1+ tanTheta^{2})
     *      with D = T_{2} - T_{1}, R = (+-r_{1} - +-r_{2}) 
     *          D_{y} - tanTheta* D_{z} = R*sqrt(1 + tanTheta^{2})
     *     D_{y}^{2} - 2 D_{y}*D_{z} * tanTheta + D_{z}^{2} * tanTheta^{2} = R^{2}(1+tanTheta^{2}) 
     *    (D_{z}^{2} -  R^{2})* tanTheta^{2} - 2 D_{y}*D_{z} * tanTheta + D_{y}^{2} - R^{2} = 0
     * 
     *                       2* D_{y}*D_{z} +- sqrt ( 4 *(D_{y}*D_{z})^{2} - 4 * (D_{y}^{2} - R^{2}) * (D_{z}^{2} -  R^{2}))
     *       tanTheta =     -----------------------------------------------------------------------------------------------
     *                                            2 * (D_{z}^{2} -  R^{2})
     * 
     *      simplify the sqrt term -->
     *                      D_{y}*D_{z} +- |R|sqrt (D_{z}^{2} + D_{y}^{2} - R^{2})    D_{y}*D_{z} +- |R|*S
     *       tanTheta =   -------------------------------------------------------- =  --------------------
     *                                      (D_{z}^{2} -  R^{2})                             K
     * 
     *    dTanTheta     K*( +- dR/dr_{i} * S - +-|R|^{2}dR / dr_{i} / S) - (D_{y}*D_{z} +- S)(-2*R dR/dr_{i})
     *    ---------  =  -------------------------------------------------------------------------------------
     *       dr_{i}                            K^{2}
     * 
     *    dy_{0}          dtanTheta                                                    tanTheta          dtanTheta
     *   --------  =  -  ---------- T_{zi} +- sqrt( 1 + tanTheta^{2}) +- r_{i}   -------------------  * ---------- 
     *    dr_{i}           dr_{i}                                                sqrt(1+ tanTheta^2)       dr_{i}   */
    std::optional<SegmentSeed>  
        MdtSegmentSeedGenerator::buildSeed(const EventContext& ctx,
                                           const HoughHitType & topHit, 
                                           const HoughHitType & bottomHit, 
                                           const std::array<int,3> & signs) {
        
        const auto* bottomPrd =static_cast<const xAOD::MdtDriftCircle*>(bottomHit->primaryMeasurement()); 
        const auto* topPrd =static_cast<const xAOD::MdtDriftCircle*>(topHit->primaryMeasurement());
        if (bottomPrd->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime ||
            topPrd->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                return std::nullopt;
        }
        
        const Amg::Vector3D& bottomPos{bottomHit->positionInChamber()};
        const Amg::Vector3D& topPos{topHit->positionInChamber()};
        const Muon::IMuonIdHelperSvc* idHelperSvc{topHit->chamber()->idHelperSvc()};
        const Amg::Vector3D D = topPos - bottomPos; 
        ATH_MSG_VERBOSE("Bottom tube "<<idHelperSvc->toString(bottomHit->identify())<<" "<<Amg::toString(bottomPos)
                        <<", driftRadius: "<<bottomHit->driftRadius()<<" - top tube "<<idHelperSvc->toString(topHit->identify())
                        <<" "<<Amg::toString(topPos)<<", driftRadius: "<<topHit->driftRadius()
                        <<"Distance: "<<Amg::toString(D));

        int signTop = signs[0];
        int signBot = signs[1];
        int solSign = signs[2]; 

        const double R = signBot *bottomHit->driftRadius() - signTop * topHit->driftRadius(); 
        double det = D.z()*D.z() + D.y()*D.y() - R * R; 
        if (det < 0) {
            ATH_MSG_WARNING("For one reason or another the drift radii are larger than the tube pitch "
                            <<bottomHit->chamber()->idHelperSvc()->toString(bottomHit->identify()));
            return std::nullopt; 
        }
        const double sqrtTerm = std::abs(R) * std::sqrt(det); 
        const double denom = D.z()*D.z() -R*R;
        const double tanTheta = (D.y()*D.z() + solSign *sqrtTerm) / denom;
        const double csec = std::hypot(1., tanTheta);
        const double Y0 = bottomPos.y() - tanTheta * bottomPos.z() + signBot * bottomHit->driftRadius()*csec;
        const double Y1 = topPos.y() - tanTheta * topPos.z() + signTop * topHit->driftRadius()*csec;
        /// Reject invalid solutions... Not totally clear why not all 8 converge
        if (std::abs(Y0 -Y1) > std::numeric_limits<float>::epsilon()) {
            ATH_MSG_VERBOSE("Mismatch between tube intercepts: "<<Y0<<" vs. "<<Y1);
            return std::nullopt;
        }

        auto dTanTheta = [&](const HoughHitType& hit){
            const int dR = hit == bottomHit? signBot : -signTop;
            return  ( dR*solSign*sqrtTerm - solSign*R*R*dR/sqrtTerm) / denom - (D.y()*D.z() + solSign*sqrtTerm)*(-2*R*dR)/ (denom*denom);
        };
        auto dY = [&](const HoughHitType& hit) {
            const double dTan = dTanTheta(hit);
            const int sign = hit == bottomHit ? signBot : signTop;
            return - hit->positionInChamber().z() * dTan + sign * csec + sign*hit->driftRadius()* tanTheta/ csec * dTan; 
        };

        /// Add the solution to the list. That we don't iterate twice over it
        SeedSolution solCandidate{};
        solCandidate.Y0 = Y0;
        solCandidate.tanTheta = tanTheta;
        solCandidate.dTanTheta = m_cfg.uncertScale * std::hypot(dTanTheta(bottomHit) * bottomPrd->driftRadiusUncert(),
                                                                dTanTheta(topHit) * topPrd->driftRadiusUncert());

        solCandidate.dY0 = m_cfg.uncertScale * std::hypot(dY(bottomHit) * bottomPrd->driftRadiusUncert(),
                                                          dY(topHit) * topPrd->driftRadiusUncert());

        ATH_MSG_VERBOSE("Test new solution tanTheta: "<<solCandidate.tanTheta<<"+-"<<solCandidate.dTanTheta
                    <<", Y0: "<<solCandidate.Y0<<"+-"<<solCandidate.dY0);
        /// Check whether the seed has been seen before
        if (std::find_if(m_seenSolutions.begin(), m_seenSolutions.end(),
                        [this, &solCandidate] (const SeedSolution& seen) {
                            return std::abs(seen.Y0 - solCandidate.Y0) < std::hypot(seen.dY0, solCandidate.dY0) &&
                                   std::abs(seen.tanTheta - solCandidate.tanTheta) < std::hypot(seen.dTanTheta, solCandidate.dTanTheta);
                        }) != m_seenSolutions.end()){
            return std::nullopt;
        }
        solCandidate.seedHits.resize(2*m_hitLayers.mdtHits().size());
        solCandidate.solutionSigns.resize(2*m_hitLayers.mdtHits().size());

        const auto covScale = [&solCandidate] (const double cov) {
            return cov / (cov + solCandidate.dTanTheta*solCandidate.dTanTheta + solCandidate.dY0 * solCandidate.dY0);
        };
        SegmentFit::Parameters candidatePars = m_segmentSeed->parameters();
        candidatePars[toInt(AxisDefs::y0)] = Y0;
        candidatePars[toInt(AxisDefs::tanTheta)] = tanTheta;
            
        const auto [linePos, lineDir] = makeLine(candidatePars);
        
        HitVec assocHits{};
        unsigned int nMdt{0}, nLayer{0};
        /** Check for the best Mdt hit per each layer */
        for (const std::vector<HoughHitType>& hitsInLayer : m_hitLayers.mdtHits()) {
            HoughHitType bestHit{nullptr}, secBestHit{nullptr};
            double bestChi2{m_cfg.chi2PerHit};
            for (const HoughHitType testMe : hitsInLayer){
                ISpacePointCalibrator::CalibSpacePointPtr calibHit = m_cfg.calibrator->calibrate(ctx,testMe, linePos,lineDir, 0.);
                if (!calibHit || calibHit->fitState() != CalibratedSpacePoint::State::Valid) {
                    continue;
                }
                const double chi2 = SegmentFitHelpers::chiSqTermMdt(linePos, lineDir, *calibHit, msg())*covScale( testMe->covariance()(Amg::y,Amg::y));              
                ATH_MSG_VERBOSE("Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->positionInChamber())<<", chi2: "<<chi2);
                /// Add all hits with a chi2 better than 5
                if (chi2 < bestChi2) {
                    secBestHit = bestHit;
                    bestHit = testMe;
                    bestChi2 = chi2;
                }
            }
            solCandidate.seedHits[2*nLayer] = bestHit;
            solCandidate.seedHits[2*nLayer +1] = secBestHit;
            ++nLayer;
            if (!bestHit) {
                continue;
            }
            assocHits.push_back(bestHit);
            ++nMdt;
            if (secBestHit && driftSign(linePos,lineDir,bestHit, msg()) != driftSign(linePos,lineDir,secBestHit, msg())) {
                ATH_MSG_VERBOSE("Add second hit to the seed.");
                assocHits.push_back(secBestHit);
                ++nMdt;               
            }
        }
        /** Reject seeds with too litle mdt hit association */
        if (nMdt < m_cfg.nMdtHitCut) {
            return std::nullopt;
        }
        /* Calculate the left-right signs of the used hits */
        if (m_cfg.overlapCorridor) {
            solCandidate.solutionSigns = driftSigns(linePos, lineDir, solCandidate.seedHits, msg());
            ATH_MSG_VERBOSE("Circle solutions for seed "<<idHelperSvc->toStringChamber(bottomHit->identify())<<" - "
                           <<"Y0: "<<solCandidate.Y0<<", tanTheta: "<<solCandidate.tanTheta<<", cricle signs: "<<solCandidate.solutionSigns);
        
            /** Last check wheather another seed with the same left-right combination hasn't already been found */
            for (unsigned int a = 1; a< m_seenSolutions.size() ;++a) { 
                const SeedSolution& accepted = m_seenSolutions[a];
                unsigned int nOverlap{0}, nAccepted{0};
                std::vector<int> corridor = driftSigns(linePos, lineDir, accepted.seedHits,  msg());                
                ATH_MSG_VERBOSE("Test seed against accepted solution Y0: "<<accepted.Y0<<", tanTheta: "<<accepted.tanTheta
                              <<", initial circle signs: "<<accepted.solutionSigns<<", updated signs: "<<corridor);
                /// All seed hits are of the same size
                for (unsigned int l = 0; l < accepted.seedHits.size(); ++l){
                    nAccepted += corridor[l] !=0;
                    nOverlap  += corridor[l] == accepted.solutionSigns[l];
                }
                /// Including the places where no seed hit was assigned. Both solutions match in terms of 
                /// left-right solutions. It's very likely that they're converging to the same segment.
                if (nOverlap == corridor.size()) {
                    ATH_MSG_VERBOSE("Shared hits at the same corridor. Reject seed.");
                    return std::nullopt;
                }
            }
        }
        m_seenSolutions.emplace_back(std::move(solCandidate));
        /** If we found a long Mdt seed, then ensure that all
         *  subsequent seeds have at least the same amount of Mdt hits. */
        if (m_cfg.tightenHitCut) {
            m_cfg.nMdtHitCut = std::max(m_cfg.nMdtHitCut, nMdt);
        }
        /** Associate strip hits */
        for (const std::vector<HoughHitType>& hitsInLayer : m_hitLayers.stripHits()) {
            HoughHitType bestHit{nullptr};
            double bestChi2{m_cfg.chi2PerHit};
            for (const HoughHitType testMe : hitsInLayer){
                const double chi2 = SegmentFitHelpers::chiSqTermStrip(linePos, lineDir, testMe, msg()) /
                                     testMe->dimension();
                ATH_MSG_VERBOSE("Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->positionInChamber())<<", chi2: "<<chi2);
                /// Add all hits with a chi2 better than 3
                if (chi2 <= bestChi2) {
                    bestHit = testMe;
                    bestChi2 = chi2;
                }
            }
            if (!bestHit) {
                continue;
            }
            assocHits.push_back(bestHit);           
        }
        ++m_nGenSeeds;

        return std::make_optional<SegmentSeed>(candidatePars[toInt(AxisDefs::tanTheta)],
                                               candidatePars[toInt(AxisDefs::y0)],
                                               candidatePars[toInt(AxisDefs::tanPhi)],
                                               candidatePars[toInt(AxisDefs::x0)],
                                               nMdt, std::move(assocHits), 
                                               m_segmentSeed->parentBucket());
    }  
}
