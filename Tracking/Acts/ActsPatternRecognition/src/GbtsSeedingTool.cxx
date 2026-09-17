/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#if defined(FLATTEN) && defined(__GNUC__)
// Avoid warning in dbg build
#pragma GCC optimize "-fno-var-tracking-assignments"
#endif

#include "src/GbtsSeedingTool.h"

#include "src/GbtsConnectionTableReader.h"

#include "CxxUtils/inline_hints.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <utility>

namespace ActsTrk {

  GbtsSeedingTool::GbtsSeedingTool(const std::string& type,
    const std::string& name,
    const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode GbtsSeedingTool::initialize() {
    ATH_CHECK(m_layerTool.retrieve());
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    // Make the logger And Propagate to ACTS routines
    m_logger = makeActsAthenaLogger(this, "Acts");

    // etaMin,etaMax,zMin,zMax
    m_internalRoi.emplace(-4.5, 4.5, -150.0, 150.0);

    ATH_CHECK( prepareConfiguration());
    printGbtsConfig();

    // The layer tool builds the GBTS layers, in dense layer index order, and
    // knows which layer each module hash belongs to and what it is made of.
    const std::vector<Acts::Experimental::GbtsLayerDescription>& layers =
      m_layerTool->layerDescriptions();

    m_pixelHashToLayer = &m_layerTool->pixelLayers();
    m_stripHashToLayer = &m_layerTool->stripLayers();

    if (!m_usePixelLayers && !m_useStripLayers) {
      ATH_MSG_ERROR("Neither pixel nor strip layers are enabled, there is "
                    "nothing to seed on.");
      return StatusCode::FAILURE;
    }

    std::vector<Acts::Experimental::GbtsLayerConnection> connections;
    float etaBinWidth = 0.0f;
    ATH_CHECK(readConnections(layers, connections, etaBinWidth));

    // option that allows for adding custom eta binning (default is at 0.2)
    if (m_etaBinWidthOverride.value() != 0.0f) {
      etaBinWidth = m_etaBinWidthOverride.value();
    }

    // the cluster width cuts are the only user of the tau lookup table
    if (m_finderCfg.useClusterWidthCuts) {
      ATH_CHECK(readTauLookupTable(m_finderCfg.tauLookupTable));
    }

    // create geoemtry object that holds allowed pairing of allowed eta regions in each layer
    // holds all geometry information (m_layergeomtry and connection table)
    auto gbtsGeo = std::make_shared<Acts::Experimental::GbtsGeometry>(
      layers, connections, etaBinWidth, Acts::Experimental::GbtsZ0Range{}, logger());

    m_finder = Acts::Experimental::GraphBasedTrackSeeder(
      Acts::Experimental::GraphBasedTrackSeeder::DerivedConfig(m_finderCfg),
      gbtsGeo, logger().cloneWithSuffix("gbtsFinder"));

    m_filter = Acts::Experimental::GbtsTrackingFilter(m_filterCfg, gbtsGeo);

    return StatusCode::SUCCESS;
  }

  ATH_FLATTEN
  StatusCode GbtsSeedingTool::createSeeds(
    const EventContext& ctx,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    ActsTrk::SeedContainer& seedContainer) const
  {
    // to avoid compile issues with unused veriables 
    (void) ctx;

    const Acts::Experimental::GraphBasedTrackSeeder::Options options(bFieldInZ);


    std::vector<const xAOD::SpacePoint*> tmpSpacePoints;

    // add spacepoint pointers to singel container, this makes indexing them easier 
    for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
      for (const xAOD::SpacePoint* sp : *spacePoints) {
        tmpSpacePoints.emplace_back(sp);
      }
    }


    // create the node storage and fill it from the xAOD space points
    Acts::Experimental::GbtsNodeStorage nodeStorage = m_finder->makeNodeStorage();

    // node positions are relative to the beam spot in x and y if the correction is on
    const float offsetX = m_finderCfg.beamSpotCorrection ? beamSpotPos[0] : 0.0f;
    const float offsetY = m_finderCfg.beamSpotCorrection ? beamSpotPos[1] : 0.0f;

    std::size_t nPixelNodes = 0;
    std::size_t nStripNodes = 0;

    // space points GBTS has no layer for, counted rather than reported per
    // space point: the loop runs over the whole event
    std::size_t nUnmappedHashes = 0;
    std::size_t nUngroupedModules = 0;

    // add spacepoints to node storage
    for(std::size_t idx = 0; idx < tmpSpacePoints.size(); ++idx){
      // obtain module hash for spacepoint
      const xAOD::SpacePoint* sp = tmpSpacePoints[idx];
      const std::vector<xAOD::DetectorIDHashType>& elementlist = sp->elementIdList();

      // a strip space point is made of one cluster on each side of a stereo pair
      const bool isPixel(elementlist.size() == 1);

      if (isPixel ? !m_usePixelLayers : !m_useStripLayers) {
        continue;
      }

      const std::vector<short>& hashToLayer =
        isPixel ? *m_pixelHashToLayer : *m_stripHashToLayer;
      const auto hash = static_cast<std::size_t>(elementlist[0]);
      if (hash >= hashToLayer.size()) [[unlikely]] {
        ++nUnmappedHashes;
        continue;
      }

      const short layer = hashToLayer[hash];
      if (layer == IGbtsLayerTool::kNoLayer) {
        // a wafer GBTS does not group into any of its layers
        ++nUngroupedModules;
        continue;
      }

      float clusterWidth = 0.0f;
      float localPositionY = 0.0f;
      if (m_finderCfg.useClusterWidthCuts && isPixel) {
        assert(dynamic_cast<const xAOD::PixelCluster*>(sp->measurements().front())!=nullptr);
        const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(sp->measurements().front());
        clusterWidth = pCL->widthInEta();
        localPositionY = pCL->localPosition<2>().y();
      }

      // the stereo pair of a strip space point, for the strip calibration in GBTS
      Acts::OuterStripSpacePointCalibrationDetails stripDetails{};
      const Acts::OuterStripSpacePointCalibrationDetails* strip = nullptr;
      if (!isPixel) {
        // topStripCenter is global, the node frame shifts only x and y
        Eigen::Map<Eigen::Vector3f>(stripDetails.outerCenter.data()) =
          sp->topStripCenter() - Eigen::Vector3f(offsetX, offsetY, 0.0f);
        Eigen::Map<Eigen::Vector3f>(stripDetails.innerToOuterSeparation.data()) =
          sp->stripCenterDistance();
        Eigen::Map<Eigen::Vector3f>(stripDetails.outerHalfVector.data()) =
          sp->topHalfStripLength() * sp->topStripDirection();
        Eigen::Map<Eigen::Vector3f>(stripDetails.innerHalfVector.data()) =
          sp->bottomHalfStripLength() * sp->bottomStripDirection();
        strip = &stripDetails;
      }

      const float x = static_cast<float>(sp->x()) - offsetX;
      const float y = static_cast<float>(sp->y()) - offsetY;
      const std::optional<std::uint32_t> bin = nodeStorage.insert(
        static_cast<Acts::SpacePointIndex>(idx), x, y,
        static_cast<float>(sp->z()), std::hypot(x, y), std::atan2(y, x),
        static_cast<std::uint32_t>(layer), clusterWidth, localPositionY, strip);

      if (bin.has_value()) {
        ++(isPixel ? nPixelNodes : nStripNodes);
      }
    }

    ATH_MSG_DEBUG("Inserted " << nPixelNodes << " pixel and " << nStripNodes
                  << " strip nodes; the graph "
                  << (nodeStorage.hasStrips() ? "carries" : "does not carry")
                  << " stereo pairs");

    if (nUnmappedHashes != 0) [[unlikely]] {
      ATH_MSG_WARNING(nUnmappedHashes << " space points sit on a wafer hash "
                      "outside the GBTS layer map and were dropped");
    }
    if (nUngroupedModules != 0) {
      ATH_MSG_DEBUG(nUngroupedModules << " space points sit on a wafer GBTS "
                    "does not group into a layer");
    }

    // order the nodes and build the derived per-node data
    nodeStorage.finalize();

    ATH_MSG_VERBOSE("Spacepoints successfully added to node storage");

    Acts::SeedContainer seeds;
    m_finder->createSeeds(nodeStorage, m_internalRoi.value(), *m_filter, options, seeds);

    // add seeds to the output container
    seedContainer.reserve(seedContainer.size() + seeds.size(), 7.0f);
    for (auto seed : seeds) {
      seedContainer.push_back(
        seed.asConst(),
        [&](const Acts::SpacePointIndex spIndex) {
          return tmpSpacePoints[spIndex];
        });
    }

    ATH_MSG_VERBOSE("Number of seeds created is " << seedContainer.size());
    return StatusCode::SUCCESS;
  }

