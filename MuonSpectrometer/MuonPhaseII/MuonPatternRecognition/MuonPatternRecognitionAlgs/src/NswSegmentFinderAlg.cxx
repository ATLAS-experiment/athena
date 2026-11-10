/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NswSegmentFinderAlg.h"

#include "MuonSpacePoint/SpacePointPerLayerSplitter.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"


#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

#include "TruthUtils/HepMCHelpers.h"

#include "Acts/Seeding/CombinatorialSeedSolver.hpp"
#include "Acts/Definitions/Units.hpp"

#include <algorithm>
#include <ranges>
#include <format>

using namespace Acts::Experimental::CombinatorialSeedSolver;
using namespace Acts::UnitLiterals;
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

    inline int multilayer(const MuonR4::SpacePoint& sp) {
        const MuonGMR4::MuonReadoutElement* re = sp.primaryMeasurement()->readoutElement();
        if (re->detectorType() == ActsTrk::DetectorType::Mm) {
            return static_cast <const MuonGMR4::MmReadoutElement*>(re)->multilayer();
        }
        assert(re->detectorType() == ActsTrk::DetectorType::sTgc);
        return static_cast<const MuonGMR4::sTgcReadoutElement*>(re)->multilayer();
    }

    //local struct to encapsulate hit candidates data (e.g for seed extension)
    struct HitCandidate {
        double minPull{std::numeric_limits<float>::max()};
        const MuonR4::SpacePoint* spacePoint{nullptr};
    };
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
    fitCfg.recoveryPull = m_minPullThreshold;
    fitCfg.parsToUse = {ParamDefs::x0, ParamDefs::y0, ParamDefs::theta, ParamDefs::phi};
    
    m_lineFitter = std::make_unique<SegmentFit::SegmentLineFitter>(name(), std::move(fitCfg));

    SegmentAmbiSolver::Config ambicfg{};
    m_ambiSolver = std::make_unique<SegmentAmbiSolver>(name(), std::move(ambicfg));

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
    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check hit "<<testHit<<", orientation: "<<classifyStrip(testHit)<<".");
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
        } case P: {
            break;
        } case Unknown:{
            break;
        }

    }
    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Hit " << m_idHelperSvc->toString(testHit.identify())
                << (below || above ? " is outside the window" : " is inside the window"));
    if(below) {
        return HitWindow::tooLow;
    }
    if(above) {
        return HitWindow::tooHigh;
    }
    return HitWindow::inside;
};

