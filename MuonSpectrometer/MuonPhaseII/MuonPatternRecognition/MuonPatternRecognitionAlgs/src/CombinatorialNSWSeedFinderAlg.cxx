/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CombinatorialNSWSeedFinderAlg.h"

#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>

#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternHelpers/CombinatorialSeedSolver.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MMCluster.h"



#include <ranges>
#include <vector>
#include <unordered_set>
#include <nlohmann/json.hpp>
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

void CombinatorialNSWSeedFinderAlg::findCombinatoricHits(const HitLayVec &combinatoricLayers, 
                                                         HitLayVec &combinatoricHitsVec) const {
  
    //try all the hits combinations from the layers for now (--rethink about optimized way)
    for(auto& hit0 : combinatoricLayers[0]){
        for(auto& hit1 : combinatoricLayers[1]){
            for(auto& hit2 : combinatoricLayers[2]){
                for(auto& hit3 : combinatoricLayers[3]){
                    combinatoricHitsVec.emplace_back(HitVec{hit0, hit1, hit2, hit3}); 
                }
            }
        }
    }
}


void CombinatorialNSWSeedFinderAlg::extendHits(const Amg::Vector3D &startPos, 
                                               const Amg::Vector3D &direction,
                                               const HitLayVec& stripHitsLayers, 
                                               HitVec &combinatoricHits,
                                               const ActsGeometryContext &gctx) const {
    


    for (unsigned int i = 0; i < stripHitsLayers.size(); i++) {
        // skip the layer with the hits that have been already considered -- not
        // use them for the extension
        const Identifier gasGapId = m_idHelperSvc->gasGapId(stripHitsLayers[i].front()->identify());
        /// Skip hits which already made it onto the input vector
        if (std::ranges::find_if(combinatoricHits, [&gasGapId,this](const SpacePoint* exist){
            return gasGapId == m_idHelperSvc->gasGapId(exist->identify());
        }) != combinatoricHits.end()) {
            continue;
        }

        const double refPlaneZ = stripHitsLayers[i].front()->positionInChamber().z();

        const Amg::Vector3D extrapPos = startPos + 
                            Amg::intersect<3>(startPos, direction, Amg::Vector3D::UnitZ(), refPlaneZ).value_or(0) *  direction;
        
        const Amg::Vector3D dirUp = Amg::dirFromAngles(direction.phi(), direction.theta() + m_windowTheta * Gaudi::Units::deg);
        const Amg::Vector3D dirDn = Amg::dirFromAngles(direction.phi(), direction.theta() - m_windowTheta * Gaudi::Units::deg);

        const Amg::Vector3D extrapPosUp = startPos + Amg::intersect<3>(startPos, dirUp, Amg::Vector3D::UnitZ(), refPlaneZ).value_or(0) *dirUp;
        const Amg::Vector3D extrapPosDn = startPos + Amg::intersect<3>(startPos, dirDn, Amg::Vector3D::UnitZ(), refPlaneZ).value_or(0) * dirDn;
        double dist = std::abs(extrapPosUp.y() - extrapPosDn.y());

        // intersect the layer in layer's z position
        //  hits on the layer that are inside the cone are picked
   
        double minPull{m_minPull};
        unsigned int indexOfHit = stripHitsLayers[i].size()+1;
       
        // loop over the hits on the same layer
        for (unsigned int j = 0; j < stripHitsLayers[i].size(); j++) {

            // for stereo layers we need to check the corners of the strips if they are both below or above the window -reject spacepoint
            // do the same also for the eta ones
            const Identifier &chId = stripHitsLayers[i].at(j)->identify();
            const MuonGMR4::MmReadoutElement *mmReadoutEle =
                m_detMgr->getMmReadoutElement(chId);
            // strip's edges in the local chamber's frame
            const Amg::Vector3D localRightEdge =
                mmReadoutEle->msSector()->globalToLocalTrans(gctx) *
                mmReadoutEle->rightStripEdge(gctx, chId);
            const Amg::Vector3D localLeftEdge =
                mmReadoutEle->msSector()->globalToLocalTrans(gctx) *
                mmReadoutEle->leftStripEdge(gctx, chId);
            ATH_MSG_VERBOSE("Corner of the strips right:"<<Amg::toString(localRightEdge)<<", left:"<<Amg::toString(localLeftEdge));
            // no reason to check for the other hits -- they are sorted in y when they belong on the same layer
            if (localRightEdge.y() > extrapPos.y() + 0.5 * dist && localLeftEdge.y() > extrapPos.y() + 0.5 * dist) {
                 
                    continue;
                //break;
            }
            // we do not want them if below the window - continue with the others
         
            if (!(localRightEdge.y() <= extrapPos.y() - 0.5 * dist && localLeftEdge.y() <= extrapPos.y() - 0.5 * dist)) {
                double pull = std::sqrt(SegmentFitHelpers::chiSqTermStrip(extrapPos, direction, *stripHitsLayers[i].at(j), msg()));
                if (pull < minPull) {
                    indexOfHit = j;
                    minPull = pull;
                }
            }
        }

        // complete the seed with the extended hits
        if (indexOfHit < stripHitsLayers[i].size()  && minPull < m_minPullThreshold) {
            ATH_MSG_VERBOSE("Extension successfull - hit" << m_idHelperSvc->toString(stripHitsLayers[i].at(indexOfHit)->identify())<<", pos: "
                              <<Amg::toString(stripHitsLayers[i].at(indexOfHit)->positionInChamber())<<", dir: "<<Amg::toString(stripHitsLayers[i].at(indexOfHit)->directionInChamber())<<" found with pull "<<minPull);
            combinatoricHits.push_back(stripHitsLayers[i].at(indexOfHit));
        }
    }
}

std::unique_ptr<SegmentSeed>
CombinatorialNSWSeedFinderAlg::buildSegmentSeed(HitVec& hits, 
                                                const AmgSymMatrix(2)& bMatrix,
                                                const HoughMaximum& max, 
                                                const ActsGeometryContext& gctx,
                                                const HitLayVec& stripHitsLayers) const {
    // we require at least four hits for the seeding
    if (hits.size() != minLayers) {
        ATH_MSG_VERBOSE("Wrong number of initial layers for seeding --they should be four");
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
                ATH_MSG_VERBOSE("Invalid seed - outside of the strip's length");
                return nullptr;
            }
        }
    }

    // extend the seed to the segment -- include hits from the other layers too

    extendHits(segPos, direction, stripHitsLayers, hits, gctx);
    return std::make_unique<SegmentSeed>(tanTheta, interceptY, tanPhi,
                                         interceptX, hits.size(),
                                         std::move(hits), max.parentBucket());
}

std::vector<std::unique_ptr<SegmentSeed>>
CombinatorialNSWSeedFinderAlg::findSeedsFromMaximum(const HoughMaximum &max, const ActsGeometryContext &gctx) const {
    // first sort the hits per layer from the maximum
    SpacePointPerLayerSorter hitLayers{max.getHitsInMax()};

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
            if (!simHit || std::abs(simHit->pdgId()) != 13) continue;
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
                  
                    HitLayVec result, layers{stripHitsLayers[i], stripHitsLayers[j], stripHitsLayers[k], stripHitsLayers[l]};
             
    
                    // each layer may have more than one hit - take the hit combinations                    
                    findCombinatoricHits(layers, result);               

                    // we have made sure to have hits from all the four layers -
                    // start by 4 hits for the seed and try to build the seed for the combinatorics found
                    for (auto &combinatoricHits : result) {
                        auto seed = buildSegmentSeed(combinatoricHits, bMatrix, max, gctx, stripHitsLayers);
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
