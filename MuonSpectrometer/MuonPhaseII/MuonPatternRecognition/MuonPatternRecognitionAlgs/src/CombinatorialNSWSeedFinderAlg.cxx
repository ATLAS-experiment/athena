/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CombinatorialNSWSeedFinderAlg.h"

#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>

#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternHelpers/CombinatorialSeedSolver.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MMCluster.h"

#include "TruthUtils/HepMCHelpers.h"

#include <ranges>
#include <vector>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <format>
#include <fstream> 



namespace MuonR4 {

constexpr unsigned int minLayers{4};

StatusCode CombinatorialNSWSeedFinderAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_etaKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_visionTool.retrieve(DisableTool{m_visionTool.empty()}));
    ATH_CHECK(detStore()->retrieve(m_detMgr));

    if (!(m_idHelperSvc->hasMM() || m_idHelperSvc->hasSTGC())) {
        ATH_MSG_ERROR("MM or STGC not part of initialized detector layout");
        return StatusCode::FAILURE;
    }
   
    return StatusCode::SUCCESS;
}

template <class ContainerType>
StatusCode CombinatorialNSWSeedFinderAlg::retrieveContainer(const EventContext &ctx, 
                                                            const SG::ReadHandleKey<ContainerType> &key,
                                                            const ContainerType* &contToPush) const {
    contToPush = nullptr;
    if (key.empty()) {
        ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle readHandle{key, ctx};
    ATH_CHECK(readHandle.isPresent());
    contToPush = readHandle.cptr();
    return StatusCode::SUCCESS;
}

HitWindow CombinatorialNSWSeedFinderAlg::findHitInWindow(const Amg::Vector3D& startPos, 
                                                        const SpacePoint* testHit, 
                                                        const Amg::Vector3D& dirEstUp,
                                                        const Amg::Vector3D& dirEstDn) const{

    const double planeOffSet = testHit->positionInChamber().dot(testHit->planeNormal());

    const Amg::Vector3D estPlaneArrivalUp = startPos+ 
        Amg::intersect<3>(startPos, dirEstUp, testHit->planeNormal(), planeOffSet).value_or(0) * dirEstUp;
    const Amg::Vector3D estPlaneArrivalDn = startPos + 
        Amg::intersect<3>(startPos, dirEstDn, testHit->planeNormal(), planeOffSet).value_or(0) * dirEstDn;

    switch (testHit->type()) {
    case xAOD::UncalibMeasType::MMClusterType:{
    const auto* prd = static_cast<const xAOD::MMCluster*>(testHit->primaryMeasurement());

    const double halfLength =  prd->readoutElement()->stripLayer(prd->measurementHash()).design().stripLength(prd->channelNumber()) * 0.5;
    /// Calculate the strip edges
    const Amg::Vector3D leftEdge = testHit->positionInChamber() - halfLength * testHit->directionInChamber();
    const Amg::Vector3D rightEdge = testHit->positionInChamber() + halfLength * testHit->directionInChamber();

    const bool below = estPlaneArrivalDn.y() > std::max(leftEdge.y(), rightEdge.y());
    const bool above = estPlaneArrivalUp.y() < std::min(leftEdge.y(), rightEdge.y()); 
    ATH_MSG_DEBUG("Hit " << m_idHelperSvc->toString(testHit->identify())
                        << (below || above ? " is outside the window" : " is inside the window"));

  
    if(below) return HitWindow::tooLow;
    if(above) return HitWindow::tooHigh;
    
    return HitWindow::inside;
    }
    default:
    ATH_MSG_DEBUG("STGCs not implemented yet - I will do ");
    break;
    }
    //return randmoly an outside hit for now
    return HitWindow::tooHigh;
};

HitLayVec CombinatorialNSWSeedFinderAlg::findCombinatoricHits(const Amg::Vector3D& beamSpot,
                                                              const HitLayVec &combinatoricLayers) const {
  

    HitLayVec seedHitsFromLayers{};
    //try all the hits combinations from the layers for now (--rethink about optimized way)
    unsigned int iterLay0{0}, iterLay1{0}, iterLay2{0}, iterLay3{0};    
    unsigned int startLay1{0}, startLay2{0}, startLay3{0};
    
    for( ; iterLay0 <  combinatoricLayers[0].size() ; ++iterLay0){

        const SpacePoint* hit0 = combinatoricLayers[0][iterLay0];
        /// Construct the beamspot to first hit connection to guestimate the angle
        const Amg::Vector3D initSeedDir{(beamSpot - hit0->positionInChamber()).unit()};
        const Amg::Vector3D dirEstUp = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() - m_windowTheta); 
        const Amg::Vector3D dirEstDn = Amg::dirFromAngles(initSeedDir.phi(), initSeedDir.theta() + m_windowTheta); 

        ATH_MSG_VERBOSE("Reference hit: "<<m_idHelperSvc->toString(hit0->identify())
                    <<", position: "<<Amg::toString(hit0->positionInChamber())<<" seed dir: "<<Amg::toString(initSeedDir)
                <<"  seed plane"<<Amg::toString(beamSpot + Amg::intersect<3>(beamSpot, initSeedDir, hit0->planeNormal(),
                                                                hit0->planeNormal().dot(hit0->positionInChamber())).value_or(0.) * initSeedDir ));
        /** Apply cut window on theta of the seed. */
        for( iterLay1 = startLay1; iterLay1 <  combinatoricLayers[1].size() ; ++iterLay1){
            const SpacePoint* hit1 = combinatoricLayers[1][iterLay1];
            const auto hit1InWindow = findHitInWindow(beamSpot, hit1, dirEstUp, dirEstDn);
            // hit is above- hits are sorted in y so break the loops (we need 4 hits for the seed = no reason to check the others)
            if(hit1InWindow == HitWindow::tooHigh) {
                break;
            }else if(hit1InWindow == HitWindow::tooLow){
                startLay1=iterLay1;
                continue;
            }
            for( iterLay2 = startLay2; iterLay2 < combinatoricLayers[2].size() ; ++iterLay2){
                const SpacePoint* hit2 = combinatoricLayers[2][iterLay2];
                const auto hit2InWindow = findHitInWindow(beamSpot, hit2, dirEstUp, dirEstDn);
                if (hit2InWindow == HitWindow::tooHigh){
                    iterLay1 = combinatoricLayers[1].size();
                    break;
                }else if(hit2InWindow == HitWindow::tooLow){
                    startLay2=iterLay2;
                    continue;
                }    
                for( iterLay3 = startLay3; iterLay3 < combinatoricLayers[3].size(); ++iterLay3){
                    const SpacePoint* hit3 = combinatoricLayers[3][iterLay3];
                    const auto hit3InWindow = findHitInWindow(beamSpot, hit3, dirEstUp, dirEstDn);
                    if (hit3InWindow == HitWindow::tooHigh){
                        iterLay1 = combinatoricLayers[1].size();
                        iterLay2 = combinatoricLayers[2].size();
                        break;
                    }else if(hit3InWindow == HitWindow::tooLow){
                        startLay3=iterLay3;
                        continue;
                    }
                    seedHitsFromLayers.emplace_back(HitVec{hit0, hit1, hit2, hit3}); 
                }
            }
        }
    }
    return seedHitsFromLayers;
}