  // this is called in initialise
  // adds all veriables that may have been changed in the gaudi properties defined in headerfile 
  
  StatusCode GbtsSeedingTool::readConnections(
    const std::vector<Acts::Experimental::GbtsLayerDescription>& layers,
    std::vector<Acts::Experimental::GbtsLayerConnection>& connections,
    float& etaBinWidth) const
  {
    std::ifstream connectionStream(m_connectorInputFile.value());
    if (!connectionStream.is_open()) {
      ATH_MSG_ERROR("Cannot open the GBTS connection table "
                    << m_connectorInputFile.value());
      return StatusCode::FAILURE;
    }

    GbtsConnectionTable::ReadResult table;
    try {
      table = GbtsConnectionTable::read(connectionStream);
    } catch (const std::exception& e) {
      ATH_MSG_ERROR("Cannot read " << m_connectorInputFile.value() << ": "
                    << e.what());
      return StatusCode::FAILURE;
    }

    // the table names a layer by its id, the layer tool by its dense index
    std::unordered_map<std::uint32_t, Acts::Experimental::GbtsLayerTechnology>
      layerTechnologies;
    layerTechnologies.reserve(layers.size());
    for (const Acts::Experimental::GbtsLayerDescription& layer : layers) {
      layerTechnologies.emplace(static_cast<std::uint32_t>(layer.id),
                                layer.technology);
    }

    etaBinWidth = table.etaBinWidth;

    // the stage column only fixes the order the connections are handed over in
    std::vector<std::pair<std::uint32_t, Acts::Experimental::GbtsLayerConnection>> staged;
    staged.reserve(table.connections.size());

    std::size_t nOtherTechnology = 0;
    std::size_t nUnknownLayer = 0;

    for (const GbtsConnectionTable::Connection& connection :
         table.connections) {
      const auto src = layerTechnologies.find(connection.src);
      const auto dst = layerTechnologies.find(connection.dst);
      if (src == layerTechnologies.end() || dst == layerTechnologies.end()) {
        ++nUnknownLayer;
        continue;
      }

      // GBTS pairs a layer only with one of its own technology
      const bool wanted = src->second == dst->second &&
                          (src->second ==
                             Acts::Experimental::GbtsLayerTechnology::Pixel
                             ? m_pixelConnections.value()
                             : m_stripConnections.value());
      if (!wanted) {
        ++nOtherTechnology;
        continue;
      }

      staged.emplace_back(connection.stage,
                          Acts::Experimental::GbtsLayerConnection{connection.src,
                                                                  connection.dst});
    }

    std::ranges::stable_sort(staged, {}, [](const auto& entry) { return entry.first; });

    connections.clear();
    connections.reserve(staged.size());
    for (const auto& entry : staged) {
      connections.push_back(entry.second);
    }
    const std::size_t nKept = connections.size();

    if (nUnknownLayer != 0) {
      ATH_MSG_WARNING(nUnknownLayer << " connections of "
                      << m_connectorInputFile.value() << " name no GBTS layer "
                      "and were dropped");
    }
    if (nKept == 0) {
      ATH_MSG_ERROR("None of the connections of "
                    << m_connectorInputFile.value() << " are usable: the table "
                    "does not match the detector this job is reconstructing");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Kept " << nKept << " GBTS layer connections, dropping "
                  << nOtherTechnology << " of a technology not asked for, eta bin width "
                  << etaBinWidth);

    return StatusCode::SUCCESS;
  }

