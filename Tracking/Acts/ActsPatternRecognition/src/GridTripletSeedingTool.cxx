/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GridTripletSeedingTool.h"

namespace ActsTrk {

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

  // Make the logger && Propagate to ACTS routines
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

  m_gridCfg.minPt = m_minPt;
  m_gridCfg.rMin = 0;
  m_gridCfg.rMax = m_gridRMax;
  m_gridCfg.zMin = m_zMin;
  m_gridCfg.zMax = m_zMax;
  m_gridCfg.deltaRMax = m_deltaRMax;
  m_gridCfg.cotThetaMax = m_cotThetaMax;
  m_gridCfg.impactMax = m_impactMax;
  m_gridCfg.phiMin = m_gridPhiMin;
  m_gridCfg.phiMax = m_gridPhiMax;
  m_gridCfg.phiBinDeflectionCoverage = m_phiBinDeflectionCoverage;
  m_gridCfg.maxPhiBins = m_maxPhiBins;
  m_gridCfg.zBinEdges = m_zBinEdges;
  m_gridCfg.rBinEdges = m_rBinEdges;
  m_gridCfg.bFieldInZ = 0;  // will be set later
  m_gridCfg.bottomBinFinder = Acts::GridBinFinder<3ul>(
      m_numPhiNeighbors.value(), m_zBinNeighborsBottom.value(),
      m_rBinNeighborsBottom.value());
  m_gridCfg.topBinFinder = Acts::GridBinFinder<3ul>(m_numPhiNeighbors.value(),
                                                    m_zBinNeighborsTop.value(),
                                                    m_rBinNeighborsTop.value());
  m_gridCfg.navigation[0ul] = {};
  m_gridCfg.navigation[1ul] = m_zBinsCustomLooping;
  m_gridCfg.navigation[2ul] = m_rBinsCustomLooping;

  m_bottomDoubletFinderCfg.spacePointsSortedByRadius = true;
  m_bottomDoubletFinderCfg.candidateDirection = Acts::Direction::Backward();
  m_bottomDoubletFinderCfg.deltaRMin = m_deltaRMinBottomSP;
  m_bottomDoubletFinderCfg.deltaRMax = m_deltaRMaxBottomSP;
  m_bottomDoubletFinderCfg.deltaZMin = -m_deltaZMax;
  m_bottomDoubletFinderCfg.deltaZMax = m_deltaZMax;
  m_bottomDoubletFinderCfg.impactMax = m_impactMax;
  m_bottomDoubletFinderCfg.interactionPointCut = m_interactionPointCut;
  m_bottomDoubletFinderCfg.collisionRegionMin = m_collisionRegionMin;
  m_bottomDoubletFinderCfg.collisionRegionMax = m_collisionRegionMax;
  m_bottomDoubletFinderCfg.cotThetaMax = m_cotThetaMax;
  m_bottomDoubletFinderCfg.minPt = m_minPt;
  m_bottomDoubletFinderCfg.helixCutTolerance = 1.;
  if (m_useExperimentCuts) {
    m_bottomDoubletFinderCfg.experimentCuts
        .connect<&ActsTrk::GridTripletSeedingTool::doubletSelectionFunction>(
            this);
  }

  m_topDoubletFinderCfg = m_bottomDoubletFinderCfg;  // copy the bottom cuts
  m_topDoubletFinderCfg.candidateDirection = Acts::Direction::Forward();
  m_topDoubletFinderCfg.deltaRMin = m_deltaRMinTopSP;
  m_topDoubletFinderCfg.deltaRMax = m_deltaRMaxTopSP;

  m_tripletFinderCfg.useStripInfo = m_useDetailedDoubleMeasurementInfo;
  m_tripletFinderCfg.sortedByCotTheta = !m_useDetailedDoubleMeasurementInfo;
  m_tripletFinderCfg.minPt = m_minPt;
  m_tripletFinderCfg.sigmaScattering = m_sigmaScattering;
  m_tripletFinderCfg.radLengthPerSeed = m_radLengthPerSeed;
  m_tripletFinderCfg.maxPtScattering = m_maxPtScattering;
  m_tripletFinderCfg.impactMax = m_impactMax;
  m_tripletFinderCfg.helixCutTolerance = 1.;
  m_tripletFinderCfg.toleranceParam = m_toleranceParam;