HitVec CombinatorialNSWSeedFinderAlg::extendHits(const Amg::Vector3D &startPos, 
                                               const Amg::Vector3D &direction,
                                               const HitLayVec& stripHitsLayers) const {
    
    //the hits we need to return to extend the segment seed
    HitVec combinatoricHits;
    
    //the stripHitsLayers are already the unused ones - only use for the extension
    for (unsigned int i = 0; i < stripHitsLayers.size(); i++) {

        double offset = stripHitsLayers[i].front()->positionInChamber().dot(stripHitsLayers[i].front()->planeNormal());
        const Amg::Vector3D extrapPos = startPos + Amg::intersect<3>(startPos, direction, stripHitsLayers[i].front()->planeNormal(), offset).value_or(0) *  direction;

        unsigned int indexOfHit = stripHitsLayers[i].size()+1;
        unsigned int triedHit{0};
        double minPull{std::numeric_limits<double>::max()};
       
        // loop over the hits on the same layer
        for (unsigned int j = 0; j < stripHitsLayers[i].size(); j++) {
            auto hit = stripHitsLayers[i].at(j);   
            double pull = std::sqrt(SegmentFitHelpers::chiSqTermStrip(extrapPos, direction, *hit, msg()));
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
            ATH_MSG_VERBOSE("Extension successfull - hit" << m_idHelperSvc->toString(stripHitsLayers[i].at(indexOfHit)->identify())<<", pos: "
                              <<Amg::toString(stripHitsLayers[i].at(indexOfHit)->positionInChamber())<<", dir: "<<Amg::toString(stripHitsLayers[i].at(indexOfHit)->directionInChamber())<<" found with pull "<<minPull);
            combinatoricHits.push_back(stripHitsLayers[i].at(indexOfHit));
        }
    }
    return combinatoricHits;
}

