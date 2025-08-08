/*  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "src/GridTripletSeedingTool.h"

#include <cmath>

namespace ActsTrk {

namespace {

static constexpr float expCutRMin = 45.;

static inline bool itkFastTrackingSpCut(
    const Acts::Experimental::ConstSpacePointProxy2& sp) {
  float r = sp.r();
  float zabs = std::abs(sp.z());

  // We perform a triangular cut and remove the space points
  // that have |z| < 200 and radius < expCutRMin
  // But we do that only if the eta of the space point wrt origin is < 3.6
  // eta 3.6 corresponds to 18.2855
  if (zabs > 200. and zabs < 18.2855 * r and r < expCutRMin) {
    return false;
  }

  // Remove space points beyond eta=4 if their z is larger than the max seed z0
  // (150.)
  float cotTheta = 27.2899;  // corresponds to eta=4
  if ((zabs - 150.) > cotTheta * r) {
    return false;
  }

  return true;
}

static inline bool itkFastDoubletCut(
    const Acts::Experimental::ConstSpacePointProxy2& /*middle*/,
    const Acts::Experimental::ConstSpacePointProxy2& other, float cotTheta,
    bool isBottomCandidate) {
  // We remove here some seeds, in case the bottom space point radius is
  // too small (i.e. < fastTrackingRMin)

  // This operation is done only within a specific eta window
  // Instead of eta we use the doublet cottheta
  // We require:
  //     fastTrackingCotThetaWindowMin < cottheta doublet <
  //     fastTrackingCotThetaWindowMax
  // with stranslates to an eta window.
  // cottheta of 1.5 is about eta 1.2
  // cottheta of 18.2855 is about eta of 3.6
  constexpr float fastTrackingCotThetaWindowMin = 1.5;
  constexpr float fastTrackingCotThetaWindowMax = 18.2855;

  float absCotTheta = std::abs(cotTheta);
  if (isBottomCandidate && other.r() < expCutRMin &&
      absCotTheta > fastTrackingCotThetaWindowMin &&
      absCotTheta < fastTrackingCotThetaWindowMax) {
    return false;
  }

  return true;
}

}  // namespace

