/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CombinatorialNSWSeedFinderAlg.h"

#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>


#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"

#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

#include "TruthUtils/HepMCHelpers.h"

#include "Acts/Seeding/CombinatorialSeedSolver.hpp"

#include <ranges>
#include <format>

using namespace Acts::Experimental::CombinatorialSeedSolver;
namespace {
    inline const MuonGMR4::StripDesign& getDesign(const MuonR4::SpacePoint& sp) {
        if (sp.type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto* prd = static_cast<const xAOD::MMCluster*>(sp.primaryMeasurement());
            return prd->readoutElement()->stripLayer(prd->measurementHash()).design();
        } else if (sp.type() == xAOD::UncalibMeasType::sTgcStripType) {
            const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(sp.primaryMeasurement());
            const auto* re = prd->readoutElement();
            switch(prd->channelType()) {
                case sTgcIdHelper::Strip:
                    return re->stripDesign(prd->measurementHash());
                case sTgcIdHelper::Wire:
                    return re->wireDesign(prd->measurementHash());
                case sTgcIdHelper::Pad:
                    return re->padDesign(prd->measurementHash());
            }
        }
        THROW_EXCEPTION("Invalid space point for design retrival "<<sp.msSector()->idHelperSvc()->toString(sp.identify()));
    }
    inline double stripHalfLength(const MuonR4::SpacePoint& sp) {
        const auto& design = getDesign(sp);
        if (sp.type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto* prd = static_cast<const xAOD::MMCluster*>(sp.primaryMeasurement());
            return 0.5* design.stripLength(prd->channelNumber());
        }
        return 0.;
    }
}

namespace MuonR4 {

using namespace SegmentFit;
constexpr unsigned int minLayers{4};

StatusCode CombinatorialNSWSeedFinderAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_etaKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_visionTool.retrieve(DisableTool{m_visionTool.empty()}));

    if (!(m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC())) {
        ATH_MSG_ERROR("MM or STGC not part of initialized detector layout");
        return StatusCode::FAILURE;
    }
   
    return StatusCode::SUCCESS;
}

CombinatorialNSWSeedFinderAlg::UsedHitMarker_t 
    CombinatorialNSWSeedFinderAlg::emptyBookKeeper(const HitLayVec& sortedSp) const{
        UsedHitMarker_t emptyKeeper(sortedSp.size());
        for (std::size_t l = 0; l < sortedSp.size(); ++l) {
            emptyKeeper[l].resize(sortedSp[l].size(), 0);
        }
        return emptyKeeper;
}

CombinatorialNSWSeedFinderAlg::StripOrient 
    CombinatorialNSWSeedFinderAlg::classifyStrip(const SpacePoint& sp) const{
    
    if (sp.type() == xAOD::UncalibMeasType::MMClusterType) {
        const auto& design = getDesign(sp);
        if (!design.hasStereoAngle()) {
            return StripOrient::X;
        } 
        return design.stereoAngle() > 0. ? StripOrient::U : StripOrient::V;
    } else if (sp.type() == xAOD::UncalibMeasType::sTgcStripType) {
        const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(sp.primaryMeasurement());
        if (sp.dimension() == 2) {
            return StripOrient::C;
        }
        return prd->channelType() == sTgcIdHelper::Strip ? StripOrient::X : StripOrient::P;
         
    }
    ATH_MSG_WARNING("Cannot classify orientation of "<<m_idHelperSvc->toString(sp.identify()));
    return StripOrient::Unknown;
}
inline CombinatorialNSWSeedFinderAlg::HitWindow 
    CombinatorialNSWSeedFinderAlg::hitFromIPCorridor(const SpacePoint& testHit, 
                                                     const Amg::Vector3D& beamSpotPos, 
                                                     const Amg::Vector3D& dirEstUp,
                                                     const Amg::Vector3D& dirEstDn) const{

    const Amg::Vector3D estPlaneArrivalUp = SeedingAux::extrapolateToPlane(beamSpotPos, dirEstUp, testHit);
    const Amg::Vector3D estPlaneArrivalDn = SeedingAux::extrapolateToPlane(beamSpotPos, dirEstDn, testHit); 

    bool below{true}, above{true};
    switch (classifyStrip(testHit)) {
        using enum StripOrient;
        case U:
        case V:{
            const double halfLength = 0.5* stripHalfLength(testHit);
            /// Calculate the strip edges
            const Amg::Vector3D leftEdge  = testHit.localPosition() - halfLength * testHit.sensorDirection();
            const Amg::Vector3D rightEdge = testHit.localPosition() + halfLength * testHit.sensorDirection();

            /// Check whether the both edges are below the lower estimated muon arrival
            below = estPlaneArrivalDn.y() > std::max(leftEdge.y(), rightEdge.y());
            /// Analogous check for the upper edge
            above = estPlaneArrivalUp.y() < std::min(leftEdge.y(), rightEdge.y());
            break; 
        } case X:
          case C: {
            /// No extrapolation needed
            const double hY = testHit.localPosition().y();
            below = estPlaneArrivalDn.y() > hY;
            /// Analogous check for the upper edge
            above = estPlaneArrivalUp.y() < hY;
            break;
        }
        case P:{
            break;
        }
        case Unknown:{
            break;
        }

    }
    ATH_MSG_VERBOSE("Hit " << m_idHelperSvc->toString(testHit.identify())
                << (below || above ? " is outside the window" : " is inside the window"));
    if(below) {
        return HitWindow::tooLow;
    }
    if(above) {
        return HitWindow::tooHigh;
    }
    return HitWindow::inside;
};

