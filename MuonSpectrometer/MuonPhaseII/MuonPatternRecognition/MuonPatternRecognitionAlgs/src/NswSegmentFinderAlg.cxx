/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NswSegmentFinderAlg.h"

#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"


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
        } else{
            const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(sp.primaryMeasurement());
            if(prd->channelType() == sTgcIdHelper::Pad){        
                const auto& padDesign = static_cast<const MuonGMR4::PadDesign&>(design);
                auto padCorners = padDesign.padCorners(prd->channelNumber());          
                return 0.5* std::abs(padCorners[0].x() - padCorners[1].x());
            }
            return 0.5* design.stripLength(prd->channelNumber());
        }
    
        return 0.;
    }
    inline std::string sTgcChannelType(const int chType) {
        return chType == sTgcIdHelper::Strip ? "S" :
               chType == sTgcIdHelper::Wire ? "W" : "P";
    }
}

namespace MuonR4 {

using namespace SegmentFit;
constexpr unsigned minLayers{4};

using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;

StatusCode NswSegmentFinderAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_etaKey.initialize());
    ATH_CHECK(m_writeSegmentKey.initialize());
    ATH_CHECK(m_writeSegmentSeedKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_visionTool.retrieve(DisableTool{m_visionTool.empty()}));
    ATH_CHECK(detStore()->retrieve(m_detMgr));

    if (!(m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC())) {
        ATH_MSG_ERROR("MM or STGC not part of initialized detector layout");
        return StatusCode::FAILURE;
    }

    SegmentLineFitter::Config fitCfg{};
    fitCfg.calibrator = m_calibTool.get();
    fitCfg.visionTool = m_visionTool.get();
    fitCfg.calcAlongStrip = false;
    fitCfg.idHelperSvc = m_idHelperSvc.get();
    fitCfg.parsToUse = {ParamDefs::x0, ParamDefs::y0, ParamDefs::theta, ParamDefs::phi};
    
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
        //check if we have strip only or wire only measurements
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

