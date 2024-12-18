/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EtaHoughTransformAlg.h"

#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <StoreGate/ReadCondHandle.h>

#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/UtilFunctions.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"

namespace MuonR4{
EtaHoughTransformAlg::EtaHoughTransformAlg(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode EtaHoughTransformAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_maxima.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
    return StatusCode::SUCCESS;
}

template <class ContainerType>
StatusCode EtaHoughTransformAlg::retrieveContainer(const EventContext& ctx, 
                                                  const SG::ReadHandleKey<ContainerType>& key,
                                                  const ContainerType*& contToPush) const {
    contToPush = nullptr;
    if (key.empty()) {
        ATH_MSG_VERBOSE("No key has been parsed for object "
                        << typeid(ContainerType).name());
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle readHandle{key, ctx};
    ATH_CHECK(readHandle.isPresent());
    contToPush = readHandle.cptr();
    return StatusCode::SUCCESS;
}

StatusCode EtaHoughTransformAlg::execute(const EventContext& ctx) const {

    /// read the PRDs
    const SpacePointContainer* spacePoints{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_spacePointKey, spacePoints));

    // book the output container
    SG::WriteHandle<EtaHoughMaxContainer> writeMaxima(m_maxima, ctx);
    ATH_CHECK(writeMaxima.record(std::make_unique<EtaHoughMaxContainer>()));

    const ActsGeometryContext* gctx{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));

    HoughEventData data{};

    /// pre-populate the event data - sort PRDs by station
    preProcess(ctx, *gctx, *spacePoints, data);

    /// book the hough plane
    prepareHoughPlane(data);
    /// now perform the actual HT for each station
    for (auto& [station, stationHoughBuckets] : data.houghSetups) {
        // reset the list of maxima
        for (auto& bucket : stationHoughBuckets) {
            processBucket(ctx, data, bucket);
        }
        for (HoughMaximum& max : data.maxima) {
            writeMaxima->push_back(std::make_unique<HoughMaximum>(std::move(max)));
        }
        data.maxima.clear();
    }
    std::stable_sort(writeMaxima->begin(), writeMaxima->end(), 
              [](const HoughMaximum* a, const HoughMaximum* b){                
                return (*a->parentBucket()) < (*b->parentBucket());
              });
    return StatusCode::SUCCESS;
}
void EtaHoughTransformAlg::preProcess(const EventContext& ctx,
                                      const ActsGeometryContext& gctx,
                                      const SpacePointContainer& spacePoints,
                                      HoughEventData& data) const {

    ATH_MSG_DEBUG("Load " << spacePoints.size() << " space point buckets");
    for (const SpacePointBucket* bucket : spacePoints) {
        if (m_visionTool.isEnabled()) {
            m_visionTool->visualizeBucket(ctx, *bucket, "bucket");
        }
        std::vector<HoughSetupForBucket>& buckets = data.houghSetups[bucket->front()->msSector()];        
        HoughSetupForBucket& hs{buckets.emplace_back(bucket)};
        const Amg::Transform3D globToLoc{hs.bucket->msSector()->globalToLocalTrans(gctx)};
        Amg::Vector3D leftSide  = globToLoc.translation() - (hs.bucket->coveredMin() * Amg::Vector3D::UnitY());
        Amg::Vector3D rightSide = globToLoc.translation() - (hs.bucket->coveredMax() * Amg::Vector3D::UnitY());

        // get the average z of our hits and use it to correct our angle estimate
        double zmin{1.e9}, zmax{-1.e9};
        for (const std::shared_ptr<MuonR4::SpacePoint> & sp : *bucket) {
            zmin = std::min(zmin, sp->positionInChamber().z());
            zmax = std::max(zmax, sp->positionInChamber().z());
        }
        const double z = 0.5*(zmin + zmax);

        // estimate the angle, adding extra tolerance based on our target resolution
        const double tanThetaLeft  = (leftSide.y()  - m_targetResoIntercept) / (leftSide.z()  - z) - m_targetResoTanTheta;
        const double tanThetaRight = (rightSide.y() + m_targetResoIntercept) / (rightSide.z() - z) + m_targetResoTanTheta;
        hs.searchWindowTanAngle = {tanThetaLeft, tanThetaRight};
        double ymin{1e9}, ymax{-1e9}; 

        /// Project the hits onto the center (z=0) axis of the chamber, using 
        /// our guesstimate of tan(theta) 
        for (const std::shared_ptr<MuonR4::SpacePoint> & hit : *bucket){
            // two estimates: For the two extrema of tan(theta) resulting from the guesstimate
            double y0l = hit->positionInChamber().y() - hit->positionInChamber().z() * tanThetaLeft;
            double y0r = hit->positionInChamber().y() - hit->positionInChamber().z() * tanThetaRight;
            // pick the widest envelope
            ymin=std::min(ymin, std::min(y0l, y0r) - m_targetResoIntercept); 
            ymax=std::max(ymax, std::max(y0l, y0r) + m_targetResoIntercept); 
        }
        hs.searchWindowIntercept = {ymin, ymax};
    }
}
bool EtaHoughTransformAlg::isPrecisionHit(const HoughHitType& hit) {
    switch (hit->type()){
        case xAOD::UncalibMeasType::MdtDriftCircleType: {
            const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->primaryMeasurement());
            return dc->status() == Muon::MdtDriftCircleStatus::MdtStatusDriftTime;
            break;
        }
        case xAOD::UncalibMeasType::MMClusterType:
        case xAOD::UncalibMeasType::sTgcStripType:
            return hit->measuresEta();
            break;
        default:
            break;
    }
    return false;
}