void NswSegmentFinderAlg::constructPreliminarySeeds(const Amg::Vector3D& beamSpot,
                                                    const HitLaySpan_t& combinatoricLayers,
                                                    const UsedHitSpan_t& usedHits,
                                                    InitialSeedVec_t& seedHitsFromLayers) const {
    /// Assign enough memory to the vector
    seedHitsFromLayers.clear();
    std::size_t maxSize{1};
    for (const HitSpan_t& hitVec : combinatoricLayers) {
        maxSize = maxSize * hitVec.size();
    }
    seedHitsFromLayers.reserve(maxSize);

    unsigned iterLay0{0}, iterLay1{0}, iterLay2{0}, iterLay3{0};    
    unsigned startLay1{0}, startLay2{0}, startLay3{0};
    /** @brief Test whether a hit in a combinatoric layer is within a 
     *         corridor cone that is defined by the first hit. If the hit
     *         is below or above the corridor limit, the iterators are updated
     *         accordingly to avoid a testing with subsequent corridors
     *  @param dirEstDn: The lower edge direction of the defined corridor
     *  @param dirEstUp: The upper edge direction of the defined corridor
     *  @param layer: From which combinatoric layer the hit shall be picked [1-3]
     *  @param hitIter: The index iterator within the layer. Will be set to maximum if
     *                  the hit exceeds the upper limit (Subsequent hits will share the same destiny)
     *  @param startIter: The current iterator from which the loop over the hits in the combinatoric layer
     *                     will start */
    auto testHitCorridor = [&](const Amg::Vector3D& dirEstDn,
                                const Amg::Vector3D& dirEstUp,
                                const unsigned layer,
                                unsigned& hitIter,
                                unsigned& startIter)  -> bool {
        const SpacePoint* testMe = combinatoricLayers[layer][hitIter];
        if (usedHits[layer][hitIter] > m_maxUsed) {
            ATH_MSG_VERBOSE(__func__<<": "<<__LINE__<<" - Space point "<<(*testMe)
                           <<" is already defined in good seed");
            return false;
        }
        switch (hitFromIPCorridor(*testMe, beamSpot, dirEstUp, dirEstDn)) {
            case HitWindow::tooHigh: {
                hitIter = combinatoricLayers[layer].size();
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<(*testMe)
                                <<" is beyond the corridor. Break loop.");
                return false;
            } case HitWindow::tooLow: {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<(*testMe)
                            <<" is still below the corridor. Update start counter ");
                startIter = hitIter + 1;
                return false;
            } case HitWindow::inside: {
                break;
            }
        }
        return true;
    };

    for( ; iterLay0 <  combinatoricLayers[0].size() ; ++iterLay0){
        /// The hit is alrady in a good seed. Don't consider again
        if (usedHits[0][iterLay0] > m_maxUsed) {
            continue;
        }
        const SpacePoint* hit0 = combinatoricLayers[0][iterLay0];
        /// Construct the beamspot to first hit connection to guestimate the angle
        const Amg::Vector3D initSeedDir{(beamSpot - hit0->localPosition()).unit()};
        const Amg::Vector3D dirEstUp = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() - m_windowTheta); 
        const Amg::Vector3D dirEstDn = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() + m_windowTheta); 

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Reference hit: "<<(*hit0)<<", seed dir: "<<Amg::toString(initSeedDir)
                      <<", seed plane: "<<Amg::toString(SeedingAux::extrapolateToPlane(beamSpot, initSeedDir, *hit0)));
        /** Apply cut window on theta of the seed. */
        for( iterLay1 = startLay1; iterLay1 <  combinatoricLayers[1].size() ; ++iterLay1){
            if (!testHitCorridor(dirEstDn, dirEstUp, 1, iterLay1, startLay1)) {
                continue;
            }
            for( iterLay2 = startLay2; iterLay2 < combinatoricLayers[2].size() ; ++iterLay2){
                if (!testHitCorridor(dirEstDn, dirEstUp, 2, iterLay2, startLay2)) {
                    continue;
                }
                for( iterLay3 = startLay3; iterLay3 < combinatoricLayers[3].size(); ++iterLay3){
                    if (!testHitCorridor(dirEstDn, dirEstUp, 3, iterLay3, startLay3)) {
                        continue;
                    }
                    seedHitsFromLayers.emplace_back(std::array{hit0, combinatoricLayers[1][iterLay1], 
                        combinatoricLayers[2][iterLay2], combinatoricLayers[3][iterLay3]}); 
                }
            }
        }
    }
}
NswSegmentFinderAlg::HitVec 
    NswSegmentFinderAlg::extendHits(const Amg::Vector3D& startPos, 
                                    const Amg::Vector3D& direction, 
                                    const HitLaySpan_t& extensionLayers,
                                    const UsedHitSpan_t& usedHits) const {
    
    //the hits we need to return to extend the segment seed
    HitVec combinatoricHits;

    for (std::size_t i = 0; i < extensionLayers.size(); ++i) {
        const HitSpan_t& layer{extensionLayers[i]};
        const Amg::Vector3D extrapPos = SeedingAux::extrapolateToPlane(startPos, direction, *layer.front());

        
        unsigned triedHit{0};
        HitCandidate precisionHit, noPrecisionHit;
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Moving to next layer");
        
        for (unsigned j = 0; j < layer.size(); ++j) {
            if (usedHits[i][j] > m_maxUsed) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Hit " << (*layer[j])<< " already used with counter " << usedHits[i][j] << ". Skip.");
                continue;
            }
            auto hit = layer[j];
            const double pull = std::sqrt(SeedingAux::chi2Term(extrapPos, direction, *hit));
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Trying extension with hit " << *hit<<" and pull "<<pull<<" has truth: "<< (getTruthMatchedHit(*hit->primaryMeasurement()) != nullptr ? "oui" : "non"));
            bool isPrecision = isPrecisionHit(*hit);
            double minPull = isPrecision ? precisionHit.minPull : noPrecisionHit.minPull;
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Current min pull for this layer is:  "<<minPull);
            //find the hit with the minimum pull (check at least three hits after we have increasing pulls)
            if (pull > minPull) {
                triedHit+=1;  
                continue;                 
            }

            if(triedHit>3){
                break;
            }

            if(isPrecision){
                precisionHit.spacePoint = hit;
                precisionHit.minPull = pull;
                continue;
            }

            noPrecisionHit.spacePoint = hit;
            noPrecisionHit.minPull = pull;            
        }

        // complete the seed with the extended hits
        //we first choose the precision hit and if does not exist then we pick the non precision hit 
        const SpacePoint* bestCand{nullptr};
        if(precisionHit.minPull < m_minPullThreshold){ 
            bestCand = precisionHit.spacePoint;
        }else if(noPrecisionHit.minPull < m_minPullThreshold){
            bestCand = noPrecisionHit.spacePoint;
        }else{
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - No hit found in layer "<<i
                    <<" with pull below threshold "<<m_minPullThreshold);            
            continue;
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extension successfull - hit" << (*bestCand));
        combinatoricHits.push_back(bestCand);
    }
    
    return combinatoricHits;
}