void NswSegmentFinderAlg::constructPreliminarySeeds(const Amg::Vector3D& beamSpot,
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

    unsigned iterLay0{0}, iterLay1{0}, iterLay2{0}, iterLay3{0};    
    unsigned startLay1{0}, startLay2{0}, startLay3{0};
    
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

    for (std::size_t i = 0; i < extensionLayers.size(); ++i) {
        const HitVec& layer{extensionLayers[i].get()};
        const Amg::Vector3D extrapPos = SeedingAux::extrapolateToPlane(startPos, direction, *layer.front());

        unsigned indexOfHit = layer.size() + 1;
        unsigned triedHit{0};
        double minPull{std::numeric_limits<double>::max()};
       
        // loop over the hits on the same layer
        for (unsigned j = 0; j < layer.size(); ++j) {
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
    NswSegmentFinderAlg::constructCombinatorialSeed(const InitialSeed_t& initialSeed,
                                                    const AmgSymMatrix(2)& bMatrix,
                                                    const HoughMaximum& max,
                                                    const HitLaySpan_t& extensionLayers,
                                                    const UsedHitSpan_t& usedHits) const {
    bool allValid = std::any_of(initialSeed.begin(), initialSeed.end(), 
                                [this](const auto& hit){
        if (hit->type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto* mmClust = static_cast<const xAOD::MMCluster*>(hit->primaryMeasurement());
            return mmClust->stripNumbers().size() >= m_minClusSize;
        }
        return true;
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
            ATH_MSG_VERBOSE("Seed Rejection: Invalid seed - outside of the strip's length "<< m_idHelperSvc->toString(initialSeed[i]->identify())
                          <<", param: "<<params[i]<<", halfLength: "<<halfLength);
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
    std::ranges::move(extendedHits, std::back_inserter(hits));

    return std::make_unique<SegmentSeed>(tanBeta, interceptY, tanAlpha,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}


std::unique_ptr<Segment> NswSegmentFinderAlg::fitSegmentSeed(const EventContext& ctx,
                                                             const ActsTrk::GeometryContext& gctx, 
                                                             const SegmentSeed* patternSeed) const{
    
    if(patternSeed->getHitsInMax().size() < m_minSeedHits){
        ATH_MSG_VERBOSE("Not enough hits in the SegmentSeed to fit a segment");
        return nullptr;    
    }

    ATH_MSG_VERBOSE("Fit the SegmentSeed");

    //Calibration of the seed spacepoints
    CalibSpacePointVec calibratedHits = m_calibTool->calibrate(ctx, patternSeed->getHitsInMax(), 
                                                               patternSeed->localPosition(), 
                                                               patternSeed->localDirection(), 0.);

    const Amg::Transform3D& locToGlob{patternSeed->msSector()->localToGlobalTransform(gctx)};
 
    return m_lineFitter->fitSegment(ctx, patternSeed, patternSeed->parameters(),
                                                locToGlob, std::move(calibratedHits));
}

void NswSegmentFinderAlg::processSegment(std::unique_ptr<Segment> segment, 
                                         const HitVec& seedHits, 
                                         const HitLayVec& hitLayers, 
                                         UsedHitMarker_t& usedHits,
                                         SegmentVec_t& segments) const {
    
    if (!segment) {
        ATH_MSG_VERBOSE("Seed Rejection: Segment fit failed");
       
        if (m_markHitsFromSeed && seedHits.size() > m_minSeedHits) {
            // Mark hits from extended seed (used increment by 1)
            markHitsAsUsed(seedHits, hitLayers, usedHits, 1, false);
        }
        return;
    }

    // -------- success path --------
    ATH_MSG_DEBUG("Segment built with "
                  << segment->measurements().size()
                  << " hits, chi2/ndof: "
                  << segment->chi2() / std::max(1u,segment->nDoF()));

    HitVec segMeasSP;
    segMeasSP.reserve(segment->measurements().size());

    std::ranges::transform(
        segment->measurements(),
        std::back_inserter(segMeasSP),
        [](const auto& m) { return m->spacePoint(); }
    );

    // Mark segment hits as fully used (used increment by 10,
    // hits are effectively removed)
    markHitsAsUsed(segMeasSP, hitLayers, usedHits, 10, true);
    segments.push_back(std::move(segment));

}

std::pair<NswSegmentFinderAlg::SegmentSeedVec_t, NswSegmentFinderAlg::SegmentVec_t>
NswSegmentFinderAlg::buildSegmentsFromSTGC(const EventContext& ctx,
                                           const ActsTrk::GeometryContext &gctx, 
                                           const HitLayVec& hitLayers,                                          
                                           const HoughMaximum& max,
                                           const Amg::Vector3D& beamSpotPos,
                                           UsedHitMarker_t& usedHits) const {

    //go through the  layers and build seeds from the combinations of hits 
    //starting from the outermost layers with 2D measurements (excluding pads)
    SegmentSeedVec_t seeds{};
    SegmentVec_t segments{};
    std::size_t layerSize = hitLayers.size();
    double thetaWindowCut{std::cos(2*m_windowTheta)};

    // lamda helper to check if the spacepoint is combined (but not pad) and unused in an already constructed seed
    auto isUnusedCombined = [&](std::size_t layIdx, std::size_t hitIdx) -> bool {
        const SpacePoint* sp = hitLayers[layIdx][hitIdx];
        if (sp->type() != xAOD::UncalibMeasType::sTgcStripType) {
            THROW_EXCEPTION("Space point is not of sTgc type: "<<sp->msSector()->idHelperSvc()->toString(sp->identify()));
        }
        const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(sp->primaryMeasurement());
        bool isCombined = sTgcChannelType(prd->channelType()) == "S" && sp->dimension() == 2;
        bool isUnused = usedHits[layIdx].at(hitIdx) <= m_maxUsed;
        return isCombined && isUnused;      
   
    };

    // find the 2D measurements from the outermost layers - even move one layer inside 
    for(std::size_t layIdx1 = 0; layIdx1 < 2; ++layIdx1){        
        for(std::size_t layIdx2 = layerSize-1; layIdx2 >= layerSize-2; --layIdx2){
            //in case of MM layers we stop - the layers are sorted in Z
            if(hitLayers[layIdx1].front()->type() == xAOD::UncalibMeasType::MMClusterType  || 
               hitLayers[layIdx2].front()->type() == xAOD::UncalibMeasType::MMClusterType){
                ATH_MSG_VERBOSE("Outermost layers are MM - stop searching for sTgc Measurements");
                return std::make_pair(std::move(seeds), std::move(segments));
            }

            //check if we have 2D measurements on these layers that are unused (excluding the pads)
            for(std::size_t hitIdx1 = 0; hitIdx1 < hitLayers[layIdx1].size(); ++hitIdx1) {
                const SpacePoint* hit1 = hitLayers[layIdx1][hitIdx1];
                if(!isUnusedCombined(layIdx1, hitIdx1)){
                    continue;
                }
                for(std::size_t hitIdx2 = 0; hitIdx2 < hitLayers[layIdx2].size(); ++hitIdx2) {
                    const SpacePoint* hit2 = hitLayers[layIdx2][hitIdx2];
                    if(!isUnusedCombined(layIdx2, hitIdx2)){
                        continue;
                    }
                     //test if this selection of the hits from the two layers is aligned with the beam spot 
                    const Amg::Vector3D beamSpotHitDir{(beamSpotPos - hit1->localPosition()).unit()};
                    const Amg::Vector3D seedDir{(hit2->localPosition() - hit1->localPosition()).unit()};                 
                    const double cosAngle = beamSpotHitDir.dot(seedDir);
                    //accept the deflection of direction with a tolerance of 1 deg                    
                    if(std::abs(cosAngle) < thetaWindowCut){                        
                        continue;                        
                    }                   
                    //found 2D hits on the outermost layers - build a seed
                    HitVec seedHits{hit1, hit2};
                    //get the seed direction and position from the two 2D hits
                    const Amg::Vector3D seedPos = hit1->localPosition();
                     //express position in z=0
                    const Amg::Vector3D seedPosZ0 = seedPos + Amg::intersect<3>(seedPos, seedDir, Amg::Vector3D::UnitZ(), 0.).value_or(0.)*seedDir;                 
                    // extend the seed to the other layers
                    HitLaySpan_t extensionLayers{};
                    UsedHitSpan_t usedExtensionHits{};
                    extensionLayers.reserve(hitLayers.size());
                    usedExtensionHits.reserve(hitLayers.size());
                    for (std::size_t e = 0 ; e < hitLayers.size(); ++e) {
                        if (!(e == layIdx1 || e == layIdx2)){                            
                            extensionLayers.emplace_back(hitLayers[e]);
                            usedExtensionHits.emplace_back(usedHits[e]);    
                        }
                    }
                    auto extendedHits = extendHits(seedPosZ0, seedDir, extensionLayers, usedExtensionHits);
                    std::ranges::move(extendedHits, std::back_inserter(seedHits));

                    if(seedHits.size() < minLayers){
                        continue;                    
                    }

                    //make seed 
                    auto seed = std::make_unique<SegmentSeed>(houghTanBeta(seedDir), seedPosZ0.y(),
                                                         houghTanAlpha(seedDir), seedPosZ0.x(),
                                                         seedHits.size(), std::move(seedHits),
                                                         max.parentBucket());
                    //fit the segment seed
                    std::unique_ptr<Segment> segment = fitSegmentSeed(ctx, gctx, seed.get());  
                    processSegment(std::move(segment), seed->getHitsInMax(), hitLayers, usedHits, segments);
                    seeds.push_back(std::move(seed));

                }               
            }
        }
    }
    return std::make_pair(std::move(seeds),std::move(segments));
}

std::pair<NswSegmentFinderAlg::SegmentSeedVec_t, NswSegmentFinderAlg::SegmentVec_t>
NswSegmentFinderAlg::buildSegmentsFromMM(const EventContext& ctx,
                                         const ActsTrk::GeometryContext &gctx,
                                         const HitLayVec& hitLayers,                                  
                                         const HoughMaximum& max,
                                         const Amg::Vector3D& beamSpotPos,
                                         UsedHitMarker_t& usedHits,
                                         bool useOnlyMM) const {
    
    //go through the layers and build seeds from the combinations of hits
    SegmentSeedVec_t seeds{};
    SegmentVec_t segments{};
    std::size_t layerSize = hitLayers.size();

    if(layerSize < minLayers){
        ATH_MSG_VERBOSE("Not enough layers to build a seed");
        return {std::move(seeds), std::move(segments)};
    }

    //lamda helper to find the first unused strip hit on the layer
    auto unusedStripHit = [&](const HitVec& layerHits, unsigned int layIdx) -> const SpacePoint* {
        //in case of MM only combinatorial seeding - we consider only MM strip hits for seeding
        bool isMM = useOnlyMM ? layerHits.front()->type() == xAOD::UncalibMeasType::MMClusterType : true;
        for (auto [idx, hit] : Acts::enumerate(layerHits)) {
            auto spOrient = classifyStrip(*hit);
            bool isStrip = spOrient == StripOrient::X || spOrient == StripOrient::U || spOrient == StripOrient::V;
            bool isUnused = usedHits[layIdx].at(idx) <= m_maxUsed;           
            if (isStrip && isUnused && isMM) {
                return hit;
            }
        }
        return nullptr;       
    };

    std::array<const SpacePoint*, 4> seedHits{};
    InitialSeedVec_t preLimSeeds{};

    for (std::size_t i = 0; i < layerSize - 3; ++i) {
        seedHits[0] = unusedStripHit(hitLayers[i], i);
        if(!seedHits[0]) {
            continue;
        }
        for (std::size_t j = i + 1; j < layerSize - 2; ++j) {
            seedHits[1] = unusedStripHit(hitLayers[j], j);
            if(!seedHits[1]){
                continue;
            }
            for (std::size_t k = j + 1; k < layerSize - 1; ++k) {
                seedHits[2] = unusedStripHit(hitLayers[k], k);
                if(!seedHits[2]){
                    continue;
                }
                for (std::size_t l = k + 1; l < layerSize; ++l) {
                    seedHits[3] = unusedStripHit(hitLayers[l], l);
                    if(!seedHits[3]){
                        continue;
                    }

                    const HitLaySpan_t layers{hitLayers[i], hitLayers[j], hitLayers[k], hitLayers[l]};
                    //skip combination with at least one too busy layer
                    bool tooBusy = std::ranges::any_of(layers,
                                            [this](const auto& layer) {
                                                return layer.get().size() > m_maxClustersInLayer;
                                            });
                    if (tooBusy) {
                        continue; // skip this combination
                    }
                    
                    AmgSymMatrix(2) bMatrix = betaMatrix(seedHits);   
       
                    if (std::abs(bMatrix.determinant()) < 1.e-6) {
                        continue;
                    }
                    ATH_MSG_DEBUG("Space point positions for seed layers: \n"
                                 <<(*seedHits[0]) << ",\n"
                                 <<(*seedHits[1]) << ",\n"
                                 <<(*seedHits[2]) << ",\n"
                                 <<(*seedHits[3]));  
    
                  
                    UsedHitSpan_t usedHitsSpan{usedHits[i], usedHits[j], usedHits[k], usedHits[l]};    
                    // each layer may have more than one hit - take the hit combinations                    
                    constructPreliminarySeeds(beamSpotPos, layers, usedHitsSpan, preLimSeeds);

                     //the layers not participated in the seed build - gonna be used for the extension  
                    HitLaySpan_t extensionLayers{};
                    UsedHitSpan_t usedExtensionHits{};
                    usedExtensionHits.reserve(hitLayers.size());
                    extensionLayers.reserve(hitLayers.size());
                    for (std::size_t e = 0 ; e < hitLayers.size(); ++e) {
                        if (!(e == i || e == j || e == k || e == l)){
                            extensionLayers.emplace_back(hitLayers[e]);
                            usedExtensionHits.emplace_back(usedHits[e]);
                        }
                    }
                    // we have made sure to have hits from all the four layers -
                    // start by 4 hits for the seed and try to build the extended seed for the combinatorics found
                    for (auto &combinatoricHits : preLimSeeds) {
                        auto seed = constructCombinatorialSeed(combinatoricHits, bMatrix, max, extensionLayers, usedExtensionHits);
                        if (!seed) {                            
                            continue;
                        }
                                             
                        std::unique_ptr<Segment> segment = fitSegmentSeed(ctx, gctx, seed.get());
                        processSegment(std::move(segment), seed->getHitsInMax(), hitLayers, usedHits, segments);
                        seeds.push_back(std::move(seed));                                    
              
                    }
                }
            }
        }
    }
    return std::make_pair(std::move(seeds),std::move(segments));
}

std::pair<NswSegmentFinderAlg::SegmentSeedVec_t, NswSegmentFinderAlg::SegmentVec_t>
NswSegmentFinderAlg::findSegmentsFromMaximum(const HoughMaximum &max, 
                                             const ActsTrk::GeometryContext &gctx, 
                                             const EventContext& ctx) const {
    // first sort the hits per layer from the maximum
    SpacePointPerLayerSplitter hitLayers{max.getHitsInMax()};

    const HitLayVec& stripHitsLayers{hitLayers.stripHits()};
    const std::size_t layerSize = stripHitsLayers.size();
    
    //seeds and segments containers
    SegmentSeedVec_t seeds{};
    SegmentVec_t segments{};

    const Amg::Transform3D globToLocal = max.msSector()->globalToLocalTransform(gctx);
    //counters for the number of seeds, extented seeds and segments

    if (layerSize < minLayers) {
        ATH_MSG_VERBOSE("Not enough layers to build a seed");
        return std::make_pair(std::move(seeds),std::move(segments));
    }

    if (m_visionTool.isEnabled()) {
        MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{};
        constexpr double legX{0.2};
        double legY{0.8};
        for (const SpacePoint* sp : max.getHitsInMax()) {
            const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*sp->primaryMeasurement());
            if (!simHit) {
                continue;
            }

            const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(simHit->identify());
            const Amg::Transform3D toChamb = reEle->msSector()->globalToLocalTransform(gctx) * 
                                             reEle->localToGlobalTransform(gctx, sp->identify());

            const Amg::Vector3D hitPos = toChamb * xAOD::toEigen(simHit->localPosition());
            const Amg::Vector3D hitDir = toChamb.linear() * xAOD::toEigen(simHit->localDirection());
            const double pull = std::sqrt(SeedingAux::chi2Term(hitPos, hitDir, *sp));
     
            if(sp->type() == xAOD::UncalibMeasType::MMClusterType) {
                const auto* mmClust = static_cast<const xAOD::MMCluster*>(sp->primaryMeasurement());
                const MuonGMR4::MmReadoutElement* mmEle = mmClust->readoutElement();
                const auto& design = mmEle->stripLayer(mmClust->measurementHash()).design();
                std::string stereoDesign{!design.hasStereoAngle() ? "X" : design.stereoAngle() >0 ? "U": "V"};
                primitives.push_back(MuonValR4::drawLabel(std::format("ml: {:1d}, gap: {:1d}, {:}, pull: {:.2f}", 
                                                                      mmEle->multilayer(), mmClust->gasGap(),
                                     stereoDesign, pull), legX, legY, 14));
            } else if(sp->type() == xAOD::UncalibMeasType::sTgcStripType) {
                const auto* sTgcMeas = static_cast<const xAOD::sTgcMeasurement*>(sp->primaryMeasurement());          
                std::string channelString = sp->secondaryMeasurement() == nullptr ?
                                            sTgcChannelType(sTgcMeas->channelType()) :
                                            std::format("{:}/{:}",  sTgcChannelType(sTgcMeas->channelType()),
                                                         sTgcChannelType(static_cast<const xAOD::sTgcMeasurement*>(sp->secondaryMeasurement())->channelType()));
                primitives.push_back(MuonValR4::drawLabel(std::format("ml: {:1d}, gap: {:1d}, type: {:}, pull: {:.2f}", 
                                            sTgcMeas->readoutElement()->multilayer(), sTgcMeas->gasGap(), 
                                            channelString, pull), legX, legY, 14));
            }
            legY-=0.05;           
        }
        m_visionTool->visualizeBucket(ctx, *max.parentBucket(),
                                      "truth", std::move(primitives));
    }

    UsedHitMarker_t allUsedHits = emptyBookKeeper(stripHitsLayers); 
    std::size_t nSeeds{0}, nExtSeeds{0}, nSegments{0}; //for the seed statistics

    // helper lamda function to increase counters and fill the seeds and segments we want to return after we construct them 
    // the extended seeds are returned even if they did not make it to a segment and the segments only if successfully fitted
    auto processSeedsAndSegments = [&](std::pair<SegmentSeedVec_t, SegmentVec_t>&& seedSegmentPairs, std::string_view source) {
        auto& [returnSeeds, returnSegments] = seedSegmentPairs;
        ATH_MSG_DEBUG("From " << source << ": built " << returnSeeds.size() << " seeds and " << returnSegments.size() << " segments.");
        for(auto& seed : returnSeeds) {            
            ++nSeeds;
            if(seed->getHitsInMax().size() < m_minSeedHits){
                continue;
            }                   
            ++nExtSeeds;
            seeds.push_back(std::move(seed));
        }
        //move all the segments to the output container
        std::ranges::move(returnSegments, std::back_inserter(segments));
        nSegments += returnSegments.size();
    };

    //Start from outermost sTgc layers with combined 2D measurements
    ATH_MSG_VERBOSE("Start building seed from sTgc outermost layers");
    processSeedsAndSegments(buildSegmentsFromSTGC(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits), "sTgc segment seeds");    

    //continue with the combinatorial seeding for the strip measurements
    if(m_doOnlyMMCombinatorics){

        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, true), "MM combinatoric segment seeds");

    }else{

        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, true), "MM combinatoric segment seeds");
        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, false), "MM and STGC combinatoric segment seeds");

    }   
    
     if(m_seedCounter) {
        m_seedCounter->addToStat(max.msSector(), nSeeds, nExtSeeds, nSegments);
    }

    return std::make_pair(std::move(seeds),std::move(segments));
}

