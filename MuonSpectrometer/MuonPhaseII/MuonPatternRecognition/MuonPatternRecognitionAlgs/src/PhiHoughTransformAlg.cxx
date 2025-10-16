/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PhiHoughTransformAlg.h"

#include <MuonReadoutGeometryR4/SpectrometerSector.h>


#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonPatternEvent/SegmentFitterEventData.h" 
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "Acts/Utilities/Helpers.hpp"

namespace {
    constexpr double resetVal = 1.e10;
}


namespace MuonR4{
using namespace SegmentFit;

StatusCode PhiHoughTransformAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_maxima.initialize());
    ATH_CHECK(m_segmentSeeds.initialize());
    ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}
void PhiHoughTransformAlg::prepareHoughPlane(HoughEventData& data) const {
    HoughPlaneConfig cfg;
    cfg.nBinsX = m_nBinsTanPhi;
    cfg.nBinsY = m_nBinsIntercept;
    // configure the peak finder for the phi-extension. 
    // Expect "shallow" maxima with 2-3 hits. 
    ActsPeakFinderForMuonCfg peakFinderCfg;
    peakFinderCfg.fractionCutoff = 0.4;
    peakFinderCfg.threshold = 2;    // 2D spacepoints receive a weight of 2
    peakFinderCfg.minSpacingBetweenPeaks = {0., 30.};
    data.houghPlane = std::make_unique<HoughPlane>(cfg);
    data.peakFinder = std::make_unique<ActsPeakFinderForMuon>(peakFinderCfg);
}

int PhiHoughTransformAlg::countIncompatibleEtaHits(const ActsPeakFinderForMuon::Maximum& phiMaximum,
                                                   const HoughMaximum& etaMaximum) const {
    std::unordered_map<const xAOD::UncalibratedMeasurement*, bool> foundEtas;
    // loop over the original eta maximum and check all hits measuring the eta-coordinate 
    for (auto& hit : etaMaximum.getHitsInMax()) {
        if (hit->measuresEta() && hit->measuresPhi()) {
            auto [iter, added] = foundEtas.emplace(hit->primaryMeasurement(), false);
            // test if the PRD for the eta-measurement appears on at least 
            // one space point of this phi extension.
            // This will be done for all space points containing the eta-PRD
            iter->second |= phiMaximum.hitIdentifiers.count(hit);
        }
    }
    // count the number of eta PRD not compatible with this extension.
    return std::ranges::count_if(foundEtas,
                         [](const std::pair<const xAOD::UncalibratedMeasurement*, bool>& p) {
                             return !p.second;
                         });
}
std::unique_ptr<SegmentSeed> 
    PhiHoughTransformAlg::buildSegmentSeed(const HoughMaximum & etaMax,  
                                           const ActsPeakFinderForMuon::Maximum & phiMax) const {
        // book a new hit list
        std::vector<HoughHitType> hitsOnMax{}; 
        // copy the pure eta hits onto the hit list 
        std::ranges::copy_if(etaMax.getHitsInMax(), 
                             std::back_inserter(hitsOnMax), [](const HoughHitType &hit){
            return (hit->measuresEta() && !hit->measuresPhi()); 
        }); 
        // and then add all hits (2D and pure phi) from the phi-extension to it 
        hitsOnMax.insert(hitsOnMax.end(), phiMax.hitIdentifiers.begin(), phiMax.hitIdentifiers.end()); 
        // use this to construct the segment seed
        SpacePointPerLayerSorter sorter{};
        std::ranges::stable_sort(hitsOnMax, sorter);
        return std::make_unique<SegmentSeed>(etaMax.tanBeta(), etaMax.interceptY(), phiMax.x, phiMax.y, hitsOnMax.size(), std::move(hitsOnMax), etaMax.parentBucket());         
}