  m_filterCfg.deltaInvHelixDiameter = m_deltaInvHelixDiameter;
  m_filterCfg.deltaRMin = m_deltaRMin;
  m_filterCfg.compatSeedWeight = m_compatSeedWeight;
  m_filterCfg.impactWeightFactor = m_impactWeightFactor;
  m_filterCfg.zOriginWeightFactor = m_zOriginWeightFactor;
  m_filterCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
  m_filterCfg.compatSeedLimit = m_compatSeedLimit;
  m_filterCfg.seedWeightIncrement = m_seedWeightIncrement;
  m_filterCfg.numSeedIncrement = m_numSeedIncrement;
  m_filterCfg.seedConfirmation = m_seedConfirmationInFilter;
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
  m_filterCfg.maxSeedsPerSpMConf = m_maxSeedsPerSpMConf;
  m_filterCfg.maxQualitySeedsPerSpMConf = m_maxQualitySeedsPerSpMConf;
  m_filterCfg.useDeltaRinsteadOfTopRadius = m_useDeltaRorTopRadius;

  m_finder =
      Acts::Experimental::TripletSeeder(logger().cloneWithSuffix("Finder"));

  m_loggerFilter = logger().cloneWithSuffix("Filter");

  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));

  return StatusCode::SUCCESS;
}

bool GridTripletSeedingTool::spacePointSelectionFunction(
    const xAOD::SpacePoint* sp, float r) const {
  float zabs = std::abs(sp->z());
  float absCotTheta = zabs / r;

  // checking configuration to remove pixel space points
  Identifier identifier = m_pixelId->wafer_id(sp->elementIdList().at(0));
  if (m_pixelId->is_barrel(identifier)) {
    if (zabs > 200 && r < 40)
      return false;

    return true;
  }

  // Inner layers
  // Below 1.20 - accept all
  static constexpr float cotThetaEta120 = 1.5095;
  if (absCotTheta < cotThetaEta120)
    return true;

  // Below 3.40 - remove if too close to beamline
  static constexpr float cotThetaEta340 = 14.9654;
  if (absCotTheta < cotThetaEta340 && r < m_expCutrMin)
    return false;

  // Outer layers
  // Above 2.20
  static constexpr float cotThetaEta220 = 4.4571;
  if (absCotTheta > cotThetaEta220 && r > 260.)
    return false;

  // Above 2.60
  static constexpr float cotThetaEta260 = 6.6947;
  if (absCotTheta > cotThetaEta260 && r > 200.)
    return false;

  // Above 3.20
  static constexpr float cotThetaEta320 = 12.2459;
  if (absCotTheta > cotThetaEta320 && r > 140.)
    return false;

  // Above 4.00
  static constexpr float cotThetaEta400 = 27.2899;
  if (absCotTheta > cotThetaEta400)
    return false;

  return true;
}

bool GridTripletSeedingTool::doubletSelectionFunction(
    const Acts::Experimental::ConstSpacePointProxy2& /*middle*/,
    const Acts::Experimental::ConstSpacePointProxy2& other, float cotTheta,
    bool isBottomCandidate) const {
  // We remove here some seeds, in case the bottom space point radius is
  // too small (i.e. < fastTrackingRMin)

  // This operation is done only within a specific eta window
  // Instead of eta we use the doublet cottheta
  static constexpr float cotThetaEta120 = 1.5095;
  static constexpr float cotThetaEta360 = 18.2855;

  float absCotTheta = std::abs(cotTheta);
  if (isBottomCandidate && other.zr()[1] < m_expCutrMin &&
      absCotTheta > cotThetaEta120 && absCotTheta < cotThetaEta360) {
    return false;
  }

  return true;
}

std::pair<float, float> GridTripletSeedingTool::retrieveRadiusRangeForMiddle(
    const Acts::Experimental::ConstSpacePointProxy2& spM,
    const Acts::Range1D<float>& rMiddleSpRange) const {
  if (m_useVariableMiddleSPRange) {
    return {rMiddleSpRange.min(), rMiddleSpRange.max()};
  }
  if (m_rRangeMiddleSP.empty()) {
    throw std::runtime_error(
        "m_rRangeMiddleSP is empty, please check the configuration.");
  }

  // get zBin position of the middle SP
  auto pVal =
      std::lower_bound(m_zBinEdges.begin(), m_zBinEdges.end(), spM.zr()[0]);
  int zBin = std::distance(m_zBinEdges.begin(), pVal);
  // protects against zM at the limit of zBinEdges
  zBin == 0 ? zBin : --zBin;
  return {m_rRangeMiddleSP[zBin][0], m_rRangeMiddleSP[zBin][1]};
}

