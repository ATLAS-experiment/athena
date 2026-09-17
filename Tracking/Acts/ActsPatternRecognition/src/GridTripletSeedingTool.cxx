/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GridTripletSeedingTool.h"

#include <cmath>
#include <cstdint>
#include <numbers>
#include <span>
#include <string_view>
#include <vector>

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
  ATH_MSG_DEBUG("   " << m_useHVCollisionRegion);
  ATH_CHECK(m_inputHoughVtxKey.initialize(m_useHVCollisionRegion));
  if(m_useHVCollisionRegion) {
    ATH_MSG_DEBUG("   " << m_inputHoughVtxKey);
    ATH_MSG_DEBUG("   " << m_hvCollisionRegionTolerance);
  }
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

  // Both edge vectors define the bins of the corresponding grid axis, so n
  // edges give n-1 bins. Guard against an empty vector before subtracting, the
  // sizes are unsigned.
  if (m_zBinEdges.size() < 2 || m_rBinEdges.size() < 2) {
    ATH_MSG_ERROR("zBinEdges and rBinEdges must each contain at least two "
                  "edges, got "
                  << m_zBinEdges.size() << " and " << m_rBinEdges.size());
    return StatusCode::FAILURE;
  }
  const std::size_t nZBins = m_zBinEdges.size() - 1;
  const std::size_t nRBins = m_rBinEdges.size() - 1;

  // The neighbour vectors hold one entry per bin and are indexed 0-based by
  // Acts::GridBinFinder. An empty vector means "one neighbour on each side".
  auto checkNeighbors = [this](std::string_view name,
                               std::span<const std::pair<int, int>> values,
                               std::size_t nBins) {
    if (values.empty() || values.size() == nBins) {
      return true;
    }
    ATH_MSG_ERROR("Inconsistent config " << name << ": got " << values.size()
                                         << " entries but the grid has "
                                         << nBins << " bins");
    return false;
  };

  if (!checkNeighbors("zBinNeighborsTop", m_zBinNeighborsTop.value(), nZBins) ||
      !checkNeighbors("zBinNeighborsBottom", m_zBinNeighborsBottom.value(),
                      nZBins) ||
      !checkNeighbors("rBinNeighborsTop", m_rBinNeighborsTop.value(), nRBins) ||
      !checkNeighbors("rBinNeighborsBottom", m_rBinNeighborsBottom.value(),
                      nRBins)) {
    return StatusCode::FAILURE;
  }

  // The custom looping vectors are the Acts::BinnedGroup navigation and use
  // the 1-based local bin numbering of the grid axis: valid entries run from 1
  // to the number of bins. Bin 0 is the underflow bin, which never holds a
  // space point, so listing it would silently drop an entry from the loop. A
  // vector may list a subset of the bins in order to skip the remaining ones,
  // but must not repeat a bin. An empty vector means all bins in their natural
  // order.
  auto checkLooping = [this](std::string_view name,
                             std::span<const std::size_t> bins,
                             std::size_t nBins) {
    std::vector<bool> visited(nBins + 1, false);
    for (std::size_t i : bins) {
      if (i == 0 || i > nBins) {
        ATH_MSG_ERROR("Inconsistent config "
                      << name << ": bin " << i
                      << " is out of range, the numbering is 1-based and the "
                         "grid has "
                      << nBins << " bins (valid entries are 1.." << nBins
                      << ")");
        return false;
      }
      if (visited[i]) {
        ATH_MSG_ERROR("Inconsistent config " << name << ": bin " << i
                                             << " is listed more than once");
        return false;
      }
      visited[i] = true;
    }
    return true;
  };

  if (!checkLooping("zBinsCustomLooping", m_zBinsCustomLooping.value(),
                    nZBins) ||
      !checkLooping("rBinsCustomLooping", m_rBinsCustomLooping.value(),
                    nRBins)) {
    return StatusCode::FAILURE;
  }

  // rRangeMiddleSP is indexed 0-based by z bin in retrieveRadiusRangeForMiddle,
  // so it needs exactly one entry per z bin. It is only read when the variable
  // middle range is disabled.
  if (!m_useVariableMiddleSPRange && m_rRangeMiddleSP.size() != nZBins) {
    ATH_MSG_ERROR("Inconsistent config rRangeMiddleSP: got "
                  << m_rRangeMiddleSP.size()
                  << " entries but the grid has " << nZBins << " z bins");
    return StatusCode::FAILURE;
  }

  std::visit([&](auto& cfg) {
      //common settings for spherical and cylindrical grids
      cfg.minPt = m_minPt;
      cfg.rMin = 0;
      cfg.rMax = m_gridRMax;
      cfg.deltaRMax = m_deltaRMax;
      cfg.impactMax = m_impactMax;
      cfg.phiMin = m_gridPhiMin;
      cfg.phiMax = m_gridPhiMax;
      cfg.phiBinDeflectionCoverage = m_phiBinDeflectionCoverage;
      cfg.maxPhiBins = m_maxPhiBins;
      cfg.rBinEdges = m_rBinEdges;
      cfg.bFieldInZ = 0;  // will be set later


      if constexpr (std::is_same_v<std::decay_t<decltype(cfg)>, Acts::Experimental::SphericalSpacePointGrid::Config>){
        cfg.etaMin = m_etaMin;
        cfg.etaMax = m_etaMax;
        cfg.etaBinEdges = m_etaBinEdges;
        cfg.bottomBinFinder = Acts::GridBinFinder<3ul>(
            m_numPhiNeighbors.value(), m_numEtaNeighbors.value(),
            m_rBinNeighborsBottom.value());
        cfg.topBinFinder = Acts::GridBinFinder<3ul>(m_numPhiNeighbors.value(),
                                                          m_numEtaNeighbors.value(),
                                                          m_rBinNeighborsTop.value());
      }

      else {
        cfg.zMin = m_zMin;
        cfg.zMax = m_zMax;
        cfg.cotThetaMax = m_cotThetaMax;
        cfg.zBinEdges = m_zBinEdges;
        cfg.bottomBinFinder = Acts::GridBinFinder<3ul>(
            m_numPhiNeighbors.value(), m_zBinNeighborsBottom.value(),
            m_rBinNeighborsBottom.value());
        cfg.topBinFinder = Acts::GridBinFinder<3ul>(m_numPhiNeighbors.value(),
                                                          m_zBinNeighborsTop.value(),
                                                          m_rBinNeighborsTop.value());
        cfg.navigation[0ul] = {};
        cfg.navigation[1ul] = m_zBinsCustomLooping;
        cfg.navigation[2ul] = m_rBinsCustomLooping;
      }
    }, m_gridCfg
  );

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
  m_topDoubletFinderCfg = m_bottomDoubletFinderCfg;  // copy the bottom cuts
  m_topDoubletFinderCfg.candidateDirection = Acts::Direction::Forward();
  m_topDoubletFinderCfg.deltaRMin = m_deltaRMinTopSP;
  m_topDoubletFinderCfg.deltaRMax = m_deltaRMaxTopSP;

  m_tripletFinderCfg.useStripInfo = m_useDetailedDoubleMeasurementInfo;
  m_tripletFinderCfg.sortedByCotTheta = true;
  m_tripletFinderCfg.minPt = m_minPt;
  m_tripletFinderCfg.sigmaScattering = m_sigmaScattering;
  m_tripletFinderCfg.radLengthPerSeed = m_radLengthPerSeed;
  m_tripletFinderCfg.impactMax = m_impactMax;
  m_tripletFinderCfg.helixCutTolerance = 1.;
  m_tripletFinderCfg.toleranceParam = m_toleranceParam;
  m_tripletFinderCfg.cotThetaDiffMax = m_maxStripDeltaCotTheta;
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
  m_filterCfg.absDeltaEtaWeightFactor = m_absDeltaEtaWeightFactor;
  m_filterCfg.absDeltaEtaMinImpact = m_absDeltaEtaMinImpact;

  m_finder = Acts::TripletSeeder(logger().cloneWithSuffix("Finder"));

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
    const std::vector<float>& spPhi, const std::vector<float>& spAsinD0OverR,
    const Acts::ConstSpacePointProxy& middle,
    const Acts::ConstSpacePointProxy& other, float cotTheta,
    bool isBottomCandidate) const {
  if (m_doubletDPhiCut) {
    // per-pair azimuthal-swing bound: the hit azimuth of a track with impact
    // parameter d0 swings between two radii by asin(d0/rInner) -
    // asin(d0/rOuter) on top of the curvature rotation. The grid phi-bin
    // widening only knows the full radial span; this applies the exact
    // per-pair bound before the doublet enters the triplet stage.
    // NB: this container only fills the packed coordinate columns, so the
    // packed accessor zr() must be used here.
    const float rM = middle.zr()[1];
    const float rO = other.zr()[1];
    const float rInner = std::min(rM, rO);
    const float rOuter = std::max(rM, rO);

    // phi(SP) and asin(d0/r) depend only on the SP (d0 is constant from the
    // config), so they are computed once per SP in createSeeds and looked up
    // via copiedFromIndex, avoiding two atan2 and two asin calls per
    // candidate. asin(d0/r) is monotone in r, so the inner-minus-outer swing
    // equals the absolute difference of the two per-SP terms.
    const auto iM = middle.copiedFromIndex();
    const auto iO = other.copiedFromIndex();
    float dPhi = spPhi[iO] - spPhi[iM];
    const float swing = std::abs(spAsinD0OverR[iO] - spAsinD0OverR[iM]);
    if (dPhi > std::numbers::pi_v<float>) {
      dPhi -= 2.f * std::numbers::pi_v<float>;
    } else if (dPhi < -std::numbers::pi_v<float>) {
      dPhi += 2.f * std::numbers::pi_v<float>;
    }
    const float bound = m_doubletDPhiConst +
                        m_doubletDPhiSlope * (rOuter - rInner) +
                        std::min(m_doubletDPhiCap.value(), swing);

    if (std::abs(dPhi) > bound) {
      return false;
    }
  }

  if (!m_useExperimentCuts) {
    return true;
  }

  // We remove some doublets that have the middle space point in some specific
  // areas This should eventually be moved inside ACTS and allow a veto
  // mechanism according to the user desire. As of now we cannot really do this
  // since we define a range of validity of the middle candidate, and if we want
  // to veto some sub-regions inside it, we need to do it here.
  if (std::abs(middle.zr()[0]) > 1500 and middle.zr()[1] > 100 and
      middle.zr()[1] < 150) {
    return false;
  }

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
    const Acts::ConstSpacePointProxy& spM,
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

template<typename GridType>
StatusCode GridTripletSeedingTool::createSeedsImpl(
    const EventContext& ctx,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    ActsTrk::SeedContainer& seedContainer, GridType::Config gridCfg) const {
  (void)ctx;

  gridCfg.bFieldInZ = bFieldInZ;

  GridType grid(gridCfg, logger().cloneWithSuffix("Grid"));

  std::size_t totalSpacePoints = 0;
  for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
    totalSpacePoints += spacePoints->size();
  }

  std::vector<const xAOD::SpacePoint*> selectedXAODSpacePoints;
  std::vector<float> selectedSpacePointsR;
  selectedXAODSpacePoints.reserve(totalSpacePoints);
  selectedSpacePointsR.reserve(totalSpacePoints);
  // Per-SP inputs for the doublet dPhi selection (see
  // doubletSelectionFunction); only filled when the cut is enabled.
  std::vector<float> selectedSpacePointsPhi;
  std::vector<float> selectedSpacePointsAsinD0OverR;
  const float dPhiCutD0 =
      m_doubletDPhiD0Max < 0.f ? m_impactMax.value() : m_doubletDPhiD0Max.value();
  if (m_doubletDPhiCut) {
    selectedSpacePointsPhi.reserve(totalSpacePoints);
    selectedSpacePointsAsinD0OverR.reserve(totalSpacePoints);
  }

  for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
    for (const xAOD::SpacePoint* sp : *spacePoints) {
      float x = static_cast<float>(sp->x() - beamSpotPos[0]);
      float y = static_cast<float>(sp->y() - beamSpotPos[1]);
      float z = static_cast<float>(sp->z());
      float r = std::hypot(x, y);
      float phi = std::atan2(y, x);

      if (m_useExperimentCuts && !spacePointSelectionFunction(sp, r)) {
        continue;
      }

      //This takes care of using translating z -> z/r in the spherical grid insertion
      SPGridTraits<GridType>::insert(grid, selectedXAODSpacePoints.size(), phi, z, r);
      selectedXAODSpacePoints.push_back(sp);
      selectedSpacePointsR.push_back(r);
      if (m_doubletDPhiCut) {
        selectedSpacePointsPhi.push_back(phi);
        selectedSpacePointsAsinD0OverR.push_back(
            std::asin(std::min(1.f, dPhiCutD0 / std::max(r, 1.f))));
      }
    }
  }

  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::ranges::sort(
        grid.at(i), [&](Acts::SpacePointIndex a, Acts::SpacePointIndex b) {
          return selectedSpacePointsR[a] < selectedSpacePointsR[b];
        });
  }

  Acts::SpacePointContainer selectedSpacePoints;
  selectedSpacePoints.createColumns(
      Acts::SpacePointColumns::CopiedFromIndex |
      Acts::SpacePointColumns::PackedXY | Acts::SpacePointColumns::PackedZR |
      Acts::SpacePointColumns::VarianceZ | Acts::SpacePointColumns::VarianceR);
  if (m_useDetailedDoubleMeasurementInfo) {
    selectedSpacePoints.createColumns(
        Acts::SpacePointColumns::StripCalibrationDetails);
  }
  selectedSpacePoints.reserve(grid.numberOfSpacePoints());
  std::vector<Acts::SpacePointIndexRange> gridSpacePointRanges;
  gridSpacePointRanges.reserve(grid.numberOfBins());
  for (std::size_t i = 0; i < grid.numberOfBins(); ++i) {
    std::uint32_t begin = selectedSpacePoints.size();
    for (const Acts::SpacePointIndex spIndex : grid.at(i)) {
      const xAOD::SpacePoint* sp = selectedXAODSpacePoints[spIndex];

      auto newSp = selectedSpacePoints.createSpacePoint();
      newSp.copiedFromIndex() = spIndex;
      newSp.xy() =
          std::array<float, 2>{static_cast<float>(sp->x() - beamSpotPos[0]),
                               static_cast<float>(sp->y() - beamSpotPos[1])};
      newSp.zr() = std::array<float, 2>{static_cast<float>(sp->z()),
                                        selectedSpacePointsR[spIndex]};
      newSp.varianceZ() = static_cast<float>(sp->varianceZ());
      newSp.varianceR() = static_cast<float>(sp->varianceR());

      if (m_useDetailedDoubleMeasurementInfo) {
        const Eigen::Vector3f innerStripHalfVector =
            sp->bottomHalfStripLength() * sp->bottomStripDirection();
        const Eigen::Vector3f outerStripCenter = sp->topStripCenter();
        const Eigen::Vector3f outerStripHalfVector =
            sp->topHalfStripLength() * sp->topStripDirection();
        const Eigen::Vector3f stripSeparation = sp->stripCenterDistance();

        newSp.outerStripCalibrationDetails().outerCenter = std::array<float, 3>{
            outerStripCenter.x(), outerStripCenter.y(), outerStripCenter.z()};
        newSp.outerStripCalibrationDetails().innerToOuterSeparation =
            std::array<float, 3>{stripSeparation.x(), stripSeparation.y(),
                                 stripSeparation.z()};
        newSp.outerStripCalibrationDetails().outerHalfVector =
            std::array<float, 3>{outerStripHalfVector.x(),
                                 outerStripHalfVector.y(),
                                 outerStripHalfVector.z()};
        newSp.outerStripCalibrationDetails().innerHalfVector =
            std::array<float, 3>{innerStripHalfVector.x(),
                                 innerStripHalfVector.y(),
                                 innerStripHalfVector.z()};
      }
    }
    std::uint32_t end = selectedSpacePoints.size();
    gridSpacePointRanges.emplace_back(begin, end);
  }

  // clear temporary
  selectedSpacePointsR = {};

  ACTS_VERBOSE("Number of space points after selection "
               << selectedSpacePoints.size() << " out of " << totalSpacePoints);

  // Compute radius range. We rely on the fact the grid is storing the proxies
  // with a sorting in the radius
  const Acts::Range1D<float> rRange = [&]() -> Acts::Range1D<float> {
    float minRange = std::numeric_limits<float>::max();
    float maxRange = std::numeric_limits<float>::lowest();
    for (const Acts::SpacePointIndexRange& range : gridSpacePointRanges) {
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

  auto bottomDoubletFinderCfg = m_bottomDoubletFinderCfg;
  auto topDoubletFinderCfg = m_topDoubletFinderCfg;

  // set up the cache for the doublet selection function, which needs to know the per-SP phi and
  // asin(d0/r) values for the middle and other SPs. The cache is filled in
  // createSeeds, and the selection function is connected to the doublet finders below.
  auto doubletSelection =
      [this, &selectedSpacePointsPhi, &selectedSpacePointsAsinD0OverR](
          const Acts::ConstSpacePointProxy& middle,
          const Acts::ConstSpacePointProxy& other, float cotTheta,
          bool isBottomCandidate) {
        return doubletSelectionFunction(selectedSpacePointsPhi,
                                        selectedSpacePointsAsinD0OverR, middle,
                                        other, cotTheta, isBottomCandidate);
      };
  if (m_useExperimentCuts || m_doubletDPhiCut) {
    bottomDoubletFinderCfg.experimentCuts.connect(doubletSelection);
    topDoubletFinderCfg.experimentCuts.connect(doubletSelection);
  }

  if(m_useHVCollisionRegion) {
    SG::ReadHandle<xAOD::VertexContainer> inputHoughVtx = SG::makeHandle(m_inputHoughVtxKey, ctx);
    ATH_CHECK(inputHoughVtx.isValid());

    for(const auto* vtx: *inputHoughVtx)
    {
      if(vtx->vertexType() == xAOD::VxType::PriVtx)
      {
        bottomDoubletFinderCfg.collisionRegionMin = vtx->z() - m_hvCollisionRegionTolerance;
        bottomDoubletFinderCfg.collisionRegionMax = vtx->z() + m_hvCollisionRegionTolerance;
        break;
      }
    }
    // in case HoughVtx is not found and inputHoughVtx is empty, keep the original collision region
  }

  auto bottomDoubletFinder =
      Acts::DoubletSeedFinder::create(Acts::DoubletSeedFinder::DerivedConfig(
          bottomDoubletFinderCfg, bFieldInZ));
  auto topDoubletFinder = Acts::DoubletSeedFinder::create(
      Acts::DoubletSeedFinder::DerivedConfig(topDoubletFinderCfg, bFieldInZ));
  auto tripletFinder = Acts::TripletSeedFinder::create(
      Acts::TripletSeedFinder::DerivedConfig(m_tripletFinderCfg, bFieldInZ));

  // variable middle SP radial region of interest
  const Acts::Range1D<float> rMiddleSpRange(
      std::floor(rRange.min() / 2) * 2 + m_deltaRMiddleMinSPRange,
      std::floor(rRange.max() / 2) * 2 - m_deltaRMiddleMaxSPRange);

  Acts::BroadTripletSeedFilter::State filterState;
  Acts::BroadTripletSeedFilter::Cache filterCache;
  Acts::TripletSeeder::Cache cache;

  Acts::BroadTripletSeedFilter filter(m_filterCfg, filterState, filterCache,
                                      *m_loggerFilter);

  std::vector<Acts::SpacePointContainer::ConstRange> bottomSpRanges;
  std::optional<Acts::SpacePointContainer::ConstRange> middleSpRange;
  std::vector<Acts::SpacePointContainer::ConstRange> topSpRanges;

  Acts::SeedContainer tmpSeedContainer;

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
        [&](std::size_t b) -> Acts::SpacePointContainer::ConstRange {
          return selectedSpacePoints.range(gridSpacePointRanges[b]).asConst();
        });
    middleSpRange =
        selectedSpacePoints.range(gridSpacePointRanges[middle]).asConst();
    std::ranges::transform(
        top, std::back_inserter(topSpRanges),
        [&](std::size_t t) -> Acts::SpacePointContainer::ConstRange {
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
      [&filterState](const Acts::MutableSeedProxy& seed) -> bool {
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

  seedContainer.reserve(seedContainer.size() + tmpSeedContainer.size());

  // Select and convert the seeds
  for (Acts::MutableSeedProxy seed : tmpSeedContainer) {
    if (m_seedQualitySelection && !selectionFunction(seed)) {
      continue;
    }

    seedContainer.push_back(
        Acts::ConstSeedProxy(seed), [&](const Acts::SpacePointIndex spIndex) {
          const Acts::SpacePointIndex originalIndex =
              selectedSpacePoints.at(spIndex).copiedFromIndex();
          return selectedXAODSpacePoints[originalIndex];
        });
  }

  return StatusCode::SUCCESS;
}

template<>
struct SPGridTraits<Acts::CylindricalSpacePointGrid> {

  static void insert(auto& grid, std::size_t index, float phi, float z, float r) {
      grid.insert(index, phi, z, r);
  }

};

template<>
struct SPGridTraits<Acts::Experimental::SphericalSpacePointGrid> {  

  static void insert(auto& grid, std::size_t index, float phi, float z, float r) {
      grid.insert(index, phi, z/r, r);
  }

};


StatusCode GridTripletSeedingTool::createSeeds(
    const EventContext& ctx,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    ActsTrk::SeedContainer& seedContainer) const {

      if (m_sphericalGrid) return createSeedsImpl<Acts::Experimental::SphericalSpacePointGrid>(ctx, spacePointCollections, beamSpotPos, bFieldInZ, seedContainer, std::get<Acts::Experimental::SphericalSpacePointGrid::Config>(m_gridCfg));
      else                 return createSeedsImpl<Acts::CylindricalSpacePointGrid>(ctx, spacePointCollections, beamSpotPos, bFieldInZ, seedContainer, std::get<Acts::CylindricalSpacePointGrid::Config>(m_gridCfg));



    }
}  // namespace ActsTrk