std::unique_ptr<SegmentSeed>
CombinatorialNSWSeedFinderAlg::buildSegmentSeed(HitVec& hits, 
                                                const AmgSymMatrix(2)& bMatrix,
                                                const HoughMaximum& max, 
                                                const HitLayVec& extensionLayers) const {
    // we require at least four hits for the seeding
    if (hits.size() != minLayers) {
        ATH_MSG_VERBOSE("Seed Rejection: Wrong number of initial layers for seeding --they should be four");
        return nullptr;
    }

    std::array<double, 4> params = CombinatorialSeedSolver::defineParameters(bMatrix, hits);

    const auto [segPos, direction] = CombinatorialSeedSolver::seedSolution(hits, params);

    double tanPhi = houghTanPhi(direction);
    double tanTheta = houghTanTheta(direction);

    double interceptX = segPos.x();
    double interceptY = segPos.y();
    // check the consistency of the parameters - expected to lay in the strip's
    // length
    for (std::size_t i = 0; i < 4; i++) {
        const xAOD::UncalibratedMeasurement *primaryMeas = hits[i]->primaryMeasurement();

        if (primaryMeas->type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto *clust = static_cast<const xAOD::MMCluster *>(primaryMeas);
            
            double halfLength = 0.5 * clust->readoutElement()->stripLayer(clust->measurementHash()).design().stripLength(clust->channelNumber());
        
            if (std::abs(params[i]) > halfLength) {
                ATH_MSG_VERBOSE("Seed Rejection: Invalid seed - outside of the strip's length");
                return nullptr;
            }
        }
    }

    // extend the seed to the segment -- include hits from the other layers too
    auto extendedHits = extendHits(segPos, direction, extensionLayers);
    hits.insert(hits.end(), extendedHits.begin(), extendedHits.end());
    return std::make_unique<SegmentSeed>(tanTheta, interceptY, tanPhi,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}

std::vector<std::unique_ptr<SegmentSeed>>
CombinatorialNSWSeedFinderAlg::findSeedsFromMaximum(const HoughMaximum &max, const ActsGeometryContext &gctx) const {
    // first sort the hits per layer from the maximum
    SpacePointPerLayerSplitter hitLayers{max.getHitsInMax()};

    HitLayVec stripHitsLayers{hitLayers.stripHits()};

    std::vector<std::unique_ptr<SegmentSeed>> seeds;

    unsigned int layerSize = stripHitsLayers.size();

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
        
            const double pull = std::sqrt(SegmentFitHelpers::chiSqTermStrip(hitPos,hitDir, *sp, msgStream()));
            const double pull2 = (mmClust->localPosition<1>().x() - simHit->localPosition().x()) / std::sqrt(mmClust->localCovariance<1>().x());
            primitives.push_back(MuonValR4::drawLabel(std::format("ml: {:1d}, gap: {:1d}, {:}, pull: {:.2f} / {:.2f}", reEle->multilayer(), mmClust->gasGap(), 
                                !design.hasStereoAngle() ? "X" : design.stereoAngle() >0 ? "U": "V",pull, pull2),legX,legY,14));
            legY-=0.05;           
        }
        m_visionTool->visualizeBucket(Gaudi::Hive::currentContext(), *max.parentBucket(),
                                      "truth", std::move(primitives));
    }

    const Amg::Transform3D globToLocal = max.msSector()->globalToLocalTrans(gctx);
    std::array<const SpacePoint*, 4> seedHits{};
    for (std::size_t i = 0; i < layerSize - 3; ++i) {
        seedHits[0] = stripHitsLayers[i].front();
        for (std::size_t j = i + 1; j < layerSize - 2; ++j) {
            seedHits[1] = stripHitsLayers[j].front();
            for (std::size_t k = j + 1; k < layerSize - 1; ++k) {
                seedHits[2] = stripHitsLayers[k].front();
                for (std::size_t l = k + 1; l < layerSize; ++l) {
                    seedHits[3] = stripHitsLayers[l].front();
                    AmgSymMatrix(2) bMatrix = CombinatorialSeedSolver::betaMatrix(seedHits);                   
                    if (std::abs(bMatrix.determinant()) < 1.e-6) {
                       
                        continue;
                    }
                  
                    const HitLayVec layers{stripHitsLayers[i], stripHitsLayers[j], stripHitsLayers[k], stripHitsLayers[l]};
             
    
                    // each layer may have more than one hit - take the hit combinations                    
                     HitLayVec result = findCombinatoricHits(globToLocal.translation(), layers);

                     //the layers not participated in the seed build - gonna be used for the extension  
                    HitLayVec extensionLayers;
                    std::copy_if(stripHitsLayers.begin(), stripHitsLayers.end(), std::back_inserter(extensionLayers),
                                [&layers,this](const HitVec& stripLayer){
                                const Identifier gasGapId = m_idHelperSvc->gasGapId(stripLayer.front()->identify());
                                return std::ranges::find_if(layers,[&gasGapId, this](const HitVec& spacePoints){
                                    return gasGapId == m_idHelperSvc->gasGapId(spacePoints.front()->identify());
                                }) == layers.end();
                    });               

                    // we have made sure to have hits from all the four layers -
                    // start by 4 hits for the seed and try to build the seed for the combinatorics found
                    for (auto &combinatoricHits : result) {
                        auto seed = buildSegmentSeed(combinatoricHits, bMatrix, max, extensionLayers);
                        if (seed) {
                            seeds.push_back(std::move(seed));
                            
                        }
                    }
         
                }
            }
        }
    }

    return seeds;
}