StatusCode GridTripletSeedingTool::createSeeds2(
    const EventContext& ctx,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    ActsTrk::SeedContainer& seedContainer) const {
  (void)ctx;
  (void)beamSpotPos;

  auto gridCfg = m_gridCfg;
  gridCfg.bFieldInZ = bFieldInZ;

  Acts::Experimental::CylindricalSpacePointGrid2 grid(
      gridCfg, logger().cloneWithSuffix("Grid"));

  std::size_t totalSpacePoints = 0;
  for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
    totalSpacePoints += spacePoints->size();
  }

  std::vector<const xAOD::SpacePoint*> selectedXAODSpacePoints;
  std::vector<float> selectedSpacePointsR;
  selectedXAODSpacePoints.reserve(totalSpacePoints);
  selectedSpacePointsR.reserve(totalSpacePoints);

  for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
    for (const xAOD::SpacePoint* sp : *spacePoints) {
      float x = static_cast<float>(sp->x());
      float y = static_cast<float>(sp->y());
      float z = static_cast<float>(sp->z());
      float r = std::hypot(x, y);
      float phi = std::atan2(y, x);

      if (m_useExperimentCuts && !spacePointSelectionFunction(sp, r)) {
        continue;
      }

      grid.insert(selectedXAODSpacePoints.size(), phi, z, r);
      selectedXAODSpacePoints.push_back(sp);
      selectedSpacePointsR.push_back(r);
    }
  }

  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::ranges::sort(
        grid.at(i), [&](const Acts::Experimental::SpacePointIndex2& a,
                        const Acts::Experimental::SpacePointIndex2& b) {
          return selectedSpacePointsR[a] < selectedSpacePointsR[b];
        });
  }

  Acts::Experimental::SpacePointContainer2 selectedSpacePoints;
  selectedSpacePoints.createColumns(
      Acts::Experimental::SpacePointColumns::SourceLinks |
      Acts::Experimental::SpacePointColumns::XY |
      Acts::Experimental::SpacePointColumns::ZR |
      Acts::Experimental::SpacePointColumns::VarianceZ |
      Acts::Experimental::SpacePointColumns::VarianceR);
  if (m_useDetailedDoubleMeasurementInfo) {
    selectedSpacePoints.createColumns(
        Acts::Experimental::SpacePointColumns::Strip);
  }
  selectedSpacePoints.reserve(grid.numberOfSpacePoints());
  std::vector<Acts::Experimental::SpacePointIndex2> copyFromIndices;
  copyFromIndices.reserve(grid.numberOfSpacePoints());
  std::vector<Acts::Experimental::SpacePointIndexRange2> gridSpacePointRanges;
  gridSpacePointRanges.reserve(grid.numberOfBins());
  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::uint32_t begin = selectedSpacePoints.size();
    for (const Acts::Experimental::SpacePointIndex2 spIndex : grid.at(i)) {
      const xAOD::SpacePoint* sp = selectedXAODSpacePoints[spIndex];

      auto newSp = selectedSpacePoints.createSpacePoint();
      newSp.assignSourceLinks(
          std::array<Acts::SourceLink, 1>{Acts::SourceLink(sp)});
      newSp.xy() = std::array<float, 2>{static_cast<float>(sp->x()),
                                        static_cast<float>(sp->y())};
      newSp.zr() = std::array<float, 2>{static_cast<float>(sp->z()),
                                        selectedSpacePointsR[spIndex]};
      newSp.varianceZ() = static_cast<float>(sp->varianceZ());
      newSp.varianceR() = static_cast<float>(sp->varianceR());
      if (m_useDetailedDoubleMeasurementInfo) {
        newSp.topStripVector() =
            sp->topHalfStripLength() * sp->topStripDirection();
        newSp.bottomStripVector() =
            sp->bottomHalfStripLength() * sp->bottomStripDirection();
        newSp.stripCenterDistance() = sp->stripCenterDistance();
        newSp.topStripCenter() = sp->topStripCenter();
      }

      copyFromIndices.push_back(spIndex);
    }
    std::uint32_t end = selectedSpacePoints.size();
    gridSpacePointRanges.emplace_back(begin, end);
  }

  // clear temporary
  selectedXAODSpacePoints = {};
  selectedSpacePointsR = {};

  ACTS_VERBOSE("Number of space points after selection "
               << selectedSpacePoints.size() << " out of " << totalSpacePoints);

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
      minRange = std::min(first.zr()[1], minRange);
      maxRange = std::max(last.zr()[1], maxRange);
    }
    return {minRange, maxRange};
  }();

  auto bottomDoubletFinder = Acts::Experimental::DoubletSeedFinder::create(
      Acts::Experimental::DoubletSeedFinder::DerivedConfig(
          m_bottomDoubletFinderCfg, bFieldInZ));
  auto topDoubletFinder = Acts::Experimental::DoubletSeedFinder::create(
      Acts::Experimental::DoubletSeedFinder::DerivedConfig(
          m_topDoubletFinderCfg, bFieldInZ));
  auto tripletFinder = Acts::Experimental::TripletSeedFinder::create(
      Acts::Experimental::TripletSeedFinder::DerivedConfig(m_tripletFinderCfg,
                                                           bFieldInZ));

  // variable middle SP radial region of interest
  const Acts::Range1D<float> rMiddleSpRange(
      std::floor(rRange.min() / 2) * 2 + m_deltaRMiddleMinSPRange,
      std::floor(rRange.max() / 2) * 2 - m_deltaRMiddleMaxSPRange);

  Acts::Experimental::BroadTripletSeedFilter::State filterState;
  Acts::Experimental::BroadTripletSeedFilter::Cache filterCache;
  Acts::Experimental::TripletSeeder::Cache cache;

  Acts::Experimental::BroadTripletSeedFilter filter(
      m_filterCfg, filterState, filterCache, *m_loggerFilter);

  std::vector<Acts::Experimental::SpacePointContainer2::ConstRange>
      bottomSpRanges;
  std::optional<Acts::Experimental::SpacePointContainer2::ConstRange>
      middleSpRange;
  std::vector<Acts::Experimental::SpacePointContainer2::ConstRange> topSpRanges;

  Acts::Experimental::SeedContainer2 tmpSeedContainer;
  tmpSeedContainer.reserve(seedContainer.capacity());

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
        cache, *bottomDoubletFinder, *topDoubletFinder, *tripletFinder, filter,
        selectedSpacePoints, bottomSpRanges, *middleSpRange, topSpRanges,
        radiusRangeForMiddle, tmpSeedContainer);
  }

  // Selection function - temporary implementation
  // need change from ACTS for final implementation
  // To be used only on PPP
  auto selectionFunction =
      [&filterState](
          const Acts::Experimental::MutableSeedProxy2& seed) -> bool {
    float seedQuality = seed.quality();
    float bottomQuality =
        filterState.bestSeedQualityMap.at(seed.spacePointIndices()[0]);
    float middleQuality =
        filterState.bestSeedQualityMap.at(seed.spacePointIndices()[1]);
    float topQuality =
        filterState.bestSeedQualityMap.at(seed.spacePointIndices()[2]);

    return bottomQuality <= seedQuality || middleQuality <= seedQuality ||
           topQuality <= seedQuality;
  };

  seedContainer.reserve(tmpSeedContainer.size());

  // Select the seeds
  for (Acts::Experimental::MutableSeedProxy2 seed : tmpSeedContainer) {
    if (m_seedQualitySelection && !selectionFunction(seed)) {
      continue;
    }

    const xAOD::SpacePoint* bottom =
        selectedSpacePoints.at(seed.spacePointIndices()[0])
            .sourceLinks()[0]
            .get<const xAOD::SpacePoint*>();
    const xAOD::SpacePoint* middle =
        selectedSpacePoints.at(seed.spacePointIndices()[1])
            .sourceLinks()[0]
            .get<const xAOD::SpacePoint*>();
    const xAOD::SpacePoint* top =
        selectedSpacePoints.at(seed.spacePointIndices()[2])
            .sourceLinks()[0]
            .get<const xAOD::SpacePoint*>();

    auto outputSeed = std::make_unique<ActsTrk::Seed>(*bottom, *middle, *top);
    outputSeed->setVertexZ(seed.vertexZ());
    outputSeed->setQuality(seed.quality());
    seedContainer.push_back(std::move(outputSeed));
  }

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