void EtaHoughTransformAlg::prepareHoughPlane(HoughEventData& data) const {
    HoughPlaneConfig cfg;
    cfg.nBinsX = m_nBinsTanTheta;
    cfg.nBinsY = m_nBinsIntercept;
    ActsPeakFinderForMuonCfg peakFinderCfg;
    peakFinderCfg.fractionCutoff = m_peakFractionCutOff;
    peakFinderCfg.threshold = m_peakThreshold;
    peakFinderCfg.minSpacingBetweenPeaks = {m_minMaxDistTheta, m_minMaxDistIntercept};
    data.houghPlane = std::make_unique<HoughPlane>(cfg);
    data.peakFinder = std::make_unique<ActsPeakFinderForMuon>(peakFinderCfg);
}

bool EtaHoughTransformAlg::passSeedQuality (const HoughSetupForBucket& currentBucket, const MuonR4::ActsPeakFinderForMuon::Maximum & maximum) const{

    // helper to check if a space-point is inside a chamber 
    auto isInside = [](const SpacePointBucket::chamberLocation & loc, const Amg::Vector3D & SP){
        return (loc.yLeft < SP.y() &&  SP.y() < loc.yRight && loc.zBottom < SP.z() && SP.z() < loc.zTop);  
    };

    // helper to check if our trajectory traverses a chamber
    auto passesThrough = [](const SpacePointBucket::chamberLocation & loc, double y0, double tanTheta){
        double yCross = (y0 + 0.5 * (loc.zBottom+loc.zTop) * tanTheta);  
        return (loc.yLeft < yCross && yCross < loc.yRight); 
    };

    // determines the local residual when traversing a chamber 
    auto proximity = []( HoughHitType dc, double y0, double tanTheta){
        return std::min(std::abs(HoughHelpers::Eta::houghParamMdtLeft(tanTheta,dc) - y0), std::abs(HoughHelpers::Eta::houghParamMdtRight(tanTheta,dc) - y0)); 
    };

    // now we propagate along the seed trajectory and collect crossed volumes 
    int expectedPrecisionChambers = 0; 
    int seenPrecisionChambers = 0; 
    bool hasTrig = false; 
    // loop over all chambers in the bucker 
    for (auto & mdtChamber : currentBucket.bucket->chamberLocations()){
        // skip any we don't touch 
        if (!passesThrough(mdtChamber, maximum.y, maximum.x)) continue; 
        // for MDT multilayers, we increase our expected number of crossed chambers / tubes
        if (mdtChamber.type == ActsTrk::DetectorType::Mdt){
            ++expectedPrecisionChambers; 
        }
        // now we check if we have a compatible measurement on our seed
        bool hasHit = false; 
        for (auto & SP : maximum.hitIdentifiers){
            // the hit should be inside the current volume and the local residual should be 
            // compatible with the desired resolution 
            if (isInside(mdtChamber, SP->positionInChamber()) && proximity(SP,maximum.y,maximum.x) < 2. * m_targetResoIntercept){
                hasHit=true;
                break;
            }
        }
        // if we find an MDT hit, we increment the counter for seen chambers
        if (mdtChamber.type == ActsTrk::DetectorType::Mdt){
            seenPrecisionChambers += (hasHit); 
        }
        // for trigger hits, we set a flag indicating we have at least one 
        else hasTrig |= hasHit; 
    }
    // now count the total number of MDT tube layers we collected on our seed 
    std::set<std::pair<int,int>> seenLayers; 
    for (auto & SP : maximum.hitIdentifiers){
        if (SP->type() == xAOD::UncalibMeasType::MdtDriftCircleType){
            // apply a compatibility window - enforce hits are at least reasonably close 
            if (proximity(SP,maximum.y,maximum.x) < 2. * m_targetResoIntercept){
                const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(SP->primaryMeasurement());
                seenLayers.emplace(dc->readoutElement()->multilayer(), dc->tubeLayer()); 
            }
        }
    }
    // compute the minimum number of requested precision layers
    // the integer division will round down (resulting cut: 2 for single-ML, 4 for dual-ML)  
    int minLayers = seenLayers.size() / 2 + 1;  
    // require 2 precision chambers with measurements, except if we only cross one precision multilayer in total 
    int minSeenPrecisionChambers = (expectedPrecisionChambers > 1) + 1; 
    // if we have at least one trigger hit, we loosen the requirements on precision hits and chambers
    if (hasTrig) {
        minLayers -= 1;
        minSeenPrecisionChambers = 1;
    }
    
    return seenPrecisionChambers >= minSeenPrecisionChambers && (int)seenLayers.size() >= minLayers; 
}