/// Macro to check whether a hit is compatible with the hit corridor
#define TEST_HIT_CORRIDOR(LAYER, HIT_ITER, START_LAYER)               \
{                                                                     \
    const SpacePoint* testMe = combinatoricLayers[LAYER].get()[HIT_ITER]; \
    if (usedHits[LAYER].get()[HIT_ITER]) {                 \
        ATH_MSG_VERBOSE(__func__<<":"<<__LINE__<<" - "     \
            <<m_idHelperSvc->toString(testMe->identify())  \
            <<" already used in good seed." );             \
        continue;                                          \
    }                                                      \
    const HitWindow inWindow = hitFromIPCorridor(*testMe, beamSpot, dirEstUp, dirEstDn); \
    if(inWindow == HitWindow::tooHigh) {                       \
        ATH_MSG_VERBOSE(__func__<<":"<<__LINE__<<" - Hit "     \
            <<m_idHelperSvc->toString(testMe->identify())      \
            <<" is beyond the corridor. Break loop");          \
        break;                                                 \
    } else if (inWindow == HitWindow::tooLow) {                \
        START_LAYER =  HIT_ITER + 1;                           \
        ATH_MSG_VERBOSE(__func__<<":"<<__LINE__<<" - Hit "     \
            <<m_idHelperSvc->toString(testMe->identify())      \
            <<" is still below the corridor. Update start to " \
             <<START_LAYER);                                   \
        continue;                                              \
    }                                                          \
}

void CombinatorialNSWSeedFinderAlg::constructPrelimnarySeeds(const Amg::Vector3D& beamSpot,
                                                             const HitLaySpan_t& combinatoricLayers,
                                                             const UsedHitSpan_t& usedHits,
                                                             InitialSeedVec_t& seedHitsFromLayers) const {
    /// Assign enough memory to the vector
    seedHitsFromLayers.clear();
    std::size_t maxSize{1};
    for (const HitVec& hitVec : combinatoricLayers) {
        maxSize = maxSize * hitVec.size();
    }
    seedHitsFromLayers.reserve(maxSize);

    
    unsigned int iterLay0{0}, iterLay1{0}, iterLay2{0}, iterLay3{0};    
    unsigned int startLay1{0}, startLay2{0}, startLay3{0};
    
    for( ; iterLay0 <  combinatoricLayers[0].get().size() ; ++iterLay0){
        /// The hit is alrady in a good seed. Don't consider again
        if (usedHits[0].get()[iterLay0]) {
            continue;
        }
        const SpacePoint* hit0 = combinatoricLayers[0].get()[iterLay0];
        /// Construct the beamspot to first hit connection to guestimate the angle
        const Amg::Vector3D initSeedDir{(beamSpot - hit0->localPosition()).unit()};
        const Amg::Vector3D dirEstUp = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() - m_windowTheta); 
        const Amg::Vector3D dirEstDn = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() + m_windowTheta); 

        ATH_MSG_VERBOSE("Reference hit: "<<m_idHelperSvc->toString(hit0->identify())
                      <<", position: "<<Amg::toString(hit0->localPosition())
                      <<", seed dir: "<<Amg::toString(initSeedDir)
                      <<", seed plane: "<<Amg::toString(SeedingAux::extrapolateToPlane(beamSpot, initSeedDir, *hit0)));
        /** Apply cut window on theta of the seed. */
        for( iterLay1 = startLay1; iterLay1 <  combinatoricLayers[1].get().size() ; ++iterLay1){
            TEST_HIT_CORRIDOR(1, iterLay1, startLay1);
            for( iterLay2 = startLay2; iterLay2 < combinatoricLayers[2].get().size() ; ++iterLay2){
                TEST_HIT_CORRIDOR(2, iterLay2, startLay2);
                for( iterLay3 = startLay3; iterLay3 < combinatoricLayers[3].get().size(); ++iterLay3){
                    TEST_HIT_CORRIDOR(3, iterLay3, startLay3);
                    seedHitsFromLayers.emplace_back(std::array{hit0, combinatoricLayers[1].get()[iterLay1], 
                                                           combinatoricLayers[2].get()[iterLay2],
                                                           combinatoricLayers[3].get()[iterLay3]}); 
                }
            }
        }
    }
}
#undef TEST_HIT_CORRIDOR