void PhiHoughTransformAlg::preProcessMaximum(const ActsGeometryContext& gctx,
                                             const HoughMaximum & maximum,
                                             HoughEventData& eventData) const{
    // reset the event data 
    eventData.phiHitsOnMax = 0; 
    eventData.searchSpaceTanAngle = std::make_pair(resetVal, -resetVal); 
    eventData.searchSpaceIntercept = std::make_pair(resetVal, -resetVal); 
    // loop over the measurements on the maximum
    for (auto hit : maximum.getHitsInMax()) {
        // reject the pure eta measurements - not relevant here
        if (!hit->measuresPhi()) {
            continue;
        }
        // find the direction of the IP viewed from the sector frame 
        const Amg::Vector3D extrapDir = (hit->localPosition() - hit->msSector()->globalToLocalTrans(gctx).translation()).unit();
        ATH_MSG_VERBOSE("Direction "<<Amg::toString(extrapDir));
        // express the x location of our phi hits on the chamber plane (z = 0) when projecting from the beam spot
        std::optional<double> dummyIntercept = Amg::intersect<3>(hit->localPosition(), extrapDir, Amg::Vector3D::UnitZ(),0); 
        double x0 = (hit->localPosition() + dummyIntercept.value_or(0) * extrapDir).x(); 
        // now we can obtain the most likely tan(phi) via the pointing vector from the origin to our hit
        double tanAlpha = houghTanAlpha(extrapDir); 
        // update our search space with this info 
        eventData.updateSearchWindow(eventData.searchSpaceTanAngle, tanAlpha);
        eventData.updateSearchWindow(eventData.searchSpaceIntercept, x0);
        // and increment the hit counter
        ++eventData.phiHitsOnMax; 
    }
    // now use the results from the individual hits to define an axis range adapted to the binning we desire 
    double chamberCenter = (eventData.searchSpaceIntercept.second + eventData.searchSpaceIntercept.first) * 0.5; 
    double searchStart = chamberCenter - 0.5 * eventData.houghPlane->nBinsY() * m_targetResoIntercept; 
    double searchEnd = chamberCenter + 0.5 * eventData.houghPlane->nBinsY()  * m_targetResoIntercept; 
    // Protection for very wide buckets - if the search space does not cover all of the bucket, widen the bin size 
    // so that we cover everything  
    searchStart = std::min(searchStart, eventData.searchSpaceIntercept.first- m_minSigmasSearchIntercept * m_targetResoIntercept); 
    searchEnd = std::max(searchEnd, eventData.searchSpaceIntercept.second + m_minSigmasSearchIntercept * m_targetResoIntercept); 
    // also treat tan(phi) 
    double tanPhiMean = 0.5 * (eventData.searchSpaceTanAngle.first + eventData.searchSpaceTanAngle.second); 
    double searchStartTanPhi = tanPhiMean - 0.5 *  eventData.houghPlane->nBinsX() * m_targetResoTanPhi; 
    double searchEndTanPhi = tanPhiMean + 0.5* eventData.houghPlane->nBinsX() * m_targetResoTanPhi; 
    searchStartTanPhi = std::min(searchStartTanPhi, eventData.searchSpaceTanAngle.first- m_minSigmasSearchTanPhi * m_targetResoTanPhi); 
    searchEndTanPhi = std::max(searchEndTanPhi, eventData.searchSpaceTanAngle.second + m_minSigmasSearchTanPhi * m_targetResoTanPhi); 

    // and update the axis ranges for the search space according to our results
    eventData.currAxisRanges =
    Acts::HoughTransformUtils::HoughAxisRanges{searchStartTanPhi, searchEndTanPhi, searchStart, searchEnd};
    ATH_MSG_VERBOSE("Accumulator search window: tanAlpha: ["<<searchStartTanPhi<<";"<<searchEndTanPhi<<"], x0: ["
                <<searchStart<<";"<<searchEnd<<"]");
}

std::vector<ActsPeakFinderForMuon::Maximum> 
    PhiHoughTransformAlg::findRankedSegmentSeeds(const EventContext& ctx, 
                                                 HoughEventData & eventData, const HoughMaximum & maximum) const{
    std::unordered_map<int, std::vector<ActsPeakFinderForMuon::Maximum>>  rankedSeeds;  
    using namespace std::placeholders; 
    // reset the accumulator
    eventData.houghPlane->reset();
    // fill the accumulator with the phi measurements   
    for (auto hit : maximum.getHitsInMax()){
        if (!hit->measuresPhi()) {
            ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hit->identify())<<" does not have a phi measurement");
            continue;
        }
        ATH_MSG_VERBOSE("Fill hit "<<m_idHelperSvc->toString(hit->identify())<<", "<<Amg::toString(hit->localPosition()));
        eventData.houghPlane->fill<HoughHitType>(
            hit, eventData.currAxisRanges,
            HoughHelpers::Phi::houghParamStrip,
            std::bind(HoughHelpers::Phi::houghWidthStrip, _1, _2, m_targetResoIntercept), hit, 0, 
            // up-weigh 2D spacepoints w.r.t 1D phi hits to prevent 
            // discarding measurements known to be compatible in eta 
            (hit->measuresEta() ? 2.0 : 1.0) / (m_downWeightMultiplePrd? hit->nPhiInstanceCounts() : 1)
            );
    }
    // run the peak finder 
    auto foundMaxPhi = eventData.peakFinder->findPeaks(*eventData.houghPlane, eventData.currAxisRanges);
    if (m_visionTool.isEnabled()) {
        m_visionTool->visualizeAccumulator(ctx,*eventData.houghPlane, eventData.currAxisRanges, foundMaxPhi,
                                           "#phi accumulator");
    }
    // now rank the found peaks by the number of eta-compatible PRDs they would discard. 
    for (const auto& solution : foundMaxPhi) {
        // for each solution, count how many eta PRDs would not be compatible with this phi extension. 
        // rank by the lowest number of such "holes". 
        rankedSeeds[countIncompatibleEtaHits(solution, maximum)].push_back(solution); 
    }
    // return only the best solution(s) 
    auto best = rankedSeeds.begin(); 
    // and apply the maximum hole cut. 
    if (best != rankedSeeds.end() && best->first <= m_maxEtaHolesOnMax){
        return best->second; 
    } 
    return {}; 
}            
std::unique_ptr<SegmentSeed> 
    PhiHoughTransformAlg::recoverSinglePhiMax(HoughEventData & data, const HoughMaximum & maximum) const{
    // recovers cases of a single phi hit assuming a straight 
    // line extrapolation from the beam line to the phi measurement
    std::vector<HoughHitType> hits{maximum.getHitsInMax()};
    SpacePointPerLayerSorter sorter{};
    std::ranges::stable_sort(hits, sorter);
    return std::make_unique<SegmentSeed>(maximum.tanBeta(), maximum.interceptY(), 
                        data.searchSpaceTanAngle.first, 
                        data.searchSpaceIntercept.first, 
                        maximum.getCounts(), 
                        std::move(hits), maximum.parentBucket()); 

} 


