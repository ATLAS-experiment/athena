/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NswSegmentFinderAlg.h"

#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>


#include "MuonPatternEvent/SegmentFitterEventData.h"

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
        THROW_EXCEPTION("Invalid space point for design retrieval "<<sp.msSector()->idHelperSvc()->toString(sp.identify()));
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
using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;

StatusCode NswSegmentFinderAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_etaKey.initialize());
    ATH_CHECK(m_writeSegmentKey.initialize());
    ATH_CHECK(m_writeSegmentSeedKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_visionTool.retrieve(DisableTool{m_visionTool.empty()}));

    if (!(m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC())) {
        ATH_MSG_ERROR("MM or STGC not part of initialized detector layout");
        return StatusCode::FAILURE;
    }

    SegmentLineFitter::Config fitCfg{};
    fitCfg.calibrator = m_calibTool.get();
    fitCfg.visionTool = m_visionTool.get();
    fitCfg.idHelperSvc = m_idHelperSvc.get();
    
    m_lineFitter = std::make_unique<SegmentFit::SegmentLineFitter>(name(), std::move(fitCfg));

    if(m_dumpSeedStatistics){

        m_seedCounter = std::make_unique<SeedStatistics>();
    }

    return StatusCode::SUCCESS;
}

NswSegmentFinderAlg::UsedHitMarker_t 
    NswSegmentFinderAlg::emptyBookKeeper(const HitLayVec& sortedSp) const{
        UsedHitMarker_t emptyKeeper(sortedSp.size());
        for (std::size_t l = 0; l < sortedSp.size(); ++l) {
            emptyKeeper[l].resize(sortedSp[l].size(), 0);
        }
        return emptyKeeper;
}