CombinatorialNSWSeedFinderAlg::HitVec 
    CombinatorialNSWSeedFinderAlg::extendHits(const Amg::Vector3D& startPos, 
                                              const Amg::Vector3D& direction, 
                                              const HitLaySpan_t& extensionLayers,
                                              const UsedHitSpan_t& usedHits) const {
    
    //the hits we need to return to extend the segment seed
    HitVec combinatoricHits;

    //the stripHitsLayers are already the unused ones - only use for the extension
    for (std::size_t i = 0; i < extensionLayers.size(); ++i) {
        const HitVec& layer{extensionLayers[i].get()};
        const Amg::Vector3D extrapPos = SeedingAux::extrapolateToPlane(startPos, direction, *layer.front());

        unsigned int indexOfHit = layer.size() + 1;
        unsigned int triedHit{0};
        double minPull{std::numeric_limits<double>::max()};
       
        // loop over the hits on the same layer
        for (unsigned int j = 0; j < layer.size(); ++j) {
            if (usedHits[i].get().at(j)) {
                continue;
            }
            auto hit = layer.at(j);
            const double pull = std::sqrt(SeedingAux::chi2Term(extrapPos, direction, *hit));
            ATH_MSG_VERBOSE("Trying extension with hit " << m_idHelperSvc->toString(hit->identify()));
           
            //find the hit with the minimum pull (check at least one hit after we have increasing pulls)
            if (pull > minPull) {
                triedHit+=1;  
                continue;                 
            }

            if(triedHit>1){
                break;
            }

            indexOfHit = j;
            minPull = pull;
        }

        // complete the seed with the extended hits
        if (minPull < m_minPullThreshold) {
            const auto* bestCand = layer.at(indexOfHit);
            ATH_MSG_VERBOSE("Extension successfull - hit" << m_idHelperSvc->toString(bestCand->identify())
                          <<", pos: "<<Amg::toString(bestCand->localPosition())
                          <<", dir: "<<Amg::toString(bestCand->sensorDirection())<<" found with pull "<<minPull);
            combinatoricHits.push_back(bestCand);
        }
    }
    return combinatoricHits;
}

