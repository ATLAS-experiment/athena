/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EtaHoughTransformAlg.h"

#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <StoreGate/ReadCondHandle.h>

#include "MuonPatternHelpers/HoughHelperFunctions.h"

namespace {
    void sortHits(std::vector<MuonR4::HoughHitType>& hits) {
        std::ranges::sort(hits, 
              [](const MuonR4::HoughHitType&a, const MuonR4::HoughHitType& b){   
                    const Amg::Vector3D& hitA{a->positionInChamber()};
                    const Amg::Vector3D& hitB{b->positionInChamber()};
                    constexpr double layerTol = 1.*Gaudi::Units::mm;
                    if (std::abs(hitA.z() - hitB.z()) > layerTol) {
                        return hitA.z() < hitB.z();
                    }
                    return hitA.y() < hitB.y();
              });
    }
}

namespace MuonR4{
EtaHoughTransformAlg::EtaHoughTransformAlg(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode EtaHoughTransformAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_maxima.initialize());

    return StatusCode::SUCCESS;
}

template <class ContainerType>
StatusCode EtaHoughTransformAlg::retrieveContainer(
    const EventContext& ctx, const SG::ReadHandleKey<ContainerType>& key,
    const ContainerType*& contToPush) const {
    contToPush = nullptr;
    if (key.empty()) {
        ATH_MSG_VERBOSE("No key has been parsed for object "
                        << typeid(ContainerType).name());
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle<ContainerType> readHandle{key, ctx};
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
    ATH_CHECK(preProcess(*gctx, *spacePoints, data));

    /// book the hough plane
    ATH_CHECK(prepareHoughPlane(data));
    /// now perform the actual HT for each station
    for (auto& [station, stationHoughBuckets] : data.houghSetups) {
        // reset the list of maxima
        for (auto& bucket : stationHoughBuckets) {
            ATH_CHECK(processBucket(data, bucket));
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
StatusCode EtaHoughTransformAlg::preProcess(const ActsGeometryContext& gctx,
                                            const SpacePointContainer& spacePoints,
                                            HoughEventData& data) const {

    ATH_MSG_DEBUG("Load " << spacePoints.size() << " space point buckets");
    for (const SpacePointBucket* bucket : spacePoints) {
        std::vector<HoughSetupForBucket>& buckets = data.houghSetups[bucket->front()->chamber()];        
        HoughSetupForBucket& hs{buckets.emplace_back(bucket)};
        const Amg::Transform3D globToLoc{hs.bucket->chamber()->globalToLocalTrans(gctx)};
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
    return StatusCode::SUCCESS;
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
            return true;
            break;
        default:
            break;
    }
    return false;
}

StatusCode EtaHoughTransformAlg::prepareHoughPlane(HoughEventData& data) const {
    HoughPlaneConfig cfg;
    cfg.nBinsX = m_nBinsTanTheta;
    cfg.nBinsY = m_nBinsIntercept;
    ActsPeakFinderForMuonCfg peakFinderCfg;
    peakFinderCfg.fractionCutoff = m_peakFractionCutOff;
    peakFinderCfg.threshold = m_peakThreshold;
    peakFinderCfg.minSpacingBetweenPeaks = {m_minMaxDistTheta, m_minMaxDistIntercept};
    data.houghPlane = std::make_unique<HoughPlane>(cfg);
    data.peakFinder = std::make_unique<ActsPeakFinderForMuon>(peakFinderCfg);

    return StatusCode::SUCCESS;
}

StatusCode EtaHoughTransformAlg::processBucket(HoughEventData& data, 
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
    double searchStartTanTheta =
        tanThetaMean - 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    double searchEndTanTheta =
        tanThetaMean + 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    searchStartTanTheta =
        std::min(searchStartTanTheta,
                 bucket.searchWindowTanAngle.first - m_minSigmasSearchTanTheta * m_targetResoTanTheta);
    searchEndTanTheta =
        std::max(searchEndTanTheta, bucket.searchWindowTanAngle.second +
                                        m_minSigmasSearchTanTheta * m_targetResoTanTheta);

    data.currAxisRanges = Acts::HoughTransformUtils::HoughAxisRanges{
        searchStartTanTheta, searchEndTanTheta, searchStart, searchEnd};

    data.houghPlane->reset();
    for (const SpacePointBucket::value_type& hit : *(bucket.bucket)) {
        fillFromSpacePoint(data, hit.get());
    }
    auto maxima =
        data.peakFinder->findPeaks(*(data.houghPlane), data.currAxisRanges);
    if (maxima.empty()) {
        ATH_MSG_DEBUG("Station "<<bucket.bucket->chamber()->stationName() 
            <<" eta "<<bucket.bucket->chamber()->stationEta()
            <<" "<<bucket.bucket->chamber()->stationPhi()
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
        return StatusCode::SUCCESS;
    }
    for (const auto& max : maxima) {
        /// TODO: Proper weighted hit counting...
        std::vector<HoughHitType> hitList;
        hitList.reserve(max.hitIdentifiers.size());
        unsigned int nPrec{0};
        for (const HoughHitType& hit : max.hitIdentifiers) {
            nPrec += isPrecisionHit(hit);
            hitList.push_back(hit);
        }
        if (nPrec < m_nPrecHitCut) {
            ATH_MSG_VERBOSE("The maximum did not pass the precision hit cut");
            continue;
        }
        size_t nHits = hitList.size();
        extendWithPhiHits(hitList, bucket);
        sortHits(hitList);
        data.maxima.emplace_back(max.x, max.y, nHits, std::move(hitList), bucket.bucket);
    }

    return StatusCode::SUCCESS;
}
void EtaHoughTransformAlg::fillFromSpacePoint(HoughEventData& data, const HoughHitType& SP) const {

    using namespace std::placeholders; 
    double w = 1.0; 
    // downweight RPC measurements in the barrel relative to MDT  
    if (SP->primaryMeasurement()->type() == xAOD::UncalibMeasType::RpcStripType){
        w = 0.5; 
    }
    if (SP->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
        data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtLeft,
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2, m_targetResoIntercept), SP, 0, w);
        data.houghPlane->fill<HoughHitType>(SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtRight,
                                            std::bind(HoughHelpers::Eta::houghWidthMdt, _1, _2, m_targetResoIntercept), SP, 0, w);
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