  StatusCode GbtsSeedingTool::readTauLookupTable(
    Acts::Experimental::detail::GbtsTauLookupTable& tauLookupTable) const
  {
    std::ifstream lutStream(m_lutFile.value());
    if (!lutStream.is_open()) {
      ATH_MSG_ERROR("Cannot open the GBTS tau lookup table " << m_lutFile.value());
      return StatusCode::FAILURE;
    }

    tauLookupTable.clear();

    // the width is dropped: a row is located by index, one row per
    // tauLutBinWidth of cluster width, never searched
    float clusterWidth = 0.0f;
    Acts::Experimental::detail::GbtsTauBounds bounds;
    while (lutStream >> clusterWidth >> bounds.minTau >> bounds.maxTau >>
           bounds.minTauNearEdge >> bounds.maxTauNearEdge) {
      tauLookupTable.push_back(bounds);
    }

    if (!lutStream.eof()) {
      // ended on a parse error, not on a clean end of file
      ATH_MSG_ERROR("Malformed GBTS tau lookup table " << m_lutFile.value());
      return StatusCode::FAILURE;
    }
    if (tauLookupTable.empty()) {
      ATH_MSG_ERROR("The GBTS tau lookup table " << m_lutFile.value() << " is empty");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Read " << tauLookupTable.size() << " rows of the GBTS tau lookup table "
                  << m_lutFile.value());

    return StatusCode::SUCCESS;
  }

