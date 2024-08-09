/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>

namespace MuonR4{
    using namespace SegmentFit;

    MdtSegmentSeedGenerator::MdtSegmentSeedGenerator(const std::string& name,
                                                     const SegmentSeed* segmentSeed, 
                                                     const Config& configuration):
            AthMessaging{name},
            m_cfg{configuration},
            m_segmentSeed{segmentSeed} {
        
        const Muon::IMuonIdHelperSvc* idHelperSvc{segmentSeed->chamber()->idHelperSvc()};
        m_mdtHitsPerLayer.reserve(8);

        /// The hits are radially sorted from low local-z to high local z. Build the gasGap Identifier
        /// to find out to which layer the hit belongs to and then use the layer counting map as auxillary object
        /// fetch the indices for the sorted measurements
        using LayerCounting = std::unordered_map<Identifier, unsigned int>;
        LayerCounting mdtLayerCounting{}, stripLayerCounting{};

        for (const HoughHitType& hit : segmentSeed->getHitsInMax()) {
            const Identifier& id {hit->identify()};
            if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                /// Sort the Mdt hits
                const auto* prd = static_cast<const xAOD::MdtDriftCircle*>(hit->primaryMeasurement());
                if (prd ->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime){
                    continue;
                }
                const MdtIdHelper& idHelper{idHelperSvc->mdtIdHelper()};
                const Identifier layId = idHelper.channelID(id, idHelper.multilayer(id), idHelper.tubeLayer(id), 1);
             
                const unsigned int layer = mdtLayerCounting.insert(std::make_pair(layId, mdtLayerCounting.size())).first->second;
                if (layer >= m_mdtHitsPerLayer.size()) {
                    m_mdtHitsPerLayer.resize(layer + 1);
                }
                std::vector<HoughHitType>& pushTo{m_mdtHitsPerLayer[layer]};
                if (pushTo.capacity() == pushTo.size()){
                    pushTo.reserve(pushTo.size() + 5);
                }
                /* There's no way that the seed may be correct */
                if (pushTo.size() >=2) {
                    m_cfg.startWithPattern = false;
                }
                pushTo.push_back(hit);
                continue;
            }
            /** Sort the strip hits */
            const Identifier layId = idHelperSvc->gasGapId(id);

            const unsigned int layer = stripLayerCounting.insert(std::make_pair(layId, stripLayerCounting.size())).first->second;
            
            if (layer >= m_stripHitsPerLayer.size()) {
                m_stripHitsPerLayer.resize(layer + 1);
            }
            std::vector<HoughHitType>& pushTo{m_stripHitsPerLayer[layer]};
            if (pushTo.capacity() == pushTo.size()){
                pushTo.reserve(pushTo.size() + 5);
            }
            pushTo.push_back(hit);
        }
        if (m_mdtHitsPerLayer.empty()) return;
        // Set the start for the upper layer
        m_upperLayer = m_mdtHitsPerLayer.size()-1; 