std::unique_ptr<SegmentSeed>
    NswSegmentFinderAlg::constructCombinatorialSeed(const InitialSeed_t& initialSeed,
                                                    const AmgSymMatrix(2)& bMatrix,
                                                    const HoughMaximum& max,
                                                    const ActsTrk::GeometryContext& gctx,
                                                    const HitLaySpan_t& extensionLayers,
                                                    const UsedHitSpan_t& usedHits) const {
 
    if (std::ranges::none_of(initialSeed, 
                                [this](const SpacePoint* hit){
        if (hit->type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto* mmClust = static_cast<const xAOD::MMCluster*>(hit->primaryMeasurement());
            return mmClust->stripNumbers().size() >= m_minClusSize;
        }
        return true;
    })) {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed rejection: Not all clusters meet minimum strip size");
        return nullptr; 
    }
    

    std::array<double, 4> params = defineParameters(bMatrix, initialSeed);

    const auto [segPos, direction] = seedSolution(initialSeed, params);

    // check the consistency of the parameters - expected to lay in the strip's
    // length
    for (std::size_t i = 0; i < 4; ++i) {
        const double halfLength = stripHalfLength(*initialSeed[i]);
        
        if (std::abs(params[i]) > halfLength) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed Rejection: Invalid seed - outside of the strip's length "<< m_idHelperSvc->toString(initialSeed[i]->identify())
                          <<", param: "<<params[i]<<", halfLength: "<<halfLength);
            return nullptr;
        }
    }
    double tanAlpha = houghTanAlpha(direction);
    double tanBeta = houghTanBeta(direction);

    double interceptX = segPos.x();
    double interceptY = segPos.y();

    //seed quality check - we expect the tanAlpha not to be too big which would mean big deflection along the strip layers
    //we also set a cut using the beamspot for the tan alpha
    if(std::abs(tanAlpha) > m_maxTanAlpha){
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed Rejection: Invalid seed - tanAlpha "<<tanAlpha<<" above threshold "<<m_maxTanAlpha);
        return nullptr;
    }
    const Amg::Vector3D beamSpotPos = max.msSector()->globalToLocalTransform(gctx).translation();
    Amg::Vector3D bsDir{(segPos - beamSpotPos).unit()};
    if (bsDir.dot(direction) < 0.) {
        bsDir = -bsDir;
    }
    const double tanAlphaBS = houghTanAlpha(bsDir);
    const double cosAngle = bsDir.dot(direction);
    double thetaWindowCut{std::cos(2.*m_windowTheta)};
                    //accept the deflection of direction with a tolerance of 1 deg                    
    if(std::abs(cosAngle) < thetaWindowCut){  
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed Rejection: Not agreement with beam spot direction");                      
        return nullptr;                        
    }             

    // extend the seed to the segment -- include hits from the other layers too
    auto extendedHits = extendHits(segPos, direction, extensionLayers, usedHits);
    HitVec hits{};
    hits.insert_range(hits.end(), initialSeed);
    hits.insert_range(hits.end(), std::move(extendedHits));

    return std::make_unique<SegmentSeed>(tanBeta, interceptY, tanAlphaBS,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}