GridTripletSeedingTool::GridTripletSeedingTool(const std::string& type,
                                               const std::string& name,
                                               const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode GridTripletSeedingTool::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_MSG_DEBUG("Properties Summary:");
  ATH_MSG_DEBUG("   " << m_zBinNeighborsTop);
  ATH_MSG_DEBUG("   " << m_zBinNeighborsBottom);
  ATH_MSG_DEBUG("   " << m_rBinNeighborsTop);
  ATH_MSG_DEBUG("   " << m_rBinNeighborsBottom);
  ATH_MSG_DEBUG("   " << m_numPhiNeighbors);

  ATH_MSG_DEBUG(" *  Used by space point grid config:");
  ATH_MSG_DEBUG("   " << m_minPt);
  ATH_MSG_DEBUG("   " << m_cotThetaMax);
  ATH_MSG_DEBUG("   " << m_impactMax);
  ATH_MSG_DEBUG("   " << m_zMin);
  ATH_MSG_DEBUG("   " << m_zMax);
  ATH_MSG_DEBUG("   " << m_gridPhiMin);
  ATH_MSG_DEBUG("   " << m_gridPhiMax);
  ATH_MSG_DEBUG("   " << m_zBinEdges);
  ATH_MSG_DEBUG("   " << m_rBinEdges);
  ATH_MSG_DEBUG("   " << m_deltaRMax);
  ATH_MSG_DEBUG("   " << m_gridRMax);
  ATH_MSG_DEBUG("   " << m_phiBinDeflectionCoverage);

  ATH_MSG_DEBUG(" * Used by seed finder config:");
  ATH_MSG_DEBUG("   " << m_minPt);
  ATH_MSG_DEBUG("   " << m_cotThetaMax);
  ATH_MSG_DEBUG("   " << m_impactMax);
  ATH_MSG_DEBUG("   " << m_zMin);
  ATH_MSG_DEBUG("   " << m_zMax);
  ATH_MSG_DEBUG("   " << m_zBinEdges);
  ATH_MSG_DEBUG("   " << m_rMax);
  ATH_MSG_DEBUG("   " << m_deltaRMin);
  ATH_MSG_DEBUG("   " << m_deltaRMax);
  ATH_MSG_DEBUG("   " << m_deltaRMinTopSP);
  ATH_MSG_DEBUG("   " << m_deltaRMaxTopSP);
  ATH_MSG_DEBUG("   " << m_deltaRMinBottomSP);
  ATH_MSG_DEBUG("   " << m_deltaRMaxBottomSP);
  ATH_MSG_DEBUG("   " << m_deltaZMax);
  ATH_MSG_DEBUG("   " << m_collisionRegionMin);
  ATH_MSG_DEBUG("   " << m_collisionRegionMax);
  ATH_MSG_DEBUG("   " << m_sigmaScattering);
  ATH_MSG_DEBUG("   " << m_maxPtScattering);
  ATH_MSG_DEBUG("   " << m_radLengthPerSeed);
  ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
  ATH_MSG_DEBUG("   " << m_interactionPointCut);
  ATH_MSG_DEBUG("   " << m_zBinsCustomLooping);
  ATH_MSG_DEBUG("   " << m_rBinsCustomLooping);
  ATH_MSG_DEBUG("   " << m_useVariableMiddleSPRange);
  if (m_useVariableMiddleSPRange) {
    ATH_MSG_DEBUG("   " << m_deltaRMiddleMinSPRange);
    ATH_MSG_DEBUG("   " << m_deltaRMiddleMaxSPRange);
  } else if (not m_rRangeMiddleSP.empty())
    ATH_MSG_DEBUG("   " << m_rRangeMiddleSP);
  ATH_MSG_DEBUG("   " << m_seedConfirmation);
  if (m_seedConfirmation) {
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralRMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinImpact);
    ATH_MSG_DEBUG("   " << m_seedConfForwardZMin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardZMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardRMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinImpact);
  }
  ATH_MSG_DEBUG("   " << m_useDetailedDoubleMeasurementInfo);
  ATH_MSG_DEBUG("   " << m_toleranceParam);
  ATH_MSG_DEBUG("   " << m_phiMin);
  ATH_MSG_DEBUG("   " << m_phiMax);
  ATH_MSG_DEBUG("   " << m_rMin);
  ATH_MSG_DEBUG("   " << m_zAlign);
  ATH_MSG_DEBUG("   " << m_rAlign);
  ATH_MSG_DEBUG("   " << m_sigmaError);

  ATH_MSG_DEBUG(" * Used by seed filter config:");
  ATH_MSG_DEBUG("   " << m_deltaRMin);
  ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
  ATH_MSG_DEBUG("   " << m_useDeltaRorTopRadius);
  ATH_MSG_DEBUG("   " << m_seedConfirmationInFilter);
  if (m_seedConfirmationInFilter) {
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_maxQualitySeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralRMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinImpact);
    ATH_MSG_DEBUG("   " << m_seedConfForwardZMin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardZMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardRMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinImpact);
  }
  ATH_MSG_DEBUG("   " << m_impactWeightFactor);
  ATH_MSG_DEBUG("   " << m_compatSeedWeight);
  ATH_MSG_DEBUG("   " << m_compatSeedLimit);
  ATH_MSG_DEBUG("   " << m_seedWeightIncrement);
  ATH_MSG_DEBUG("   " << m_numSeedIncrement);
  ATH_MSG_DEBUG("   " << m_deltaInvHelixDiameter);

  // Make the logger And Propagate to ACTS routines
  m_logger = makeActsAthenaLogger(this, "Acts");

  if (m_zBinEdges.size() - 1 != m_zBinNeighborsTop.size() &&
      not m_zBinNeighborsTop.empty()) {
    ATH_MSG_ERROR("Inconsistent config zBinNeighborsTop");
    return StatusCode::FAILURE;
  }

  if (m_zBinEdges.size() - 1 != m_zBinNeighborsBottom.size() &&
      not m_zBinNeighborsBottom.empty()) {
    ATH_MSG_ERROR("Inconsistent config zBinNeighborsBottom");
    return StatusCode::FAILURE;
  }

  if (m_rBinEdges.size() - 1 != m_rBinNeighborsTop.size() &&
      not m_rBinNeighborsTop.empty()) {
    ATH_MSG_ERROR("Inconsistent config rBinNeighborsTop");
    return StatusCode::FAILURE;
  }

  if (m_rBinEdges.size() - 1 != m_rBinNeighborsBottom.size() &&
      not m_rBinNeighborsBottom.empty()) {
    ATH_MSG_ERROR("Inconsistent config rBinNeighborsBottom");
    return StatusCode::FAILURE;
  }

  if (m_zBinsCustomLooping.size() != 0) {
    // zBinsCustomLooping can contain a number of elements <= to the total
    // number of bin in zBinEdges
    for (std::size_t i : m_zBinsCustomLooping) {
      if (i >= m_zBinEdges.size()) {
        ATH_MSG_ERROR(
            "Inconsistent config zBinsCustomLooping contains bins that are not "
            "in zBinEdges");
        return StatusCode::FAILURE;
      }
    }
  }

  if (m_rBinsCustomLooping.size() != 0) {
    for (std::size_t i : m_rBinsCustomLooping) {
      if (i >= m_rBinEdges.size()) {
        ATH_MSG_ERROR(
            "Inconsistent config rBinsCustomLooping contains bins that are not "
            "in rBinEdges");
        return StatusCode::FAILURE;
      }
    }
  }

  m_gridCfg.minPt = m_minPt.value();
  m_gridCfg.rMin = 0;
  m_gridCfg.rMax = m_gridRMax.value();
  m_gridCfg.zMin = m_zMin.value();
  m_gridCfg.zMax = m_zMax.value();
  m_gridCfg.deltaRMax = m_deltaRMax.value();
  m_gridCfg.cotThetaMax = m_cotThetaMax.value();
  m_gridCfg.impactMax = m_impactMax.value();
  m_gridCfg.phiMin = m_gridPhiMin.value();
  m_gridCfg.phiMax = m_gridPhiMax.value();
  m_gridCfg.phiBinDeflectionCoverage = m_phiBinDeflectionCoverage.value();
  m_gridCfg.maxPhiBins = m_maxPhiBins.value();
  m_gridCfg.zBinEdges = m_zBinEdges.value();
  m_gridCfg.rBinEdges = m_rBinEdges.value();
  m_gridCfg.bFieldInZ = 0;  // will result in max phi bins
  m_gridCfg.bottomBinFinder = Acts::GridBinFinder<3ul>(
      m_numPhiNeighbors.value(), m_zBinNeighborsBottom.value(),
      m_rBinNeighborsBottom.value());
  m_gridCfg.topBinFinder = Acts::GridBinFinder<3ul>(m_numPhiNeighbors.value(),
                                                    m_zBinNeighborsTop.value(),
                                                    m_rBinNeighborsTop.value());
  m_gridCfg.navigation[0ul] = {};
  m_gridCfg.navigation[1ul] = m_zBinsCustomLooping.value();
  m_gridCfg.navigation[2ul] = m_rBinsCustomLooping.value();

  m_bottomDoubletFinderCfg.candidateDirection = Acts::Direction::Backward();
  m_bottomDoubletFinderCfg.deltaRMin = m_deltaRMinBottomSP.value();
  m_bottomDoubletFinderCfg.deltaRMax = m_deltaRMaxBottomSP.value();
  m_bottomDoubletFinderCfg.deltaZMin = -std::numeric_limits<float>::infinity();
  m_bottomDoubletFinderCfg.deltaZMax = m_deltaZMax.value();
  m_bottomDoubletFinderCfg.impactMax = m_impactMax.value();
  m_bottomDoubletFinderCfg.interactionPointCut = m_interactionPointCut.value();
  m_bottomDoubletFinderCfg.collisionRegionMin = m_collisionRegionMin.value();
  m_bottomDoubletFinderCfg.collisionRegionMax = m_collisionRegionMax.value();
  m_bottomDoubletFinderCfg.cotThetaMax = m_cotThetaMax.value();
  m_bottomDoubletFinderCfg.minPt = m_minPt.value();
  m_bottomDoubletFinderCfg.helixCutTolerance = 1.;
  if (m_useExperimentCuts) {
    m_bottomDoubletFinderCfg.experimentCuts.connect<itkFastDoubletCut>();
  }

  m_topDoubletFinderCfg = m_bottomDoubletFinderCfg;  // copy the bottom cuts
  m_topDoubletFinderCfg.candidateDirection = Acts::Direction::Forward();
  m_topDoubletFinderCfg.deltaRMin = m_deltaRMinTopSP.value();
  m_topDoubletFinderCfg.deltaRMax = m_deltaRMaxTopSP.value();

  m_tripletCuts.minPt = m_minPt.value();
  m_tripletCuts.sigmaScattering = m_sigmaScattering.value();
  m_tripletCuts.radLengthPerSeed = m_radLengthPerSeed.value();
  m_tripletCuts.maxPtScattering = m_maxPtScattering.value();
  m_tripletCuts.impactMax = m_impactMax.value();
  m_tripletCuts.helixCutTolerance = 1.;
  m_tripletCuts.toleranceParam = m_toleranceParam.value();

  m_filterCfg.deltaInvHelixDiameter = m_deltaInvHelixDiameter.value();
  m_filterCfg.deltaRMin = m_deltaRMin.value();
  m_filterCfg.compatSeedWeight = m_compatSeedWeight.value();
  m_filterCfg.impactWeightFactor = m_impactWeightFactor.value();
  m_filterCfg.zOriginWeightFactor = m_zOriginWeightFactor.value();
  m_filterCfg.maxSeedsPerSpM = m_maxSeedsPerSpM.value();
  m_filterCfg.compatSeedLimit = m_compatSeedLimit.value();
  m_filterCfg.seedWeightIncrement = m_seedWeightIncrement.value();
  m_filterCfg.numSeedIncrement = m_numSeedIncrement.value();
  m_filterCfg.seedConfirmation = m_seedConfirmationInFilter.value();
  m_filterCfg.centralSeedConfirmationRange.zMinSeedConf = m_seedConfCentralZMin;
  m_filterCfg.centralSeedConfirmationRange.zMaxSeedConf = m_seedConfCentralZMax;
  m_filterCfg.centralSeedConfirmationRange.rMaxSeedConf = m_seedConfCentralRMax;
  m_filterCfg.centralSeedConfirmationRange.nTopForLargeR =
      m_seedConfCentralNTopLargeR;
  m_filterCfg.centralSeedConfirmationRange.nTopForSmallR =
      m_seedConfCentralNTopSmallR;
  m_filterCfg.centralSeedConfirmationRange.seedConfMinBottomRadius =
      m_seedConfCentralMinBottomRadius;
  m_filterCfg.centralSeedConfirmationRange.seedConfMaxZOrigin =
      m_seedConfCentralMaxZOrigin;
  m_filterCfg.centralSeedConfirmationRange.minImpactSeedConf =
      m_seedConfCentralMinImpact;
  m_filterCfg.forwardSeedConfirmationRange.zMinSeedConf = m_seedConfForwardZMin;
  m_filterCfg.forwardSeedConfirmationRange.zMaxSeedConf = m_seedConfForwardZMax;
  m_filterCfg.forwardSeedConfirmationRange.rMaxSeedConf = m_seedConfForwardRMax;
  m_filterCfg.forwardSeedConfirmationRange.nTopForLargeR =
      m_seedConfForwardNTopLargeR;
  m_filterCfg.forwardSeedConfirmationRange.nTopForSmallR =
      m_seedConfForwardNTopSmallR;
  m_filterCfg.forwardSeedConfirmationRange.seedConfMinBottomRadius =
      m_seedConfForwardMinBottomRadius;
  m_filterCfg.forwardSeedConfirmationRange.seedConfMaxZOrigin =
      m_seedConfForwardMaxZOrigin;
  m_filterCfg.forwardSeedConfirmationRange.minImpactSeedConf =
      m_seedConfForwardMinImpact;
  m_filterCfg.maxSeedsPerSpMConf = m_maxSeedsPerSpMConf.value();
  m_filterCfg.maxQualitySeedsPerSpMConf = m_maxQualitySeedsPerSpMConf.value();
  m_filterCfg.useDeltaRinsteadOfTopRadius = m_useDeltaRorTopRadius.value();

  m_finderOpts.useStripMeasurementInfo =
      m_useDetailedDoubleMeasurementInfo.value();

  m_finder = Acts::Experimental::BroadTripletSeedFinder(
      logger().cloneWithSuffix("Finder"));
  m_filter = Acts::Experimental::BroadTripletSeedFilter(
      m_filterCfg, logger().cloneWithSuffix("Filter"));

  return StatusCode::SUCCESS;
}