  StatusCode GbtsSeedingTool::prepareConfiguration() {
    m_finderCfg.useStripConnections = m_stripConnections;
    m_finderCfg.useClusterWidthCuts = m_useML;
    m_finderCfg.matchBeforeCreate = m_matchBeforeCreate;
    // useOldTunings gated the curvature bounds and the phi window together,
    // while LRT mode only wanted the first, so the seeder now has them apart
    m_finderCfg.useOldTuningsCurvature = m_useOldTunings || m_LRTmode;
    m_finderCfg.useOldTuningsPhiWindow = m_useOldTunings;
    m_finderCfg.beamSpotCorrection = m_beamSpotCorrection;
    m_finderCfg.minPt = m_minPt;
    m_finderCfg.nMaxPhiSlice = m_nMaxPhiSlice;
    m_finderCfg.useEtaBinning = m_useEtaBinning;
    m_finderCfg.doubletFilterRZ = m_doubletFilterRZ;
    m_finderCfg.minDeltaRadius = m_minDeltaRadius;
    m_finderCfg.nMaxEdges = m_nMaxEdges;
    m_finderCfg.tauRatioCut = m_tauRatioCut; 
    m_finderCfg.tauRatioPrecut = m_tauRatioPrecut;
    m_finderCfg.edgeMaskMinEta = m_edgeMaskMinEta;
    m_finderCfg.hitShareThreshold = m_hitShareThreshold;
    m_finderCfg.maxEndcapClusterWidth = m_maxEndcapClusterwidth;
    m_finderCfg.d0Max = m_d0Max;

    //use roi for pixel and given value for strip
    m_finderCfg.maxZ0 = m_LRTmode ? m_maxZ0.value() : m_internalRoi->zMax();
    m_finderCfg.minZ0 = m_LRTmode ? m_minZ0.value() : m_internalRoi->zMin();

    m_finderCfg.validateTriplets = m_validateTriplets;
    m_finderCfg.useAdaptiveCuts = m_useAdaptiveCuts;
    m_finderCfg.tauRatioCorr = m_tauRatioCorr;
    m_finderCfg.addTriplets = m_addTriplets;
    m_finderCfg.maxAbsEtaAddTriplets = m_maxEtaAddTriplets;
    m_finderCfg.cutDPhiMax = m_cutDPhiMax;
    m_finderCfg.cutDCurvMax = m_cutDCurvMax;
    m_finderCfg.minDeltaPhi = m_minDeltaPhi;
    m_finderCfg.maxOuterRadius = m_maxOuterRadius;

    // The seeder no longer recognises an LRT mode, so spell out the rest of
    // what it used to imply: the whole of maxCurv for the curvature bounds and
    // the phi window, a triplet with no confirmation, and no added triplets.
    // Keep this last, it overrides addTriplets.
    if (m_LRTmode) {
      m_finderCfg.oldTuningsCurvatureHighEtaFraction = 1.f;
      m_finderCfg.oldTuningsCurvatureLowEtaFraction = 1.f;
      m_finderCfg.oldTuningsPhiWindowFraction = 1.f;
      m_finderCfg.minSeedLevel = 2;
      m_finderCfg.addTriplets = false;
    }

    m_filterCfg.sigmaMS = m_sigmaMS;
    m_filterCfg.radLen = m_radLen;
    m_filterCfg.sigmaX = m_sigmaX;
    m_filterCfg.sigmaY = m_sigmaY;
    m_filterCfg.weightX = m_weightX;
    m_filterCfg.weightY = m_weightY;
    m_filterCfg.maxDChi2X = m_maxDChi2X;
    m_filterCfg.maxDChi2Y = m_maxDChi2Y;
    m_filterCfg.addHit = m_addHit;
    m_filterCfg.maxCurvature = m_maxCurvature;
    m_filterCfg.maxZ0 = m_filterMaxZ0;

    return StatusCode::SUCCESS;
  }