std::unique_ptr<Segment> NswSegmentFinderAlg::fitSegmentSeed(const EventContext& ctx,
                                                             const ActsTrk::GeometryContext& gctx, 
                                                             const SegmentSeed* patternSeed) const{
    
    if(patternSeed->getHitsInMax().size() < m_minSeedHits){
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__ <<"- Not enough hits in the SegmentSeed to fit a segment");
        return nullptr;    
    }
    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__ <<" - Use space points for the segment fit: "<<std::endl
                <<print(patternSeed->getHitsInMax()));

    //Calibration of the seed spacepoints
    CalibSpacePointVec calibratedHits = m_calibTool->calibrate(ctx, patternSeed->getHitsInMax(), 
                                                               patternSeed->localPosition(), 
                                                               patternSeed->localDirection(), 0.);

    const Acts::Transform3& locToGlob{patternSeed->msSector()->localToGlobalTransform(gctx)};
 
    return m_lineFitter->fitSegment(ctx, patternSeed, patternSeed->parameters(),
                                    locToGlob, std::move(calibratedHits));
}

void NswSegmentFinderAlg::processSegment(std::unique_ptr<Segment> segment, 
                                         const HitVec& seedHits, 
                                         const HitLayVec& hitLayers, 
                                         UsedHitMarker_t& usedHits,
                                         SegmentVec_t& segments) const {
    
    if (!segment) {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed Rejection: Segment fit failed");
       
        if (m_markHitsFromSeed && seedHits.size() > m_minSeedHits) {
            // Mark hits from extended seed (used increment by 1)            
            markHitsAsUsed(seedHits, hitLayers, usedHits, 1, false);
        }
        return;
    }

    // -------- success path --------
    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Segment built with "
                  << segment->measurements().size()
                  << " hits, chi2/ndof: "
                  << segment->chi2() / std::max(1u,segment->nDoF()));

    HitVec segMeasSP;
    segMeasSP.reserve(segment->measurements().size());

    std::ranges::transform(segment->measurements(),std::back_inserter(segMeasSP),
                            [](const Segment::MeasType& m) { return m->spacePoint(); }
    );

    // Mark segment hits as fully used (used increment by 10,
    // hits are effectively removed)
    markHitsAsUsed(segMeasSP, hitLayers, usedHits, 10, true);
    segments.push_back(std::move(segment));

}

void NswSegmentFinderAlg::resolveAmbiguities(const ActsTrk::GeometryContext& gctx, 
                                             SegmentVec_t& segmentCandidates) const{

    if(segmentCandidates.size()<=1){
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - No segments to resolve ambiguities");
        return;
    }

    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Resolving ambiguities for "<<segmentCandidates.size()<<" segments");

    std::map<const MuonGMR4::SpectrometerSector*, SegmentVec_t,
                 MuonGMR4::MuonDetectorManager::MSEnvelopeSorter> segmentsPerChamber{};
    
    //sort segments per chamber and resolve ambiguities per chamber
    for (std::unique_ptr<Segment>& seg : segmentCandidates) {
        const MuonGMR4::SpectrometerSector* chamb = seg->msSector();
        segmentsPerChamber[chamb].push_back(std::move(seg));
    }
    segmentCandidates.clear();
    for (auto& [chamber, resolveMe] : segmentsPerChamber) {    
        std::ranges::move(m_ambiSolver->resolveAmbiguity(gctx, std::move(resolveMe)), 
                          std::back_inserter(segmentCandidates));
    }

}