void EtaHoughTransformAlg::processBucket(const EventContext& ctx,
                                         HoughEventData& data, 
                                         HoughSetupForBucket& bucket) const {
    /// tune the search space

    double chamberCenter = 0.5 * (bucket.searchWindowIntercept.first +
                                  bucket.searchWindowIntercept.second);
    // build a symmetric window around the (geometric) chamber center so that
    // the bin width is equivalent to our target resolution
    double searchStart = chamberCenter - 0.5 * data.houghPlane->nBinsY() * m_targetResoIntercept;
    double searchEnd = chamberCenter + 0.5 * data.houghPlane->nBinsY() * m_targetResoIntercept;
    // Protection for very wide buckets - if the search space does not cover all
    // of the bucket, widen the bin size so that we cover everything
    searchStart = std::min(searchStart, bucket.searchWindowIntercept.first -
                                        m_minSigmasSearchIntercept * m_targetResoIntercept);
    searchEnd = std::max(searchEnd, bucket.searchWindowIntercept.second +
                                        m_minSigmasSearchIntercept * m_targetResoIntercept);
    // also treat tan(theta)
    double tanThetaMean = 0.5 * (bucket.searchWindowTanAngle.first +
                                 bucket.searchWindowTanAngle.second);
    double searchStartTanTheta = tanThetaMean - 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    double searchEndTanTheta = tanThetaMean + 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    searchStartTanTheta = std::min(searchStartTanTheta, bucket.searchWindowTanAngle.first - 
                                   m_minSigmasSearchTanTheta * m_targetResoTanTheta);
    searchEndTanTheta = std::max(searchEndTanTheta, bucket.searchWindowTanAngle.second +
                                 m_minSigmasSearchTanTheta * m_targetResoTanTheta);

    data.currAxisRanges = Acts::HoughTransformUtils::HoughAxisRanges{
        searchStartTanTheta, searchEndTanTheta, searchStart, searchEnd};

    data.houghPlane->reset();
    for (const SpacePointBucket::value_type& hit : *(bucket.bucket)) {
        fillFromSpacePoint(data, hit.get());
    }
    auto maxima = data.peakFinder->findPeaks(*(data.houghPlane), data.currAxisRanges);
    if (m_visionTool.isEnabled()) {
        m_visionTool->visualizeAccumulator(ctx, *data.houghPlane, data.currAxisRanges, maxima,
                                           "#eta Hough accumulator");
    }
    if (maxima.empty()) {
        ATH_MSG_DEBUG("Station "<<bucket.bucket->msSector()->identString()
            <<":\n     Mean tanTheta was "<<tanThetaMean 
            << " and my intercept "<<chamberCenter 
            <<", with hits in the bucket in "<< bucket.bucket->coveredMin() 
            <<" - "<<bucket.bucket->coveredMax() 
            <<". The bucket found a search range of ("
            <<bucket.searchWindowTanAngle.first<<" - "
            <<bucket.searchWindowTanAngle.second<<") and ("
            <<bucket.searchWindowIntercept.first<<" - "
            <<bucket.searchWindowIntercept.second 
            <<") , and my final search range is ["
            <<searchStartTanTheta<<" - "<<searchEndTanTheta
            <<"] and ["<<searchStart<<" - "<<searchEnd
            <<"] with "<<m_nBinsTanTheta<<" and "
            <<m_nBinsIntercept<<" bins.");  
        return;
    }

    // remember used hits - assign only to first maximum when counting
    // precision hits
    std::set<HoughHitType> seenHits;

    // now clean up and potentially write the maxima
    for (const auto& max : maxima) {

        // precision hit cut, using only the measurements on the hough maximum
        unsigned int nPrec{0};
        auto toBins = [&data](double x, double y){
            return std::make_pair(
                Acts::HoughTransformUtils::binIndex(data.currAxisRanges.xMin, data.currAxisRanges.xMax, data.houghPlane->nBinsX(), x), 
                Acts::HoughTransformUtils::binIndex(data.currAxisRanges.yMin, data.currAxisRanges.yMax, data.houghPlane->nBinsY(), y)
            );
        };
        auto accumulatorBins = toBins(max.x,max.y); 
        for (const HoughHitType& hit : data.houghPlane->hitIds(accumulatorBins.first, accumulatorBins.second)) {
            auto res = seenHits.emplace(hit); 
            if (res.second){
                nPrec += isPrecisionHit(hit);
            }
        }
        if (nPrec < m_nPrecHitCut) {
            ATH_MSG_VERBOSE("The maximum did not pass the precision hit cut");
            continue;
        }      

        // convert the set of hit identifiers from ACTS to the vector we need later 
        std::vector<HoughHitType> hitList;
        hitList.reserve(max.hitIdentifiers.size());
        for (auto & hit : max.hitIdentifiers){
            hitList.push_back(hit);
        }

        // apply a seed quality cut. 
        if (!passSeedQuality(bucket,max)) {
            // if seed visualisation is enabled, draw the rejected seed 
            if (m_visionTool.isEnabled()) {
                const HoughMaximum& houghMax{max.x, max.y, (double)hitList.size(), std::move(hitList), bucket.bucket};
                const SegmentSeed seed{houghMax};
                MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{};  
                MuonValR4::IPatternVisualizationTool::PrimitiveVec primitivesForAcc{};  
                for (auto & chamber : bucket.bucket->chamberLocations()){
                    primitives.push_back(MuonValR4::drawBox(chamber.yLeft, chamber.zBottom, chamber.yRight, chamber.zTop, kGray+2)); 
                }
       
                primitives.push_back(MuonValR4::drawLabel(std::format("Missed seed - score {}, layer score {}, comprising {} measurements ",data.houghPlane->nHits(accumulatorBins.first, accumulatorBins.second),data.houghPlane->nLayers(accumulatorBins.first, accumulatorBins.second),hitList.size()),0.05,0.03,12)); 
       
                primitivesForAcc.push_back(MuonValR4::drawLabel(std::format("Missed seed - score {}, layer score {}, comprising {} measurements ",data.houghPlane->nHits(accumulatorBins.first, accumulatorBins.second),data.houghPlane->nLayers(accumulatorBins.first, accumulatorBins.second),hitList.size()),0.05,0.03,12)); 
                m_visionTool->visualizeAccumulator(ctx, *data.houghPlane, data.currAxisRanges, {max},
                                "MissedAccumulator", std::move(primitivesForAcc));
                m_visionTool->visualizeSeed(ctx, seed, "Missed seed",std::move(primitives));
            }
            continue;
        }
        
        // this seed looks good! Let's finalise it 
        size_t nHits = hitList.size();
        // add phi measurements - will be filtered for compatibility in separate algorithm
        extendWithPhiHits(hitList, bucket);
        // sort hits by layer 
        sortByLayer(hitList);
        // create hough maximum instance and add it to the event data for later writing! 
        const HoughMaximum& houghMax{data.maxima.emplace_back(max.x, max.y, nHits, std::move(hitList), bucket.bucket)};

        // if desired, visualise the result 
        if (m_visionTool.isEnabled()) {
            const SegmentSeed seed{houghMax};
            MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{}; 
            MuonValR4::IPatternVisualizationTool::PrimitiveVec primitivesForAcc{};   
            for (auto & chamber : bucket.bucket->chamberLocations()){
                primitives.push_back(MuonValR4::drawBox(chamber.yLeft, chamber.zBottom, chamber.yRight, chamber.zTop, kGray+2)); 
            }
            
            primitives.push_back(MuonValR4::drawLabel(std::format("score {}, layer score {}, comprising {} measurements. wx = {:.2f}, wy = {:.1f} ",data.houghPlane->nHits(accumulatorBins.first, accumulatorBins.second),data.houghPlane->nLayers(accumulatorBins.first, accumulatorBins.second),hitList.size(), max.wx, max.wy),0.05,0.03,12)); 

            primitivesForAcc.push_back(MuonValR4::drawLabel(std::format("score {}, layer score {}, comprising {} measurements. wx = {:.2f}, wy = {:.1f} ",data.houghPlane->nHits(accumulatorBins.first, accumulatorBins.second),data.houghPlane->nLayers(accumulatorBins.first, accumulatorBins.second),hitList.size(), max.wx / m_targetResoTanTheta, max.wy / m_targetResoIntercept),0.05,0.03,12)); 

            m_visionTool->visualizeAccumulator(ctx, *data.houghPlane, data.currAxisRanges, {max},"#eta Hough accumulator", std::move(primitivesForAcc));
            m_visionTool->visualizeSeed(ctx, seed, "#eta-HoughSeed", std::move(primitives));
        }
    }
}
void EtaHoughTransformAlg::fillFromSpacePoint(HoughEventData& data, const HoughHitType& SP) const {

    using namespace std::placeholders; 
    double w = 1.0; 
    // downweight RPC measurements in the barrel relative to MDT  
    if (SP->primaryMeasurement()->type() == xAOD::UncalibMeasType::RpcStripType){
        w = 0.5; 
    }
    if (SP->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
        // if invalid time, do not count this hit towards a potential peak.
        // The hits will still be included in a potential maximum formed by valid hits,
        // for later recovery.  
        if (!isPrecisionHit(SP)) w = 0;
        const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(SP->primaryMeasurement());
        // dummy index for precision layer counting within the hough plane 
        const unsigned precisionLayerIndex = (dc->readoutElement()->multilayer() * 10 + dc->tubeLayer());
        data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtLeft,
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2,  m_targetResoIntercept), SP, precisionLayerIndex, w);
        data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtRight,
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2,  m_targetResoIntercept), SP, precisionLayerIndex, w);
    } else {
        if (SP->measuresEta()) {
            data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamStrip,
                                                std::bind(HoughHelpers::Eta::houghWidthStrip, _1, _2, m_targetResoIntercept), SP, 0, w * (
                                                m_downWeightMultiplePrd ? 1.0 / SP->nEtaInstanceCounts() : 1.));
        }
    }
}
void EtaHoughTransformAlg::extendWithPhiHits(std::vector<HoughHitType>& hitList, 
                                             HoughSetupForBucket& bucket) const {
    for (const SpacePointBucket::value_type& hit : *bucket.bucket) {
        if (!hit->measuresEta()) {
            hitList.push_back(hit.get());
        }
    }
}
}