  // called in initialise, used to make sure all config settings look sensible
void GbtsSeedingTool::printGbtsConfig() const {
  ATH_MSG_DEBUG("===== GBTS finder config =====");
  ATH_MSG_DEBUG( "beamSpotCorrection: " << m_finderCfg.beamSpotCorrection);
  ATH_MSG_DEBUG( "connectorInputFile: " << m_connectorInputFile.value());
  ATH_MSG_DEBUG( "lutInputFile: " << m_lutFile.value());
  ATH_MSG_DEBUG( "LRTmode: " << m_LRTmode.value());
  ATH_MSG_DEBUG( "useStripConnections: " << m_finderCfg.useStripConnections);
  ATH_MSG_DEBUG( "useClusterWidthCuts: " << m_finderCfg.useClusterWidthCuts);
  ATH_MSG_DEBUG( "matchBeforeCreate: " << m_finderCfg.matchBeforeCreate);
  ATH_MSG_DEBUG( "useOldTuningsCurvature: " << m_finderCfg.useOldTuningsCurvature);
  ATH_MSG_DEBUG( "useOldTuningsPhiWindow: " << m_finderCfg.useOldTuningsPhiWindow);
  ATH_MSG_DEBUG( "minSeedLevel: " << m_finderCfg.minSeedLevel);
  ATH_MSG_DEBUG( "tauRatioPrecut: " << m_finderCfg.tauRatioPrecut);
  ATH_MSG_DEBUG( "tauRatioCut: " << m_finderCfg.tauRatioCut);
  ATH_MSG_DEBUG( "tauRatioCorr: " << m_finderCfg.tauRatioCorr);
  ATH_MSG_DEBUG( "etaBinWidthOverride: " << m_etaBinWidthOverride.value());
  ATH_MSG_DEBUG( "nMaxPhiSlice: " << m_finderCfg.nMaxPhiSlice);
  ATH_MSG_DEBUG( "minPt: " << m_finderCfg.minPt);
  ATH_MSG_DEBUG( "useEtaBinning: " << m_finderCfg.useEtaBinning);
  ATH_MSG_DEBUG( "doubletFilterRZ: " << m_finderCfg.doubletFilterRZ);
  ATH_MSG_DEBUG( "nMaxEdges: " << m_finderCfg.nMaxEdges);
  ATH_MSG_DEBUG( "minDeltaRadius: " << m_finderCfg.minDeltaRadius);
  ATH_MSG_DEBUG( "edgeMaskMinEta: " << m_finderCfg.edgeMaskMinEta);
  ATH_MSG_DEBUG( "hitShareThreshold: " << m_finderCfg.hitShareThreshold);
  ATH_MSG_DEBUG( "maxEndcapClusterWidth: " << m_finderCfg.maxEndcapClusterWidth);
  ATH_MSG_DEBUG( "d0Max: " << m_finderCfg.d0Max);
  ATH_MSG_DEBUG("maxZ0: " << m_finderCfg.maxZ0);
  ATH_MSG_DEBUG("minZ0: " << m_finderCfg.minZ0);
  ATH_MSG_DEBUG( "validateTriplets: " << m_finderCfg.validateTriplets);
  ATH_MSG_DEBUG( "useAdaptiveCuts: " << m_finderCfg.useAdaptiveCuts);
  ATH_MSG_DEBUG( "addTriplets: " << m_finderCfg.addTriplets);
  ATH_MSG_DEBUG( "maxEtaAddTriplets: " << m_finderCfg.maxAbsEtaAddTriplets);
  ATH_MSG_DEBUG("cutDphiMax: " << m_finderCfg.cutDPhiMax);
  ATH_MSG_DEBUG("cutDCurvMax: " << m_finderCfg.cutDCurvMax);
  ATH_MSG_DEBUG("minDeltaPhi: " << m_finderCfg.minDeltaPhi);
  ATH_MSG_DEBUG("maxOuterRadius: " << m_finderCfg.maxOuterRadius);

  ATH_MSG_DEBUG("===== GBTS filter config =====");
  ATH_MSG_DEBUG( "sigmaMS: " << m_filterCfg.sigmaMS);
  ATH_MSG_DEBUG( "radLen: " << m_filterCfg.radLen);
  ATH_MSG_DEBUG( "sigmaX: " << m_filterCfg.sigmaX);
  ATH_MSG_DEBUG( "sigmaY: " << m_filterCfg.sigmaY);
  ATH_MSG_DEBUG( "weightX: " << m_filterCfg.weightX);
  ATH_MSG_DEBUG( "weightY: " << m_filterCfg.weightY);
  ATH_MSG_DEBUG( "maxDChi2X: " << m_filterCfg.maxDChi2X);
  ATH_MSG_DEBUG( "maxDChi2Y: " << m_filterCfg.maxDChi2Y);
  ATH_MSG_DEBUG( "addHit: " << m_filterCfg.addHit);
  ATH_MSG_DEBUG( "maxCurvature: " << m_filterCfg.maxCurvature);
  ATH_MSG_DEBUG( "filterMaxZ0: " << m_filterCfg.maxZ0);
}

} // namespace ActsTrk