std::pair<NswSegmentFinderAlg::SegmentSeedVec_t, NswSegmentFinderAlg::SegmentVec_t>
NswSegmentFinderAlg::buildSegmentsFromSTGC(const EventContext& ctx,
                                           const ActsTrk::GeometryContext &gctx, 
                                           const HitLayVec& hitLayers,                                          
                                           const HoughMaximum& max,
                                           const Amg::Vector3D& beamSpotPos,
                                           UsedHitMarker_t& usedHits,
                                           bool usePad) const {

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
        if(sp->dimension() != 2){
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Ignoring the 1D measurement for seeding: "<<m_idHelperSvc->toString(sp->identify()));
            return false;
        }
        const auto* prdPrim = static_cast<const xAOD::sTgcMeasurement*>(sp->primaryMeasurement());
        const auto* prdSec = static_cast<const xAOD::sTgcMeasurement*>(sp->secondaryMeasurement());
        //we know we have a 2D spacepoint can be: pad, strip+pad, strip+wire, wire+pad and we dont want pads appearing in this seeding stage
        bool hasPad = prdSec ? prdSec->channelType()==sTgcIdHelper::sTgcChannelTypes::Pad : prdPrim->channelType()==sTgcIdHelper::sTgcChannelTypes::Pad;
        if(hasPad && !usePad){
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Ignoring the 2D pad measurement for seeding because of pads: "<<m_idHelperSvc->toString(sp->identify()));
            return false;
        }
        bool isPrecision = isPrecisionHit(*sp);
        return isPrecision && usedHits[layIdx].at(hitIdx) <= m_maxUsed;

    };

    // find the 2D measurements from the outermost layers - even move one layer inside 
    for(std::size_t layIdx1 = 0; layIdx1 < 2; ++layIdx1){        
        for(std::size_t layIdx2 = layerSize-1; layIdx2 >= layerSize-2; --layIdx2){
            //in case of MM layers we stop - the layers are sorted in Z
            if(hitLayers[layIdx1].front()->type() == xAOD::UncalibMeasType::MMClusterType  || 
               hitLayers[layIdx2].front()->type() == xAOD::UncalibMeasType::MMClusterType){
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"Outermost layers are MM - stop searching for sTgc Measurements");
                return std::make_pair(std::move(seeds), std::move(segments));
            }

            //we might request to have hits from the two different multilayers for better lever arm
            if(m_skipSameMultilayer){
                const int ml1 = multilayer(*hitLayers[layIdx1].front());
                const int ml2 = multilayer(*hitLayers[layIdx2].front());
                if(ml1 == ml2){
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"Skip sTGC seeding because the two hits are in the same multilayer");
                    return std::make_pair(std::move(seeds),std::move(segments));
                }           
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
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"Attempt for STGC segment starting with the hits: " << *hit1 << ", " << *hit2);
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

                    const Amg::Vector3D seedBeamspotDir{(hit2->localPosition() - beamSpotPos).unit()};
                    //
                    auto seed = std::make_unique<SegmentSeed>(houghTanBeta(seedDir), seedPosZ0.y(),
                                                         houghTanAlpha(seedBeamspotDir), seedPosZ0.x(),
                                                         seedHits.size(), std::move(seedHits),
                                                         max.parentBucket());
            
                    /** Do not promote seeds with little hits to segments */    
                    if(seed->getHitsInMax().size() < m_minSeedHits){
                        seeds.push_back(std::move(seed));
                        continue;
                    }
                    //fit the segment seed
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Start to fit an STGC segment seed with "<<seed->getHitsInMax().size()<<" hits \n"<<print(seed->getHitsInMax()));
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
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Not enough layers to build a seed");
        return {std::move(seeds), std::move(segments)};
    }
    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Construct seeds from "<<layerSize<<" layers.");
    
    //lamda helper to find the first unused strip hit on the layer


    std::array<const SpacePoint*, 4> seedHits{};
    std::array<StripOrient, 4> orientation{};
    std::array<std::size_t, 4> layOrder{};
    InitialSeedVec_t preLimSeeds{};
    /** @brief Attempt to find the next unused hit in a certain layer and fill
     *         the hit in the seedHits array
     *  @param seedIdx: Destination index where the hit will be filled in the 
     *                  seedHits / layIndices array
     *  @param layIdx: The index of the layer to pick from the input layer hit input */
    auto unusedStripHit = [&](unsigned seedIdx, unsigned layIdx) -> bool {
        const HitVec& layerHits = hitLayers[layIdx];
        
        //in case of MM only combinatorial seeding - we consider only MM strip hits for seeding
        bool isMM = useOnlyMM ? layerHits.front()->type() == xAOD::UncalibMeasType::MMClusterType : true;
        for (auto [idx, hit] : Acts::enumerate(layerHits)) {
            auto spOrient = classifyStrip(*hit);
            bool isStrip = spOrient == StripOrient::X || spOrient == StripOrient::U || spOrient == StripOrient::V;
            bool isUnused = usedHits[layIdx].at(idx) <= m_maxUsed;           
            if (isStrip && isUnused && isMM) {
                layOrder[seedIdx] = layIdx;
                seedHits[seedIdx] = hit;
                orientation[seedIdx] = spOrient;
                return true;
            }
        }
        return false;       
    };

    for (std::size_t i = 0; i < layerSize - 3; ++i) {
        if(!unusedStripHit(0, i)) {
            continue;
        }
        for (std::size_t j = i + 1; j < layerSize - 2; ++j) {
            if(!unusedStripHit(1, j)){
                continue;
            }
            for (std::size_t l = layerSize - 1; l > j+1; --l) {
                if(!unusedStripHit(3, l)){
                    continue;
                }
                for (std::size_t k = l-1; k > j ; --k) {
                    if(!unusedStripHit(2, k)) {
                        continue;
                    }
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Layers: "<<layOrder<<", orientation: "
                                   <<orientation<<"\n"<<print(seedHits));
                    /// If one of the two strip pairs is parallel try to swap their order
                    if (orientation[3] != orientation[1] && (orientation[3] == orientation[2]  || 
                                                             orientation[1] == orientation[0]) && 
                        max.msSector()->halfY() -  hitLayers[layOrder[3]].back()->localPosition().y() < m_combParallelYCutOff){
                        std::swap(layOrder[3], layOrder[1]);
                        std::swap(seedHits[3], seedHits[1]);
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"- Swap layer order "<<layOrder
                            <<", "<<((max.msSector()->halfY() -  hitLayers[layOrder[3]].back()->localPosition().y()) / 1._cm)
                            <<", "<<max.msSector()->halfY()<<", "<<(max.msSector()->localToGlobalTransform(gctx) * 
                        hitLayers[layOrder[3]].back()->localPosition()).perp());
                    } 
                    const HitLaySpan_t layers{hitLayers[layOrder[0]], hitLayers[layOrder[1]], 
                                              hitLayers[layOrder[2]], hitLayers[layOrder[3]]};
                    //skip combination with at least one too busy layer
                    bool tooBusy = std::ranges::any_of(layers,
                                            [this](const HitSpan_t& layer) {
                                                return layer.size() > m_maxClustersInLayer;
                                            });
                    if (tooBusy) {
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Reject due to busy limit");
                        continue; // skip this combination
                    }
                    
                    AmgSymMatrix(2) bMatrix = betaMatrix(seedHits);   
       
                    if (std::abs(bMatrix.determinant()) < 1.e-6) {
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Invalid layers picked");
                        continue;
                    }
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Accepted layers"); 
                    UsedHitSpan_t usedHitsSpan{usedHits[layOrder[0]], usedHits[layOrder[1]], 
                                               usedHits[layOrder[2]], usedHits[layOrder[3]]};    
                    // each layer may have more than one hit - take the hit combinations                    
                    constructPreliminarySeeds(beamSpotPos, layers, usedHitsSpan, preLimSeeds);
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Constructed seeds "<<preLimSeeds.size()<<".");

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
                        auto seed = constructCombinatorialSeed(combinatoricHits, bMatrix, max, gctx, 
                                                               extensionLayers, usedExtensionHits);
                        if(!seed){
                            continue;
                        }
                        if (seed->getHitsInMax().size() < m_minSeedHits) { 
                            seeds.push_back(std::move(seed));                           
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

    const Acts::Transform3 globToLocal = max.msSector()->globalToLocalTransform(gctx);
    //counters for the number of seeds, extented seeds and segments

    if (layerSize < minLayers) {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Not enough layers to build a seed");
        return std::make_pair(std::move(seeds),std::move(segments));
    }

    if (m_visionTool.isEnabled()) {
        MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{};
        constexpr double legX{0.2};
        double legY{0.8};       
        primitives.push_back(MuonValR4::drawLabel("TrueHits matched with reco houghmax hits", legX, legY, 8));
        legY -= 0.02;

        for (const SpacePoint* sp : max.getHitsInMax()) {
            const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*sp->primaryMeasurement());
            if (!simHit) {
                continue;
            }

            const MuonGMR4::MuonReadoutElement* reEle = m_detMgr->getReadoutElement(simHit->identify());
            const Acts::Transform3 toChamb = reEle->msSector()->globalToLocalTransform(gctx) * 
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
                                     stereoDesign, pull), legX, legY, 8));
            } else if(sp->type() == xAOD::UncalibMeasType::sTgcStripType) {
                const auto* sTgcMeas = static_cast<const xAOD::sTgcMeasurement*>(sp->primaryMeasurement());          
                std::string channelString = sp->secondaryMeasurement() == nullptr ?
                                            sTgcChannelType(sTgcMeas->channelType()) :
                                            std::format("{:}/{:}",  sTgcChannelType(sTgcMeas->channelType()),
                                                         sTgcChannelType(static_cast<const xAOD::sTgcMeasurement*>(sp->secondaryMeasurement())->channelType()));
                primitives.push_back(MuonValR4::drawLabel(std::format("ml: {:1d}, gap: {:1d}, type: {:}, pull: {:.2f}", 
                                            sTgcMeas->readoutElement()->multilayer(), sTgcMeas->gasGap(), 
                                            channelString, pull), legX, legY, 8));
            }
            legY-=0.02;           
        }
        m_visionTool->visualizeBucket(ctx, *max.parentBucket(),
                                      "bucket", std::move(primitives));
    }

    //dump spacepoints associated with truth sim hits to an obj file
    if(m_dumpObj){
        Acts::ObjVisualization3D visualHelper{};
         for (const SpacePoint* sp : max.getHitsInMax()) {
            const xAOD::MuonSimHit* simHit = getTruthMatchedHit(*sp->primaryMeasurement());
            if (!simHit) {
                continue;
            }
            MuonValR4::drawSpacePoint(gctx.context(), *sp, visualHelper);
        }
        visualHelper.write(std::format("Event_{:}_{:}_spacepoints_truth.obj", ctx.eventID().event_number(), max.getHitsInMax().front()->chamber()->identString()));       
    }


    UsedHitMarker_t allUsedHits = emptyBookKeeper(stripHitsLayers); 
    std::size_t nSeeds{0}, nExtSeeds{0}, nSegments{0}; //for the seed statistics

    // helper lamda function to increase counters and fill the seeds and segments we want to return after we construct them 
    // the extended seeds are returned even if they did not make it to a segment and the segments only if successfully fitted
    auto processSeedsAndSegments = [&](std::pair<SegmentSeedVec_t, SegmentVec_t>&& seedSegmentPairs, std::string_view source) {
        auto& [returnSeeds, returnSegments] = seedSegmentPairs;
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - From " << source << ": built " << returnSeeds.size() << " seeds and " << returnSegments.size() << " segments.");
        for(auto& seed : returnSeeds) {            
            ++nSeeds;
            Acts::ObjVisualization3D visualHelper{};
            if(seed->getHitsInMax().size() < m_minSeedHits){
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed with "<< seed->getHitsInMax().size() <<" hits rejected. \n"<<print(seed->getHitsInMax()));
                if(m_dumpObj){
                    for(const auto& hit : seed->getHitsInMax()){
                        MuonValR4::drawSpacePoint(gctx.context(), *hit, visualHelper);
                    }
                    visualHelper.write(std::format("Event_{:}_{:}_notExtendedSeed.obj", ctx.eventID().event_number(), 
                                        seed->getHitsInMax().front()->chamber()->identString()));
                }
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
    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Start building seed from sTgc outermost layers");
    processSeedsAndSegments(buildSegmentsFromSTGC(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, false), "sTgc segment seeds");    
    
    //continue with the combinatorial seeding for the strip measurements
    if(m_doOnlyMMCombinatorics){
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Start building combinatoric seeds only from Micromegas");
        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, true), "MM combinatoric segment seeds");
    } else {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Start building combinatoric seeds from Micromegas and sTgc hits");
        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, true), "MM combinatoric segment seeds");
        processSeedsAndSegments(buildSegmentsFromMM(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, false), "MM and STGC combinatoric segment seeds");
        processSeedsAndSegments(buildSegmentsFromSTGC(ctx, gctx, stripHitsLayers, max, globToLocal.translation(), allUsedHits, true), "sTgc segment seeds using strip+pads");    
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
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Not in the same layer since measLayer = "<< measLayer << " and "<<hitLayer);
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
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"- Marking hit "<<(*testHit)<<", used count: "
                                <<usedHitMarker[lIdx][hIdx]);
            }
        }
    }
}