StatusCode PhiHoughTransformAlg::execute(const EventContext& ctx) const {
   
    // read the inputs
    const EtaHoughMaxContainer* maxima{nullptr};
    ATH_CHECK(SG::get(maxima, m_maxima, ctx));

    const ActsGeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    // book the event data object
    HoughEventData eventData{};

    // prepare the accumulator
    prepareHoughPlane(eventData);

    // prepare our output collection
    SG::WriteHandle writeMaxima{m_segmentSeeds, ctx};
    ATH_CHECK(writeMaxima.record(std::make_unique<SegmentSeedContainer>()));

    // loop over the previously found eta-maxima for each station
    for (const HoughMaximum* max : *maxima) {
        // for each maximum, pre-process 
        ATH_MSG_VERBOSE("Search extra phi hits on maximum "<<max->msSector()->identString()<<", tanBeta: "<<max->tanBeta()
                     <<", y0: "<<max->interceptY());
        if (m_visionTool.isEnabled() && msgLvl(MSG::VERBOSE)) {
            for (const auto& truth : m_visionTool->getLabeledSegments(max->getHitsInMax())) {
                const Parameters truthPars = localSegmentPars(*truth);
                ATH_MSG_VERBOSE("Truth parameters "<<toString(truthPars)<<", tanAlpha: "
                            <<houghTanAlpha(Amg::dirFromAngles(truthPars[Acts::toUnderlying(ParamDefs::phi)],
                                                             truthPars[Acts::toUnderlying(ParamDefs::theta)])));
            }
        }
        preProcessMaximum(*gctx, *max, eventData); 
        bool foundSolution=false; 
        // if we have enough hits, run a phi transform 
        if (eventData.phiHitsOnMax > 1){        
            std::vector<ActsPeakFinderForMuon::Maximum> rankedSeeds = findRankedSegmentSeeds(ctx, eventData, *max); 
            for (auto & phiSolution : rankedSeeds){
                foundSolution = true; 
                const SegmentSeed* seed {writeMaxima->push_back(buildSegmentSeed(*max, phiSolution))};
                if (m_visionTool.isEnabled()) {
                    m_visionTool->visualizeSeed(ctx, *seed, "#phi pattern seed");
                }  
            }
        }
        ATH_MSG_VERBOSE("Solution found: "<<foundSolution);
        // if we do not have at least two phi-hits for a proper transform: 
        if (!foundSolution){
            // if we have a single phi hit, we can approximate the phi 
            // solution using the beam spot (as the IP is far). 
            // This is steered by a flag, and not appropriate for splashes
            // or cosmics. 
            if (m_recoverSinglePhiWithBS && eventData.phiHitsOnMax == 1){                
               const SegmentSeed* singleMax{writeMaxima->push_back(recoverSinglePhiMax(eventData,*max))};
               if (m_visionTool.isEnabled()) {
                    m_visionTool->visualizeSeed(ctx, *singleMax, "Single #phi hit recovery");
               } 
            }
            // otherwise we have no phi-solution, and we fall back to writing a 1D eta-maximum 
            else{ 
                writeMaxima->push_back(std::make_unique<SegmentSeed>(*max)); 
            }
        }   
    }
    // add the maxima for this station to the output

    return StatusCode::SUCCESS;
}
}