        /** Check whether the first layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_mdtHitsPerLayer[m_lowerLayer].size() >m_cfg.busyLayerLimit){
            ++m_lowerLayer;
        }
        /** Check whether the lower layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_mdtHitsPerLayer[m_upperLayer].size() > m_cfg.busyLayerLimit) {
            --m_upperLayer;
        }
       
        if (msgLvl(MSG::DEBUG)) {
            std::stringstream sstr{};
            unsigned int layCount{0};
            for (const HitVec& layer : m_mdtHitsPerLayer) { 
                sstr<<"Mdt-hits in layer "<<(++layCount)<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->chamber()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            layCount = 0;
            for (const HitVec& layer : m_stripHitsPerLayer) { 
                sstr<<"Hits in layer "<<(++layCount)<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->chamber()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            ATH_MSG_DEBUG("SeedGenerator - sorting of hits done. Mdt layers: "<<m_mdtHitsPerLayer.size()
                         <<", strip layers: "<<m_stripHitsPerLayer.size()<<std::endl<<sstr.str());
        }
    }
    
    unsigned int MdtSegmentSeedGenerator::numGenerated() const {
        return m_nGenSeeds;
    }
    std::optional<SegmentSeed> MdtSegmentSeedGenerator::nextSeed() {
        std::optional<SegmentSeed> found = std::nullopt; 
        if (!m_nGenSeeds && m_cfg.startWithPattern) {
            ++m_nGenSeeds;
            found = *m_segmentSeed;
            m_seenSolutions.emplace_back();
            m_seenSolutions.back()[toInt(AxisDefs::y0)] = m_segmentSeed->interceptY();
            m_seenSolutions.back()[toInt(AxisDefs::tanTheta)] = m_segmentSeed->tanTheta();
            return found;
        }
       
        if (m_upperLayer <= m_lowerLayer) return std::nullopt; 
        const HitVec& lower = m_mdtHitsPerLayer.at(m_lowerLayer);
        const HitVec& upper = m_mdtHitsPerLayer.at(m_upperLayer);
        ATH_MSG_VERBOSE("Layers with hits: "<<m_mdtHitsPerLayer.size()
                    <<" -- next bottom hit: "<<m_lowerLayer<<", hit: "<<m_lowerHitIndex
                    <<" ("<<lower.size()<<"), topHit " <<m_upperLayer<<", "<<m_upperHitIndex
                    <<" ("<<upper.size()<<") - ambiguity "<<s_signCombos[m_signComboIndex]);

        found = buildSeed(upper.at(m_upperHitIndex), lower.at(m_lowerHitIndex), s_signCombos.at(m_signComboIndex));

        // now increment 
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
                    while (m_lowerLayer < m_upperLayer && m_mdtHitsPerLayer[++m_lowerLayer].size() > m_cfg.busyLayerLimit){
                    } 
                    if (m_lowerLayer >= m_upperLayer){
                        m_lowerLayer = 0; 
                        while (m_lowerLayer < m_upperLayer && m_mdtHitsPerLayer[--m_upperLayer].size() > m_cfg.busyLayerLimit){
                        }
                    }
                }
            }
        }
        return found ? found : nextSeed(); 
    }
    /* Calculate the line parameters, y_{0}  & tanTheta from 2 measurements:
     * 
     *      y_{0} = T_{y1} - tanTheta * T_{z1} +- r_{1} * sqrt( 1+ tanTheta^{2})
     *      y_{0} = T_{y2} - tanTheta * T_{z2} +- r_{2} * sqrt( 1+ tanTheta^{2})
     * 
     * --> 0 = T_{y1} - T_{y2} - tanTheta*[T_{z1} - T_{z2}]   (+-r_{1} - +-r_{2})*sqrt( 1+ tanTheta^{2})
     * -->     T_{y2} - T_{y1} - tanTheta*[T_{z2} - T_{z1}] = (+-r_{1} - +-r_{2})*sqrt( 1+ tanTheta^{2})
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
     *                      D_{y}*D_{z} +- |R|sqrt (D_{z}^{2} + D_{y}^{2} - R^{2})
     *       tanTheta =   --------------------------------------------------------
     *                                      (D_{z}^{2} -  R^{2}) * */
    std::optional<SegmentSeed>  
        MdtSegmentSeedGenerator::buildSeed(const HoughHitType & topHit, 
                                           const HoughHitType & bottomHit, 
                                           const std::array<int,3> & signs) {
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
            ATH_MSG_VERBOSE("Mismatch between tube intercepts:  "<<Y0<<" vs. "<<Y1);
            return std::nullopt;
        }
        /// Check whether the seed has been seen before
        if (std::find_if(m_seenSolutions.begin(), m_seenSolutions.end(),[this, Y0, tanTheta]
                        (const std::array<double,2>& seen) {
                            return std::abs(seen[toInt(AxisDefs::y0)] - Y0) < m_cfg.interceptReso &&
                                   std::abs(seen[toInt(AxisDefs::tanTheta)] - tanTheta) < m_cfg.tanThetaReso;
                        }) != m_seenSolutions.end()){
            return std::nullopt;
        }
        /// Add the solution to the list. That we don't iterate twice over it
        m_seenSolutions.emplace_back();
        m_seenSolutions.back()[toInt(AxisDefs::y0)] = Y0;
        m_seenSolutions.back()[toInt(AxisDefs::tanTheta)] = tanTheta;

        SegmentFit::Parameters candidatePars = m_segmentSeed->parameters();
        candidatePars[toInt(AxisDefs::y0)] = Y0;
        candidatePars[toInt(AxisDefs::tanTheta)] = tanTheta;
            
        const auto [linePos, lineDir] = makeLine(candidatePars);
        
        HitVec assocHits{};
        unsigned int nMdt{0};
        /** Check for the best Mdt hit per each layer */
        for (const std::vector<HoughHitType>& hitsInLayer : m_mdtHitsPerLayer) {
            HoughHitType bestHit{nullptr};
            double bestChi2{m_cfg.chi2PerHit};
            for (const HoughHitType testMe : hitsInLayer){
                const double chi2 = SegmentFitHelpers::chiSqTermMdt(linePos, lineDir, testMe, msg());              
                ATH_MSG_VERBOSE("Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->positionInChamber())<<", chi2: "<<chi2);
                /// Add all hits with a chi2 better than 3
                if (chi2 < bestChi2) {
                    bestHit = testMe;
                    bestChi2 = chi2;
                }
            }
            if (!bestHit) {
                continue;
            }
            assocHits.push_back(bestHit);
            ++nMdt;
        }
        /** Reject seeds with too litle mdt hit association */
        if (nMdt < m_cfg.nMdtHitCut) {
            return std::nullopt;
        }
        /** If we found a long Mdt seed, then ensure that all
         *  subsequent seeds have at least the same amount of Mdt hits. */
        if (m_cfg.tightenHitCut) {
            m_cfg.nMdtHitCut = std::max(m_cfg.nMdtHitCut, nMdt);
        }
        /** Associate strip hits */
        for (const std::vector<HoughHitType>& hitsInLayer : m_stripHitsPerLayer) {
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