StatusCode NswSegmentFinderAlg::execute(const EventContext &ctx) const {
    // read the inputs
    const EtaHoughMaxContainer *maxima{nullptr};
    ATH_CHECK(SG::get(maxima, m_etaKey, ctx));

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
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Process maximum "<<max->tanBeta()<<", "<<max->interceptY()<<"\n"<<print(max->getHitsInMax()));
        auto [seeds, segments] = findSegmentsFromMaximum(*max, *gctx, ctx);
        for(auto& seed: seeds){
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Seed tanBeta = "<<seed->tanBeta()<<", y0 = "<<seed->interceptY()
                         <<", tanAlpha = "<<seed->tanAlpha()<<", x0 = "<<seed->interceptX()<<", hits in the seed "
                         <<seed->getHitsInMax().size()<<"\n"<<print(seed->getHitsInMax()));
            if (m_visionTool.isEnabled()) {          
                m_visionTool->visualizeSeed(ctx, *seed, "#phi-combinatorialSeed");
            }
            writeSegmentSeeds->push_back(std::move(seed));
        }
        if (seeds.empty() && m_visionTool.isEnabled()){
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Maximum could not be made to a seed");
            m_visionTool->visualizeSeed(ctx, *max, "failed seed");
        }

        //Resolve ambiguities between segments before writing them to the output
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Before ambiguity resolution, there are in total "<<segments.size()<<" segments:");
        for(const auto& seg : segments){
            if(msgLvl(MSG::VERBOSE)){
                std::stringstream sstr{};
                sstr<<"Segment chi2/ndof = "<<seg->chi2()/std::max(1u,seg->nDoF())<<", hits in the segment "
                         <<seg->measurements().size()<<std::endl;        
            
                const Parameters pars = localSegmentPars(*gctx, *seg);
                sstr<<"Segment parameters : "<<toString(pars)<<std::endl;
                for(const Segment::MeasType& hit : seg->measurements()){
                    if(hit->type()!=xAOD::UncalibMeasType::Other){
                        sstr<<" *** Hit "<<(*hit)<<", has truth matched: "
                            <<(getTruthMatchedHit(*hit->spacePoint()->primaryMeasurement()) !=nullptr)<<std::endl;
                    }    
                }
                ATH_MSG_VERBOSE(sstr.str());
            }
        }
        resolveAmbiguities(*gctx, segments);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - After ambiguity resolution, there are in total "
                        <<segments.size()<<" segments:");

        for (auto &seg : segments) { 
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment chi2/ndof = "<<seg->chi2()/std::max(1u,seg->nDoF())
                        <<", hits in the segment "<<seg->measurements().size()<<"\n"<<print(seg->measurements()));          
            
            if (m_visionTool.isEnabled()) {          
                m_visionTool->visualizeSegment(ctx, *seg, "#phi-segment");
            }

            if(m_dumpObj){
                Acts::ObjVisualization3D visualHelper{};
                MuonValR4::drawSegmentMeasurements(gctx->context(), *seg, visualHelper);
                MuonValR4::drawSegmentLine(gctx->context(), *seg, visualHelper);
                visualHelper.write(std::format("Event_{:}_segment_{:}.obj", ctx.eventID().event_number(), seg->msSector()->identString()));
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

void NswSegmentFinderAlg::SeedStatistics::addToStat(const MuonGMR4::SpectrometerSector* msSector, 
                                                    unsigned seeds, unsigned extSeeds, unsigned segments){
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
    sstr<<"| Chamber | Phi | Eta | Seeds | ExtSeeds | FittedSegments |"<<std::endl;
    sstr<<"-----------------------------------------------------"<<std::endl;

    using namespace  Muon::MuonStationIndex;

    for (const auto& [sector, stats] : m_seedStat) {
        sstr <<  "| " << std::setw(3) << sector.chIdx
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