std::unique_ptr<SegmentSeed> 
    CombinatorialNSWSeedFinderAlg::buildSegmentSeed(const InitialSeed_t& initialSeed,
                                                    const AmgSymMatrix(2)& bMatrix, 
                                                    const HoughMaximum& max, 
                                                    const HitLaySpan_t& extensionLayers,
                                                    const UsedHitSpan_t& usedHits) const {

    std::array<double, 4> params = defineParameters(bMatrix, initialSeed);

    const auto [segPos, direction] = seedSolution(initialSeed, params);

    // check the consistency of the parameters - expected to lay in the strip's
    // length
    for (std::size_t i = 0; i < 4; ++i) {
        const double halfLength = stripHalfLength(*initialSeed[i]);
        
        if (std::abs(params[i]) > halfLength) {
            ATH_MSG_VERBOSE("Seed Rejection: Invalid seed - outside of the strip's length");
            return nullptr;
        }
    }
    double tanPhi = houghTanPhi(direction);
    double tanTheta = houghTanTheta(direction);

    double interceptX = segPos.x();
    double interceptY = segPos.y();


    // extend the seed to the segment -- include hits from the other layers too
    auto extendedHits = extendHits(segPos, direction, extensionLayers, usedHits);
    HitVec hits{initialSeed.begin(),initialSeed.end()};
    hits.insert(hits.end(), extendedHits.begin(), extendedHits.end());
    return std::make_unique<SegmentSeed>(tanTheta, interceptY, tanPhi,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}


std::vector<std::unique_ptr<SegmentSeed>>
CombinatorialNSWSeedFinderAlg::findSeedsFromMaximum(const HoughMaximum &max, const ActsGeometryContext &gctx) const {
    // first sort the hits per layer from the maximum
    SpacePointPerLayerSplitter hitLayers{max.getHitsInMax()};

    const HitLayVec& stripHitsLayers{hitLayers.stripHits()};
    const std::size_t layerSize = stripHitsLayers.size();
    
    std::vector<std::unique_ptr<SegmentSeed>> seeds{};


    if (layerSize < minLayers) {
        ATH_MSG_VERBOSE("Not enough layers to build a seed");
        return seeds;
    }


    if (m_visionTool.isEnabled()) {
        MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{};
        const auto truthHits = getMatchingSimHits(max.getHitsInMax());
        constexpr double legX{0.2};
        double legY{0.8};
        for (const SpacePoint* sp : max.getHitsInMax()) {
            const auto* mmClust = static_cast<const xAOD::MMCluster*>(sp->primaryMeasurement());
            const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*mmClust);
            if (!simHit || !MC::isMuon(simHit)) continue;
            const MuonGMR4::MmReadoutElement* reEle = mmClust->readoutElement();
            const MuonGMR4::StripDesign& design = reEle->stripLayer(mmClust->measurementHash()).design();
            const Amg::Transform3D toChamb = reEle->msSector()->globalToLocalTrans(gctx) * 
                                             reEle->localToGlobalTrans(gctx, simHit->identify());
            const Amg::Vector3D hitPos = toChamb * xAOD::toEigen(simHit->localPosition());
            const Amg::Vector3D hitDir = toChamb.linear() * xAOD::toEigen(simHit->localDirection());
            const double pull = std::sqrt(SeedingAux::chi2Term(hitPos, hitDir, *sp));
            const double pull2 = (mmClust->localPosition<1>().x() - simHit->localPosition().x()) / std::sqrt(mmClust->localCovariance<1>().x());
            primitives.push_back(MuonValR4::drawLabel(std::format("ml: {:1d}, gap: {:1d}, {:}, pull: {:.2f} / {:.2f}", reEle->multilayer(), mmClust->gasGap(), 
                                !design.hasStereoAngle() ? "X" : design.stereoAngle() >0 ? "U": "V",pull, pull2),legX,legY,14));
            legY-=0.05;           
        }
        m_visionTool->visualizeBucket(Gaudi::Hive::currentContext(), *max.parentBucket(),
                                      "truth", std::move(primitives));
    }

    UsedHitMarker_t allUsedHits = emptyBookKeeper(stripHitsLayers);
    const Amg::Transform3D globToLocal = max.msSector()->globalToLocalTrans(gctx);
    std::array<const SpacePoint*, 4> seedHits{};

    InitialSeedVec_t preLimSeeds{};

    for (std::size_t i = 0; i < layerSize - 3; ++i) {
        seedHits[0] = stripHitsLayers[i].front();
        for (std::size_t j = i + 1; j < layerSize - 2; ++j) {
            seedHits[1] = stripHitsLayers[j].front();
            for (std::size_t k = j + 1; k < layerSize - 1; ++k) {
                seedHits[2] = stripHitsLayers[k].front();
                for (std::size_t l = k + 1; l < layerSize; ++l) {
                    seedHits[3] = stripHitsLayers[l].front();
                    AmgSymMatrix(2) bMatrix = betaMatrix(seedHits);                   
                    if (std::abs(bMatrix.determinant()) < 1.e-6) {
                        continue;
                    }
                  
                    const HitLaySpan_t layers{stripHitsLayers[i], stripHitsLayers[j], stripHitsLayers[k], stripHitsLayers[l]};
                    UsedHitSpan_t usedHits{allUsedHits[i], allUsedHits[j], allUsedHits[k], allUsedHits[l]};    
                    // each layer may have more than one hit - take the hit combinations                    
                    constructPrelimnarySeeds(globToLocal.translation(), layers, usedHits, preLimSeeds);

                     //the layers not participated in the seed build - gonna be used for the extension  
                    HitLaySpan_t extensionLayers{};
                    UsedHitSpan_t usedExtensionHits{};
                    usedExtensionHits.reserve(stripHitsLayers.size());
                    extensionLayers.reserve(stripHitsLayers.size());
                    for (std::size_t e = 0 ; e < stripHitsLayers.size(); ++e) {
                        if (!(e == i || e == j || e == k || e == l)){
                            extensionLayers.emplace_back(stripHitsLayers[e]);
                            usedExtensionHits.emplace_back(allUsedHits[e]);
                        }
                    }
                    // we have made sure to have hits from all the four layers -
                    // start by 4 hits for the seed and try to build the seed for the combinatorics found
                    for (auto &combinatoricHits : preLimSeeds) {
                        auto seed = buildSegmentSeed(combinatoricHits, bMatrix, max, extensionLayers, usedExtensionHits);
                        if (seed) {
                            markHitsAsUsed(*seed,stripHitsLayers, allUsedHits);
                            seeds.push_back(std::move(seed));
                        }
                    }
                }
            }
        }
    }

    return seeds;
}
void CombinatorialNSWSeedFinderAlg::markHitsAsUsed(const SegmentSeed& seed,
                                                   const HitLayVec& allSortHits,
                                                   UsedHitMarker_t& usedHitMarker) const {
    /// That's ultra slow & should be revised
    for (const auto* sp : seed.getHitsInMax()) {
        bool found{false};
        for (std::size_t lIdx = 0; !found && lIdx < allSortHits.size(); ++lIdx) {
            const HitVec& hVec{allSortHits[lIdx]}; 
            for (std::size_t hIdx  = 0 ; hIdx < hVec.size(); ++hIdx) {
                if (hVec[hIdx] == sp) {
                    usedHitMarker[lIdx][hIdx] = true;
                    found = true;
                    break;
                }
            }
        }
    }
}
StatusCode CombinatorialNSWSeedFinderAlg::execute(const EventContext &ctx) const {
    // read the inputs
    const EtaHoughMaxContainer *maxima{nullptr};
    ATH_CHECK(SG::get( maxima, m_etaKey, ctx));

    const ActsGeometryContext *gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    // prepare our output collection
    SG::WriteHandle writeMaxima{m_writeKey, ctx};
    ATH_CHECK(writeMaxima.record(std::make_unique<SegmentSeedContainer>()));

    // we use the information from the previous eta-hough transform
    // to get the combined hits that belong in the same maxima
    for (const HoughMaximum *max : *maxima) {
        std::vector<std::unique_ptr<SegmentSeed>> seeds = findSeedsFromMaximum(*max, *gctx);
        if (msgLvl(MSG::VERBOSE)) {
            for(const auto& hitMax : max->getHitsInMax()){
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitMax->identify())<<", "
                                <<Amg::toString(hitMax->localPosition())<<", dir: "
                                <<Amg::toString(hitMax->sensorDirection()));
            }
        }
        for (auto &seed : seeds) {
            if (msgLvl(MSG::VERBOSE)){
                std::stringstream sstr{};
                sstr<<"Seed tanTheta = "<<seed->tanTheta()<<", y0 = "<<seed->interceptY()
                         <<", tanPhi = "<<seed->tanPhi()<<", x0 = "<<seed->interceptX()<<", hits in the seed "
                         <<seed->getHitsInMax().size()<<std::endl;
        
                for(const auto& hit : seed->getHitsInMax()){
                    sstr<<" *** Hit "<<m_idHelperSvc->toString(hit->identify())<<", "
                        << Amg::toString(hit->localPosition())<<", dir: "<<Amg::toString(hit->sensorDirection())<<std::endl;
                }
                ATH_MSG_VERBOSE(sstr.str());
            }
            if (m_visionTool.isEnabled()) {          
                m_visionTool->visualizeSeed(ctx, *seed, "#phi-combinatorialSeed");
            }
            writeMaxima->push_back(std::move(seed));
        }
    }
    return StatusCode::SUCCESS;
}

}  // namespace MuonR4
