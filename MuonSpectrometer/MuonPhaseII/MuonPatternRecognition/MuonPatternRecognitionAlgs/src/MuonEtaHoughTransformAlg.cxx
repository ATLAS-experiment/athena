/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonEtaHoughTransformAlg.h"

#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <StoreGate/ReadCondHandle.h>

#include "MuonHoughHelpers.h"

using namespace MuonR4;

MuonEtaHoughTransformAlg::MuonEtaHoughTransformAlg(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonEtaHoughTransformAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_maxima.initialize());

    return StatusCode::SUCCESS;
}

template <class ContainerType>
StatusCode MuonEtaHoughTransformAlg::retrieveContainer(
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

StatusCode MuonEtaHoughTransformAlg::execute(const EventContext& ctx) const {

    /// read the PRDs
    const MuonSpacePointContainer* spacePoints{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_spacePointKey, spacePoints));

    // book the output container
    SG::WriteHandle<StationHoughMaxContainer> writeMaxima(m_maxima, ctx);
    ATH_CHECK(writeMaxima.record(std::make_unique<StationHoughMaxContainer>()));

    SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
    ATH_CHECK(gctxHandle.isValid());

    MuonHoughEventData data{*gctxHandle};

    /// pre-populate the event data - sort PRDs by station
    ATH_CHECK(preProcess(data, *spacePoints));

    /// book the hough plane
    ATH_CHECK(prepareHoughPlane(data));
    /// now perform the actual HT for each station
    for (auto& [station, stationHoughBuckets] : data.houghSetups) {
        // reset the list of maxima
        data.maxima.clear();
        for (auto& bucket : stationHoughBuckets) {
            ATH_CHECK(processBucket(data, bucket));
        }
        /// write the maxima we found
        writeMaxima->emplace(StationHoughMaxima(station, data.maxima));
    }
    return StatusCode::SUCCESS;
}
StatusCode MuonEtaHoughTransformAlg::preProcess(
    MuonHoughEventData& data,
    const MuonR4::MuonSpacePointContainer& spacePoints) const {

    ATH_MSG_DEBUG("Load " << spacePoints.size() << " space point buckets");
    for (const MuonR4::MuonSpacePointBucket* sp : spacePoints) {
        std::vector<HoughSetupForBucket>& buckets =
            data.houghSetups[sp->front()->muonChamber()];
        buckets.push_back(HoughSetupForBucket{sp});
        HoughSetupForBucket& hs = buckets.back();
        Amg::Vector3D leftSide =
            hs.bucket->muonChamber()
                ->globalToLocalTrans(data.gctx)
                .translation() -
            (hs.bucket->coveredMin() * Amg::Vector3D::UnitY());
        Amg::Vector3D rightSide =
            hs.bucket->muonChamber()
                ->globalToLocalTrans(data.gctx)
                .translation() -
            (hs.bucket->coveredMax() * Amg::Vector3D::UnitY());
        const double tanThetaLeft = leftSide.y() / leftSide.z();
        const double tanThetaRight = rightSide.y() / rightSide.z();
        hs.searchWindowTanAngle = {tanThetaLeft, tanThetaRight};
        hs.searchWindowIntercept = {hs.bucket->coveredMin(),
                                    hs.bucket->coveredMax()};
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonEtaHoughTransformAlg::prepareHoughPlane(
    MuonHoughEventData& data) const {
    HoughPlaneConfig cfg;
    cfg.nBinsX = m_nBinsTanTheta;
    cfg.nBinsY = m_nBinsIntercept;
    ActsPeakFinderForMuonCfg peakFinderCfg;
    peakFinderCfg.fractionCutoff = 0.7;
    peakFinderCfg.threshold = 3;
    peakFinderCfg.minSpacingBetweenPeaks = {0., 30.};
    data.houghPlane = std::make_unique<HoughPlane>(cfg);
    data.peakFinder = std::make_unique<ActsPeakFinderForMuon>(peakFinderCfg);

    return StatusCode::SUCCESS;
}

StatusCode MuonEtaHoughTransformAlg::processBucket(
    MuonHoughEventData& data, HoughSetupForBucket& bucket) const {
    /// tune the search space

    double chamberCenter = 0.5 * (bucket.searchWindowIntercept.first +
                                  bucket.searchWindowIntercept.second);
    // build a symmetric window around the (geometric) chamber center so that
    // the bin width is equivalent to our target resolution
    double searchStart =
        chamberCenter - 0.5 * data.houghPlane->nBinsY() * m_targetResoIntercept;
    double searchEnd =
        chamberCenter + 0.5 * data.houghPlane->nBinsY() * m_targetResoIntercept;
    // Protection for very wide buckets - if the search space does not cover all
    // of the bucket, widen the bin size so that we cover everything
    searchStart = std::min(searchStart, bucket.searchWindowIntercept.first -
                                            2. * m_targetResoIntercept);
    searchEnd = std::max(searchEnd, bucket.searchWindowIntercept.second +
                                        2. * m_targetResoIntercept);
    // also treat tan(theta)
    double tanThetaMean = 0.5 * (bucket.searchWindowTanAngle.first +
                                 bucket.searchWindowTanAngle.second);
    double searchStartTanTheta =
        tanThetaMean - 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    double searchEndTanTheta =
        tanThetaMean + 0.5 * data.houghPlane->nBinsX() * m_targetResoTanTheta;
    searchStartTanTheta =
        std::min(searchStartTanTheta,
                 bucket.searchWindowTanAngle.first - 2. * m_targetResoTanTheta);
    searchEndTanTheta =
        std::max(searchEndTanTheta, bucket.searchWindowTanAngle.second +
                                        2. * m_targetResoTanTheta);

    data.currAxisRanges = Acts::HoughTransformUtils::HoughAxisRanges{
        searchStartTanTheta, searchEndTanTheta, searchStart, searchEnd};
    data.houghPlane->reset();
    for (const HoughHitType& hit : *(bucket.bucket)) {
        fillFromSpacePoint(data, hit);
    }
    auto maxima =
        data.peakFinder->findPeaks(*(data.houghPlane), data.currAxisRanges);
    if (maxima.empty()) {
        return StatusCode::SUCCESS;
    }
    for (const auto& max : maxima) {
        /// TODO: Proper weighted hit counting...
        std::vector<HoughHitType> hitList;
        hitList.insert(hitList.end(), max.hitIdentifiers.begin(),
                       max.hitIdentifiers.end());
        size_t nHits = hitList.size();
        extendWithPhiHits(hitList, bucket);
        data.maxima.emplace_back(max.x, max.y, nHits, std::move(hitList));
    }

    return StatusCode::SUCCESS;
}
void MuonEtaHoughTransformAlg::fillFromSpacePoint(
    MuonHoughEventData& data, const MuonR4::HoughHitType& SP) const {
    if (SP->primaryMeasurement()->type() ==
        xAOD::UncalibMeasType::MdtDriftCircleType) {
        data.houghPlane->fill<HoughHitType>(
            SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtLeft,
            HoughHelpers::Eta::houghWidthMdt, SP, 0, 1.0);
        data.houghPlane->fill<HoughHitType>(
            SP, data.currAxisRanges, HoughHelpers::Eta::houghParamMdtRight,
            HoughHelpers::Eta::houghWidthMdt, SP, 0, 1.0);
    } else {
        if (SP->measuresEta()) {
            data.houghPlane->fill<HoughHitType>(
                SP, data.currAxisRanges, HoughHelpers::Eta::houghParamStrip,
                HoughHelpers::Eta::houghWidthStrip, SP, 0, 1.0);
        }
    }
}
void MuonEtaHoughTransformAlg::extendWithPhiHits(
    std::vector<HoughHitType>& hitList, HoughSetupForBucket& bucket) const {
    std::copy_if(bucket.bucket->begin(), bucket.bucket->end(),
                 std::back_inserter(hitList),
                 [](MuonR4::HoughHitType hit) { return !hit->measuresPhi(); });
}