void NswSegmentFinderAlg::markHitsAsUsed(const HitVec& spacePoints,
                                        const HitLayVec& allSortHits,
                                        UsedHitMarker_t& usedHitMarker, 
                                        unsigned incr,
                                        bool markNeighborHits) const {

    SpacePointPerLayerSorter layerSorter{};
      
    for(const auto& sp : spacePoints){
        // Proection against the auxiliary measurement
        if(!sp){
            continue;
        }           

        unsigned measLayer = layerSorter.sectorLayerNum(*sp);
        
        Amg::Vector2D spPosX{Amg::Vector2D::Zero()};
        /// @todo use the localMeasurementPos of the measurement?
        switch (sp->primaryMeasurement()->numDimensions()) {
            case 1:
                spPosX[Amg::x] = sp->primaryMeasurement()->localPosition<1>().x();
                break;
            case 2:
                spPosX = xAOD::toEigen(sp->primaryMeasurement()->localPosition<2>());
                break;
            default:
                THROW_EXCEPTION("Unsupported dimension");
        }

        for (std::size_t lIdx = 0; lIdx < allSortHits.size(); ++lIdx) {
            const HitVec& hVec{allSortHits[lIdx]}; 
            //check if they are not in the same layer
            unsigned hitLayer = layerSorter.sectorLayerNum(*hVec.front());
            if(hitLayer != measLayer) {
                ATH_MSG_VERBOSE("Not in the same layer since measLayer = "<< measLayer << " and "<<hitLayer);
                continue;
            }
            for (std::size_t hIdx = 0 ; hIdx < hVec.size(); ++hIdx) {
                //check the dY between the measurement and the hits                
                auto testHit = hVec[hIdx];
                if (testHit == sp) {
                    usedHitMarker[lIdx][hIdx] += incr;
                    if(!markNeighborHits){
                        break;
                    }                     
                } else if (markNeighborHits) {
                    Amg::Vector2D testPosX{Amg::Vector2D::Zero()};
                    /// @todo use the localMeasurementPos of the measurement?
                    switch (testHit->primaryMeasurement()->numDimensions()) {
                        case 1:
                            testPosX[Amg::x] = testHit->primaryMeasurement()->localPosition<1>().x();
                            break;
                        case 2:
                            testPosX = xAOD::toEigen(testHit->primaryMeasurement()->localPosition<2>());
                            break;
                        default:
                            THROW_EXCEPTION("Unsupported dimension");
                    }
                    //if the hit not found let's see if it is too close to the segment's measurement  
                    double deltaX = (testPosX - spPosX).mag();
                    if(deltaX < m_maxdYWindow){
                        usedHitMarker[lIdx][hIdx] += incr;
                    } 
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

StatusCode NswSegmentFinderAlg::finalize() {    
    if(m_seedCounter) {
        m_seedCounter->printTableSeedStats(msgStream());
    }
    return StatusCode::SUCCESS;
}

void NswSegmentFinderAlg::SeedStatistics::addToStat(const MuonGMR4::SpectrometerSector* msSector, unsigned seeds, unsigned extSeeds, unsigned segments){
    std::unique_lock guard{m_mutex};
    SectorField key{};
    key.chIdx = msSector->chamberIndex();
    key.phi = msSector->stationPhi();
    key.eta = msSector->chambers().front()->stationEta();

    auto &entry = m_seedStat[key]; 
    entry.nSeeds    += seeds;
    entry.nExtSeeds += extSeeds;
    entry.nSegments += segments;
}

void NswSegmentFinderAlg::SeedStatistics::printTableSeedStats(MsgStream& msg) const{


    std::stringstream sstr{};
    sstr<<"Seed statistics per sector:"<<std::endl;
    sstr<<"-----------------------------------------------------"<<std::endl;
    sstr<<"| Chamber | Phi | Eta | Seeds | ExtSeeds | Segments |"<<std::endl;
    sstr<<"-----------------------------------------------------"<<std::endl;

    using namespace  Muon::MuonStationIndex;

    for (const auto& [sector, stats] : m_seedStat) {
        sstr <<  "| " << std::setw(3) << chName(sector.chIdx)
             << " | " << std::setw(2) << static_cast<unsigned>(sector.phi)
             << " | " << std::setw(3) << static_cast<int>(sector.eta)
             << " | " << std::setw(7) << stats.nSeeds
             << " | " << std::setw(8) << stats.nExtSeeds
             << " | " << std::setw(8) << stats.nSegments
             << " |"<<std::endl;
    }

    sstr<<"------------------------------------------------------------"<<std::endl;
    msg<<MSG::ALWAYS<<"\n"<<sstr.str()<<endmsg;
 }
  

}  // namespace MuonR4