StatusCode CombinatorialNSWSeedFinderAlg::execute(const EventContext &ctx) const {
    // read the inputs
    const EtaHoughMaxContainer *maxima{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_etaKey, maxima));

    const ActsGeometryContext *gctx{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));

    // prepare our output collection
    SG::WriteHandle writeMaxima{m_writeKey, ctx};
    ATH_CHECK(writeMaxima.record(std::make_unique<SegmentSeedContainer>()));

    // we use the information from the previous eta-hough transform
    // to get the combined hits that belong in the same maxima
    for (const HoughMaximum *max : *maxima) {
        std::vector<std::unique_ptr<SegmentSeed>> seeds = findSeedsFromMaximum(*max, *gctx);
     
        for(const auto& hitMax : max->getHitsInMax()){
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hitMax->identify())<<", "
                              <<Amg::toString(hitMax->positionInChamber())<<", dir: "<<Amg::toString(hitMax->directionInChamber()));
        }

        for (auto &seed : seeds) {
            ATH_MSG_VERBOSE("Seed tanTheta = "<<seed->tanTheta()<<", y0 = "<<seed->interceptY()
                         <<", tanPhi = "<<seed->tanPhi()<<", x0 = "<<seed->interceptX()<<", hits in the seed "<<seed->getHitsInMax().size());
        
            for(const auto& hit : seed->getHitsInMax()){
                ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hit->identify())<<", "
                              <<Amg::toString(hit->positionInChamber())<<", dir: "<<Amg::toString(hit->directionInChamber()));
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