NswSegmentFinderAlg::StripOrient 
    NswSegmentFinderAlg::classifyStrip(const SpacePoint& sp) const{
    
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
inline NswSegmentFinderAlg::HitWindow 
    NswSegmentFinderAlg::hitFromIPCorridor(const SpacePoint& testHit, 
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
    if (usedHits[LAYER].get()[HIT_ITER] > m_maxUsed) {                 \
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

void NswSegmentFinderAlg::constructPrelimnarySeeds(const Amg::Vector3D& beamSpot,
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
        if (usedHits[0].get()[iterLay0] > m_maxUsed) {
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

NswSegmentFinderAlg::HitVec 
    NswSegmentFinderAlg::extendHits(const Amg::Vector3D& startPos, 
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
            if (usedHits[i].get().at(j) > m_maxUsed) {
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
    NswSegmentFinderAlg::buildSegmentSeed(const InitialSeed_t& initialSeed,
                                                    const AmgSymMatrix(2)& bMatrix, 
                                                    const HoughMaximum& max, 
                                                    const HitLaySpan_t& extensionLayers,
                                                    const UsedHitSpan_t& usedHits) const {
                                                        

    //we reject seeds with all clusters' sizes less than min value
    bool allValid = std::all_of(initialSeed.begin(), initialSeed.end(), [this](const auto& hit){

    if (hit->type() == xAOD::UncalibMeasType::MMClusterType) {
        const auto* mmClust = static_cast<const xAOD::MMCluster*>(hit->primaryMeasurement());
        return mmClust->stripNumbers().size() >= m_minClusSize;
    }
        
        return false;
    });

    if (!allValid) {
        ATH_MSG_VERBOSE("Seed rejection: Not all clusters meet minimum strip size");
        return nullptr; 
    }
    

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
    double tanAlpha = houghTanAlpha(direction);
    double tanBeta = houghTanBeta(direction);

    double interceptX = segPos.x();
    double interceptY = segPos.y();


    // extend the seed to the segment -- include hits from the other layers too
    auto extendedHits = extendHits(segPos, direction, extensionLayers, usedHits);
    HitVec hits{initialSeed.begin(),initialSeed.end()};
    hits.insert(hits.end(), extendedHits.begin(), extendedHits.end());
    

    return std::make_unique<SegmentSeed>(tanBeta, interceptY, tanAlpha,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}


std::unique_ptr<Segment> NswSegmentFinderAlg::fitSegmentSeed(const EventContext& ctx,
                                                                       const ActsTrk::GeometryContext& gctx, 
                                                                       const SegmentSeed* patternSeed) const{

    ATH_MSG_VERBOSE("Fit the SegmentSeed");

    //Calibration of the seed spacepoints
    CalibSpacePointVec calibratedHits = m_calibTool->calibrate(ctx, patternSeed->getHitsInMax(), 
    patternSeed->localPosition(), patternSeed->localDirection(), 0.);

    const Amg::Transform3D& locToGlob{patternSeed->msSector()->localToGlobalTrans(gctx)};
 
    auto segment = m_lineFitter->fitSegment(ctx, patternSeed, patternSeed->parameters(),
                                                locToGlob, std::move(calibratedHits));

    return segment;

}

std::pair<std::vector<std::unique_ptr<SegmentSeed>>, std::vector<std::unique_ptr<Segment>>>
NswSegmentFinderAlg::findSegmentsFromMaximum(const HoughMaximum &max, const ActsTrk::GeometryContext &gctx, const EventContext& ctx) const {
    // first sort the hits per layer from the maximum
    SpacePointPerLayerSplitter hitLayers{max.getHitsInMax()};

    const HitLayVec& stripHitsLayers{hitLayers.stripHits()};
    const std::size_t layerSize = stripHitsLayers.size();
    
    //seeds and segments containers
    std::vector<std::unique_ptr<SegmentSeed>> seeds{};
    std::vector<std::unique_ptr<Segment>> segments{};

    //counters for the number of seeds, extented seeds and segments
    unsigned int nSeeds{0}, nExtSeeds{0}, nSegments{0};


    if (layerSize < minLayers) {
        ATH_MSG_VERBOSE("Not enough layers to build a seed");
        return std::make_pair(std::move(seeds),std::move(segments));
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
        m_visionTool->visualizeBucket(ctx, *max.parentBucket(),
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

                    const HitLaySpan_t layers{stripHitsLayers[i], stripHitsLayers[j], stripHitsLayers[k], stripHitsLayers[l]};

                    //skip combination with at least one too busy layer
                    if (std::any_of(layers.begin(), layers.end(),
                    [this](const auto& layer) {
                    return layer.get().size() > m_maxClustersInLayer;
                    })) {
                        continue; // skip this combination
                    }

                    AmgSymMatrix(2) bMatrix = betaMatrix(seedHits);                   
                    if (std::abs(bMatrix.determinant()) < 1.e-6) {
                        continue;
                    }
                  ATH_MSG_DEBUG("Space point positions for seed layers: "
                                 << Amg::toString(seedHits[0]->localPosition()) << ", "
                                 << Amg::toString(seedHits[1]->localPosition()) << ", "
                                 << Amg::toString(seedHits[2]->localPosition()) << ", "
                                 << Amg::toString(seedHits[3]->localPosition()));
                  
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
                        //if the seed build is successful, try to build the segment 
                         ++nSeeds;  
                        if(seed->getHitsInMax().size() < m_minSeedHits){
                            ATH_MSG_VERBOSE("Not succesfully extended seed");
                            continue;
                           
                        }
                        ++nExtSeeds;
                        std::unique_ptr<Segment> segment = fitSegmentSeed(ctx, gctx, seed.get());
                        seeds.push_back(std::move(seed)); 
                                          

                        if (!segment) {
                            ATH_MSG_VERBOSE("Seed Rejection: Segment fit failed");
                            if(m_markHitsFromSeed){
                                //mark hits from extended seed if no succesfully led to segment
                                markHitsAsUsed(seeds.back()->getHitsInMax(), stripHitsLayers, allUsedHits, 1, false);
                            }
                            continue;
                        }

                        ++nSegments;
                        // Flag hits as used and in the window around segment   
                        HitVec segMeasSP;
                        segMeasSP.reserve(segment->measurements().size());
                        std::transform(segment->measurements().begin(),
                                        segment->measurements().end(),
                                        std::back_inserter(segMeasSP),
                                        [](const auto& m) { return m->spacePoint(); });
                            //mark hits from segment
                        markHitsAsUsed(segMeasSP, stripHitsLayers, allUsedHits, 10, true);
                        segments.push_back(std::move(segment));

                        }                        
                    }
                }
            }
        }
    }

    if(m_dumpSeedStatistics){
        m_seedCounter->addToStat(max.msSector(), nSeeds, nExtSeeds, nSegments);
    }
    
    return std::make_pair(std::move(seeds),std::move(segments));
}


void NswSegmentFinderAlg::markHitsAsUsed(const HitVec& spacePoints,
                                        const HitLayVec& allSortHits,
                                        UsedHitMarker_t& usedHitMarker, 
                                        unsigned int incr,
                                        bool markNeighborHits) const {

    SpacePointPerLayerSorter layerSorter{};
      
    for(const auto& sp : spacePoints){

        if(!sp){
            continue;
        }           

        unsigned int measLayer = layerSorter.sectorLayerNum(*sp);
        
        bool found{false};
        double spPosX = sp->primaryMeasurement()->localPosition<1>().x();

        for (std::size_t lIdx = 0; !found && lIdx < allSortHits.size(); ++lIdx) {
            const HitVec& hVec{allSortHits[lIdx]}; 
            //check if they are not in the same layer
            unsigned int hitLayer = layerSorter.sectorLayerNum(*hVec.front());
            if(hitLayer != measLayer){
                ATH_MSG_VERBOSE("Not in the same layer since measLayer = "<< measLayer << " and "<<hitLayer);
                continue;
            }
            
            for (std::size_t hIdx  = 0 ; hIdx < hVec.size(); ++hIdx) {
                //check the dY between the measurement and the hits
                
                auto testHit = hVec[hIdx];
                              
                if (testHit == sp) {
                    usedHitMarker[lIdx][hIdx] += incr;
                    found = true;   
                    if(!markNeighborHits){
                        break;
                    }                     
                }

                //if the hit not found let's see if it is too close to the segment's measurement  
                double deltaX = std::abs(testHit->primaryMeasurement()->localPosition<1>().x() - spPosX);          
                if(deltaX < m_maxdYWindow){               
                    usedHitMarker[lIdx][hIdx] += incr;
                    
                }    
            }
        }

    }
    
}

StatusCode NswSegmentFinderAlg::execute(const EventContext &ctx) const {
    // read the inputs
    const EtaHoughMaxContainer *maxima{nullptr};
    ATH_CHECK(SG::get( maxima, m_etaKey, ctx));

    const ActsTrk::GeometryContext *gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    // prepare our output collection
    SG::WriteHandle writeSegments{m_writeSegmentKey, ctx};
    ATH_CHECK(writeSegments.record(std::make_unique<SegmentContainer>()));

    SG::WriteHandle writeSegmentSeeds{m_writeSegmentSeedKey, ctx};
    ATH_CHECK(writeSegmentSeeds.record(std::make_unique<SegmentSeedContainer>()));

    // we use the information from the previous eta-hough transform
    // to get the combined hits that belong in the same maxima
    for (const HoughMaximum *max : *maxima) {

        auto [seeds, segments] = findSegmentsFromMaximum(*max, *gctx, ctx);
       
        if (msgLvl(MSG::VERBOSE)) {
            for(const auto& hitMax : max->getHitsInMax()){
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitMax->identify())<<", "
                                <<Amg::toString(hitMax->localPosition())<<", dir: "
                                <<Amg::toString(hitMax->sensorDirection()));
            }
        }

        for(auto& seed: seeds){

             if (msgLvl(MSG::VERBOSE)){
                std::stringstream sstr{};
                sstr<<"Seed tanBeta = "<<seed->tanBeta()<<", y0 = "<<seed->interceptY()
                         <<", tanAlpha = "<<seed->tanAlpha()<<", x0 = "<<seed->interceptX()<<", hits in the seed "
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

            writeSegmentSeeds->push_back(std::move(seed));

        }

        for (auto &seg : segments) {

            const Parameters pars = localSegmentPars(*gctx, *seg);

            ATH_MSG_VERBOSE("Segment parameters : "<<toString(pars));

            if (m_visionTool.isEnabled()) {          
                m_visionTool->visualizeSegment(ctx, *seg, "#phi-segment");
            }
            
            writeSegments->push_back(std::move(seg));
            
        }
    }
    
    return StatusCode::SUCCESS;
}

StatusCode NswSegmentFinderAlg::finalize(){
    
    if(m_dumpSeedStatistics){
        m_seedCounter->printTableSeedStats(msgStream());
    }
    return StatusCode::SUCCESS;
}

void NswSegmentFinderAlg::SeedStatistics::addToStat(const MuonGMR4::SpectrometerSector* msSector, unsigned int seeds, unsigned int extSeeds, unsigned int segments){
    std::unique_lock guard{m_mutex};
    SectorField key{};
    key.chIdx = msSector->chamberIndex();
    key.phi = msSector->stationPhi();
    key.eta = msSector->chambers().front()->stationEta();
    key.side = msSector->side();

    auto &entry = m_seedStat[key]; 
    entry.nSeeds    += seeds;
    entry.nExtSeeds += extSeeds;
    entry.nSegments += segments;
}

void NswSegmentFinderAlg::SeedStatistics::printTableSeedStats(MsgStream& msg) const{


   msg<<MSG::ALWAYS<<"Seed statistics per sector:"<<endmsg;
   msg<<MSG::ALWAYS<<"------------------------------------------------------------"<<endmsg;
   msg<<MSG::ALWAYS<<"| Chamber | Phi | Eta | Side |   Seeds | ExtSeeds | Segments |"<<endmsg;
   msg<<MSG::ALWAYS<<"------------------------------------------------------------"<<endmsg;

   using Muon::MuonStationIndex::ChIndex;

    for (const auto& entry : m_seedStat) {
        const auto& sector = entry.first;
        const auto& stats  = entry.second;

        
        msg<<MSG::ALWAYS << "| " << std::setw(3) << (sector.chIdx == ChIndex::EIL ? "EIL" :"EIS")
                        <<"  | " << std::setw(2) << sector.phi
                        << " | " << std::setw(3) << sector.eta
                        << " | " << std::setw(4) << (sector.side > 0 ? "A" : "C")
                        << " | " << std::setw(7) << stats.nSeeds
                        << " | " << std::setw(8) << stats.nExtSeeds
                        << " | " << std::setw(8) << stats.nSegments
                        << " |"<<endmsg;

        
    }

    msg<<MSG::ALWAYS<<"------------------------------------------------------------"<<endmsg;
 }
  

}  // namespace MuonR4
