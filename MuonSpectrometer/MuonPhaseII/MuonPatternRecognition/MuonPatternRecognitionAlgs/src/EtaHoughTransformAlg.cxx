/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EtaHoughTransformAlg.h"

#include <MuonReadoutGeometryR4/SpectrometerSector.h>

#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

#include "Acts/Utilities/RangeXD.hpp"
#include <format>

namespace MuonR4{
    // helper to check if our trajectory traverses a chamber
    inline bool passesThrough(const SpacePointBucket::chamberLocation & loc, double y0, double tanBeta){
        double yCross = (y0 + loc.location().z() * tanBeta);  
        return (loc.minY() < yCross && yCross < loc. maxY()); 
    } 
    // determines the local residual when traversing a chamber 
    inline double proximity(const SpacePoint* dc, double y0, double tanBeta) {
        if (dc->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            return std::min(std::abs(HoughHelpers::Eta::houghParamMdtLeft(tanBeta, dc) - y0), 
                            std::abs(HoughHelpers::Eta::houghParamMdtRight(tanBeta, dc) - y0));
        }
        return std::abs(HoughHelpers::Eta::houghParamStrip(tanBeta, dc) - y0);
    }
    /** @brief Calculates how much of the unkknown coordinate along the tube range 
     *         is covered by the chamber of interest.
     * @param seedEdges: Array encoding the minimal [0] and maximal [1] possible position of the
     *                   muon along the tube /strip
     * @param chambEdges: Array encoding the minmal [1] and maximal [1] position along the strip
     *                    of a chamber */
    constexpr double chamberCoverage(const std::array<double,2>& seedEdges,
                                     const std::array<double, 2>& chambEdges) {
        // The seed is full embedded
        if (chambEdges[0] <= seedEdges[0] && chambEdges[1] >= seedEdges[1]) {
            return 1.;
        }
        /// Partial overlap. The lower side of the seed is covered by the chamber.
        else if (chambEdges[0]<= seedEdges[0]) {
            return (chambEdges[1] - seedEdges[0]) / (seedEdges[1] - seedEdges[0]);
        } 
        /// Reverse case with the upper side of the seed covered
        else if (chambEdges[1] >= seedEdges[1]) {
            return (seedEdges[1] - chambEdges[0]) / (seedEdges[1] - seedEdges[0]);
        }
        /// The chamber is fully embedded in the possible seed range
        else if (seedEdges[0] <= chambEdges[0] && seedEdges[1] >= chambEdges[1]) {
            return (chambEdges[1] -  chambEdges[0]) / (seedEdges[1] - seedEdges[0]);
        }
        return 0.;
    }

StatusCode EtaHoughTransformAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_maxima.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
    return StatusCode::SUCCESS;
}
StatusCode EtaHoughTransformAlg::execute(const EventContext& ctx) const {

    /// read the PRDs
    const SpacePointContainer* spacePoints{nullptr};
    ATH_CHECK(SG::get(spacePoints, m_spacePointKey, ctx));

    // book the output container
    SG::WriteHandle<EtaHoughMaxContainer> writeMaxima(m_maxima, ctx);
    ATH_CHECK(writeMaxima.record(std::make_unique<EtaHoughMaxContainer>()));

    const ActsGeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

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
            zmin = std::min(zmin, sp->localPosition().z());
            zmax = std::max(zmax, sp->localPosition().z());
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
            double y0l = hit->localPosition().y() - hit->localPosition().z() * tanThetaLeft;
            double y0r = hit->localPosition().y() - hit->localPosition().z() * tanThetaRight;
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
        }
        case xAOD::UncalibMeasType::MMClusterType:{
            return hit->measuresEta();
        }
        case xAOD::UncalibMeasType::sTgcStripType: {
            const auto* meas = static_cast<const xAOD::sTgcMeasurement*>(hit->primaryMeasurement()); 
            return meas->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip;
        } default:
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

    // now we propagate along the seed trajectory and collect crossed volumes 
    int expectedPrecisionChambers{0}, seenPrecisionChambers{0}; 
    bool hasTrig = false; 

    std::unordered_set<const MuonGMR4::MuonReadoutElement*> seenChambers{};
    std::set<std::pair<int,int>> seenLayers;
    using enum Acts::TrapezoidVolumeBounds::BoundValues;
    const double halfX = currentBucket.bucket->msSector()->bounds()->get(eHalfLengthXposY);
    /** Determine the minimal & maximal possible position along the tube / strip  */
    std::array<double, 2> tubeExtend{halfX, -halfX}; 

    auto addSeenHit = [&tubeExtend, &seenLayers, &seenChambers](const SpacePoint& sp,
                                                            const MuonGMR4::MuonReadoutElement* re,
                                                            const double sensorL,
                                                            const int mL, const int layer){
        seenLayers.emplace(mL, layer);
        seenChambers.insert(re);
        tubeExtend[0] = std::min(tubeExtend[0], sp.localPosition().x() - sensorL);
        tubeExtend[1] = std::max(tubeExtend[1], sp.localPosition().x() + sensorL);
    };
    for (const SpacePoint* SP : maximum.hitIdentifiers){       
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Maximum has associated hit in "
                      << m_idHelperSvc->toStringDetEl(SP->identify()));

        if (isPrecisionHit(SP)) {
             if (SP->type() == xAOD::UncalibMeasType::MdtDriftCircleType){
                 const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(SP->primaryMeasurement());
                 const MuonGMR4::MdtReadoutElement* re = dc->readoutElement();
                 addSeenHit(*SP, re, 0.5*re->activeTubeLength(dc->measurementHash()),
                            re->multilayer(), dc->tubeLayer());                
             } else if(SP->type() == xAOD::UncalibMeasType::MMClusterType){
                 const auto* clust = static_cast<const xAOD::MMCluster*>(SP->primaryMeasurement());
                 const MuonGMR4::MmReadoutElement* re = clust->readoutElement();
                 addSeenHit(*SP, re, 0.5*re->stripLayer(clust->measurementHash()).design().stripLength(clust->channelNumber()),
                            re->multilayer(), clust->gasGap());   
             } else if (SP->type() == xAOD::UncalibMeasType::sTgcStripType) {
                 const auto* clust = static_cast<const xAOD::sTgcMeasurement*>(SP->primaryMeasurement());
                 const MuonGMR4::sTgcReadoutElement* re = clust->readoutElement();
                 addSeenHit(*SP, re, 0.5*re->stripLayer(clust->measurementHash()).design().stripLength(clust->channelNumber()),
                            re->multilayer(), clust->gasGap());   
             }
         } else {
            seenChambers.insert(xAOD::muonReadoutElement(SP->primaryMeasurement()));
         }
     }
     // loop over all chambers in the bucket    
    for (const auto & muonChamber : currentBucket.bucket->chamberLocations()){      
        // skip any we don't touch 
        if (!passesThrough(muonChamber, maximum.y, maximum.x)) {
            ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" maximum does not cross "
                          << m_idHelperSvc->toStringDetEl(muonChamber.readoutEle()->identify()));
            continue; 
        }
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" maximum crosses "
            << m_idHelperSvc->toStringDetEl(muonChamber.readoutEle()->identify())<<", "
            <<(*muonChamber.bounds())<<", @: "<<Amg::toString(muonChamber.location()));
        // for MDT multilayers, we increase our expected number of crossed chambers / tubes
        const ActsTrk::DetectorType type = muonChamber.readoutEle()->detectorType();

        // now we check if we have a compatible measurement on our seed
        const bool hasHit = seenChambers.count(muonChamber.readoutEle());
        const bool precTech = (type == ActsTrk::DetectorType::Mdt || type == ActsTrk::DetectorType::Mm ||
                               type == ActsTrk::DetectorType::sTgc) ;
        if (hasHit) {
            // if we find an MDT hit, we increment the counter for seen chambers
            if  (precTech) {
                ++seenPrecisionChambers;
            } else {
                hasTrig = true;
                continue;
            }
        } else if (precTech) {
            /// Calculate the width / tube length at the centre crossing point 
            /// (maximum.x -> tanBeta, maximum.y -> y0)
            const double lowL = muonChamber.width(maximum.y + muonChamber.location().z() * maximum.x);
            const std::array<double, 2> chambEdges{muonChamber.location().x() - lowL,
                                                   muonChamber.location().x() + lowL};

            const double coverage = chamberCoverage(tubeExtend, chambEdges);
            if  (coverage < 0.95){
                ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Reject chamber due to partial coverage: "<<coverage
                            <<", chamb extend: "<<chambEdges[0]<<"-"<<chambEdges[1]
                            <<", tube extend: "<<tubeExtend[0]<<"-"<<tubeExtend[1]
                            <<", "<<(*muonChamber.readoutEle()->msSector()->bounds()));
                continue;
            }
        } else {
            continue;
        }
        ++expectedPrecisionChambers; 
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
    ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<": "<<currentBucket.bucket->msSector()->identString()<< 
                  ", seen prec: "<<seenPrecisionChambers<<", required: "<<minSeenPrecisionChambers
            <<" -- layers: "<<seenLayers.size()<<", required: "<<minLayers);
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
            <<":\n     Mean tanBeta was "<<tanThetaMean 
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
        std::vector<HoughHitType> hitList{max.hitIdentifiers.begin(), max.hitIdentifiers.end()};

        // apply a seed quality cut. 
        if (!passSeedQuality(bucket, max)) {
            // if seed visualisation is enabled, draw the rejected seed 
            if (m_visionTool.isEnabled()) {
                const HoughMaximum& houghMax{max.x, max.y, 1. *hitList.size(), std::move(hitList), bucket.bucket};
                const SegmentSeed seed{houghMax};
                MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{};  
                MuonValR4::IPatternVisualizationTool::PrimitiveVec primitivesForAcc{};  
                for (auto & chamber : bucket.bucket->chamberLocations()) {
                    primitives.push_back(MuonValR4::drawBox(chamber.minY(), chamber.minZ(), 
                                                            chamber.maxY(), chamber.maxZ(), kGray+2)); 
                    const Identifier detId{chamber.readoutEle()->identify()};
                    const int eta = m_idHelperSvc->stationEta(detId);
                    std::string chLabel = std::format("{:}{:1d}{:}{:2d}", m_idHelperSvc->stationNameString(detId), 
                                                      std::abs(eta), eta > 0? 'A' : 'C', m_idHelperSvc->stationPhi(detId));
                    switch (chamber.readoutEle()->detectorType()) {
                        case ActsTrk::DetectorType::Mdt: {
                            chLabel += std::format("M{:1d}", m_idHelperSvc->mdtIdHelper().multilayer(detId));
                        } default:
                            break;
                    }
                    primitives.push_back(MuonValR4::drawLabel(chLabel, chamber.minY(), chamber.maxZ() + 0.02,8));
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
        const SpacePointPerLayerSorter sorter{};
        std::ranges::stable_sort(hitList, sorter);
        // create hough maximum instance and add it to the event data for later writing! 
        const HoughMaximum& houghMax{data.maxima.emplace_back(max.x, max.y, nHits, std::move(hitList), bucket.bucket)};

        // if desired, visualise the result 
        if (m_visionTool.isEnabled()) {
            const SegmentSeed seed{houghMax};
            MuonValR4::IPatternVisualizationTool::PrimitiveVec primitives{}; 
            MuonValR4::IPatternVisualizationTool::PrimitiveVec primitivesForAcc{};   
            for (auto & chamber : bucket.bucket->chamberLocations()){
                primitives.push_back(MuonValR4::drawBox(chamber.minY(), chamber.minZ(), chamber.maxY(), chamber.maxZ(), kGray+2)); 
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
    // convert Gaudi::property to double to avoid deep copy in std::bind expression
    double resolutionTarget = m_targetResoIntercept; 
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
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2,  resolutionTarget), SP, precisionLayerIndex, w);
        data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtRight,
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2,  resolutionTarget), SP, precisionLayerIndex, w);
    } else {
        if (SP->measuresEta()) {
            data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamStrip,
                                                std::bind(HoughHelpers::Eta::houghWidthStrip, _1, _2, resolutionTarget), SP, 0, w * (
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