std::pair<float, float> GridTripletSeedingTool::retrieveRadiusRangeForMiddle(
    const Acts::Experimental::ConstSpacePointProxy2& spM,
    const Acts::Range1D<float>& rMiddleSpRange) const {
  if (m_useVariableMiddleSPRange.value()) {
    return {rMiddleSpRange.min(), rMiddleSpRange.max()};
  }
  if (m_rRangeMiddleSP.empty()) {
    throw std::runtime_error(
        "m_rRangeMiddleSP is empty, please check the configuration.");
  }

  // get zBin position of the middle SP
  auto pVal = std::lower_bound(m_zBinEdges.value().begin(),
                               m_zBinEdges.value().end(), spM.z());
  int zBin = std::distance(m_zBinEdges.value().begin(), pVal);
  // protects against zM at the limit of zBinEdges
  zBin == 0 ? zBin : --zBin;
  return {m_rRangeMiddleSP.value()[zBin][0], m_rRangeMiddleSP.value()[zBin][1]};
}

StatusCode GridTripletSeedingTool::createSeeds2(
    const EventContext& ctx,
    const Acts::Experimental::SpacePointContainer2& spacePoints,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    Acts::Experimental::SeedContainer2& seedContainer) const {
  (void)ctx;
  (void)beamSpotPos;

  auto gridCfg = m_gridCfg;
  gridCfg.bFieldInZ = bFieldInZ;

  Acts::Experimental::CylindricalSpacePointGrid2 grid(
      gridCfg, logger().cloneWithSuffix("Grid"));

  for (auto sp : spacePoints) {
    if (m_useExperimentCuts && !itkFastTrackingSpCut(sp)) {
      continue;
    }

    grid.insert(sp.index(), sp.phi(), sp.z(), sp.r());
  }

  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::ranges::sort(grid.at(i),
                      [&](const Acts::Experimental::SpacePointIndex2& a,
                          const Acts::Experimental::SpacePointIndex2& b) {
                        return spacePoints[a].r() < spacePoints[b].r();
                      });
  }

  // TODO a second copy of the container should not be necessary, but
  //      the current `createSeeds2` interface requires it
  Acts::Experimental::SpacePointContainer2 selectedSpacePoints;
  selectedSpacePoints.createColumns(
      Acts::Experimental::SpacePointColumns::SourceLinks |
      Acts::Experimental::SpacePointColumns::X |
      Acts::Experimental::SpacePointColumns::Y |
      Acts::Experimental::SpacePointColumns::Z |
      Acts::Experimental::SpacePointColumns::R |
      Acts::Experimental::SpacePointColumns::VarianceR |
      Acts::Experimental::SpacePointColumns::VarianceZ |
      Acts::Experimental::SpacePointColumns::Strip);
  selectedSpacePoints.reserve(grid.numberOfSpacePoints());
  std::vector<Acts::Experimental::SpacePointIndexRange2> gridSpacePointRanges;
  gridSpacePointRanges.reserve(grid.numberOfBins());
  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::uint32_t begin = selectedSpacePoints.size();
    for (const Acts::Experimental::SpacePointIndex2 spIndex : grid.at(i)) {
      const auto sp = spacePoints[spIndex];

      auto newSp = selectedSpacePoints.createSpacePoint();
      newSp.assignSourceLinks(sp.sourceLinks());
      newSp.x() = sp.x();
      newSp.y() = sp.y();
      newSp.z() = sp.z();
      newSp.r() = sp.r();
      newSp.varianceR() = sp.varianceR();
      newSp.varianceZ() = sp.varianceZ();
    }
    std::uint32_t end = selectedSpacePoints.size();
    gridSpacePointRanges.emplace_back(begin, end);
  }

  // Compute radius range. We rely on the fact the grid is storing the proxies
  // with a sorting in the radius
  const Acts::Range1D<float> rRange = [&]() -> Acts::Range1D<float> {
    float minRange = std::numeric_limits<float>::max();
    float maxRange = std::numeric_limits<float>::lowest();
    for (const Acts::Experimental::SpacePointIndexRange2& range :
         gridSpacePointRanges) {
      if (range.first == range.second) {
        continue;
      }
      auto first = selectedSpacePoints[range.first];
      auto last = selectedSpacePoints[range.second - 1];
      minRange = std::min(first.r(), minRange);
      maxRange = std::max(last.r(), maxRange);
    }
    return {minRange, maxRange};
  }();

  Acts::Experimental::BroadTripletSeedFinder::DerivedTripletCuts tripletCuts(
      m_tripletCuts, bFieldInZ);

  Acts::Experimental::DoubletSeedFinder bottomDoubletFinder(
      Acts::Experimental::DoubletSeedFinder::DerivedConfig(
          m_bottomDoubletFinderCfg, bFieldInZ));
  Acts::Experimental::DoubletSeedFinder topDoubletFinder(
      Acts::Experimental::DoubletSeedFinder::DerivedConfig(
          m_topDoubletFinderCfg, bFieldInZ));

  // variable middle SP radial region of interest
  const Acts::Range1D<float> rMiddleSpRange(
      std::floor(rRange.min() / 2) * 2 + m_deltaRMiddleMinSPRange.value(),
      std::floor(rRange.max() / 2) * 2 - m_deltaRMiddleMaxSPRange.value());

  auto finderOpts = m_finderOpts;

  Acts::Experimental::BroadTripletSeedFinder::State state;
  Acts::Experimental::BroadTripletSeedFinder::Cache cache;

  std::vector<Acts::Experimental::SpacePointContainer2::ConstRange>
      bottomSpRanges;
  std::optional<Acts::Experimental::SpacePointContainer2::ConstRange>
      middleSpRange;
  std::vector<Acts::Experimental::SpacePointContainer2::ConstRange> topSpRanges;

  for (const auto [bottom, middle, top] : grid.binnedGroup()) {
    ACTS_VERBOSE("Process middle bin " << middle);
    if (middle >= gridSpacePointRanges.size()) {
      ATH_MSG_ERROR("Grid Binned Group returned an unreasonable middle bin");
      return StatusCode::FAILURE;
    }

    bottomSpRanges.clear();
    topSpRanges.clear();

    std::ranges::transform(
        bottom, std::back_inserter(bottomSpRanges),
        [&](std::size_t b)
            -> Acts::Experimental::SpacePointContainer2::ConstRange {
          return selectedSpacePoints.range(gridSpacePointRanges[b]).asConst();
        });
    middleSpRange =
        selectedSpacePoints.range(gridSpacePointRanges[middle]).asConst();
    std::ranges::transform(
        top, std::back_inserter(topSpRanges),
        [&](std::size_t t)
            -> Acts::Experimental::SpacePointContainer2::ConstRange {
          return selectedSpacePoints.range(gridSpacePointRanges[t]).asConst();
        });

    // we compute this here since all middle space point candidates belong to
    // the same z-bin
    auto firstMiddleSp = middleSpRange->front();
    auto radiusRangeForMiddle =
        retrieveRadiusRangeForMiddle(firstMiddleSp, rMiddleSpRange);

    ACTS_VERBOSE("Validity range (radius) for the middle space point is ["
                 << radiusRangeForMiddle.first << ", "
                 << radiusRangeForMiddle.second << "]");

    m_finder->createSeedsFromGroups(
        finderOpts, state, cache, bottomDoubletFinder, topDoubletFinder,
        tripletCuts, *m_filter, spacePoints, bottomSpRanges, *middleSpRange,
        topSpRanges, radiusRangeForMiddle, seedContainer);
  }

  if (m_seedQualitySelection) {
    // Selection function - temporary implementation
    // need change from ACTS for final implementation
    // To be used only on PPP
    auto selectionFunction =
        [&state](const Acts::Experimental::MutableSeedProxy2& seed) -> bool {
      float seedQuality = seed.quality();
      float bottomQuality =
          state.filter.bestSeedQualityMap.at(seed.spacePointIndices()[0]);
      float middleQuality =
          state.filter.bestSeedQualityMap.at(seed.spacePointIndices()[1]);
      float topQuality =
          state.filter.bestSeedQualityMap.at(seed.spacePointIndices()[2]);

      return bottomQuality <= seedQuality || middleQuality <= seedQuality ||
             topQuality <= seedQuality;
    };

    Acts::Experimental::SeedContainer2 newSeedContainer;
    newSeedContainer.reserve(seedContainer.size());

    // Select the seeds
    for (Acts::Experimental::MutableSeedProxy2 seed : seedContainer) {
      if (!selectionFunction(seed)) {
        continue;
      }
      auto newSeed = newSeedContainer.createSeed();
      newSeed.assignSpacePointIndices(seed.spacePointIndices());
      newSeed.vertexZ() = seed.vertexZ();
      newSeed.quality() = seed.quality();
    }

    // Replace the old seed container with the new one
    seedContainer = std::move(newSeedContainer);
  }

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
