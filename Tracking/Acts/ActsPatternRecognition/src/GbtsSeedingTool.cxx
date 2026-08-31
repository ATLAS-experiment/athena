/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#if defined(FLATTEN) && defined(__GNUC__)
// Avoid warning in dbg build
#pragma GCC optimize "-fno-var-tracking-assignments"
#endif

#include "src/GbtsSeedingTool.h"

#include "CxxUtils/inline_hints.h"

namespace ActsTrk {

  GbtsSeedingTool::GbtsSeedingTool(const std::string& type,
    const std::string& name,
    const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode GbtsSeedingTool::initialize() {
    ATH_CHECK(m_layerNumberTool.retrieve());
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    // Make the logger And Propagate to ACTS routines
    m_logger = makeActsAthenaLogger(this, "Acts");

    // etaMin,etaMax,zMin,zMax
    m_internalRoi.emplace(-4.5, 4.5, -150.0, 150.0);

    ATH_CHECK( prepareConfiguration());
    printGbtsConfig();

    // layer geometry creation 
    const std::vector<TrigInDetSiLayer>* pVL = m_layerNumberTool->layerGeometry(); 
    
    // layer objects used by GBTS
    std::vector<Acts::Experimental::GbtsLayerDescription> layers;
    layers.reserve(pVL->size());

    // convert from trigindetsilayer to acts::experimental::trigindetsilayer
    for (const TrigInDetSiLayer&s : *pVL) {
      const Acts::Experimental::GbtsLayerType type = s.m_type == 0 ? Acts::Experimental::GbtsLayerType::Barrel : Acts::Experimental::GbtsLayerType::Endcap;
      layers.emplace_back(s.m_subdet, type, s.m_refCoord, s.m_minBound, s.m_maxBound);
    }

    // fill which has id for each module belongs to what layer
    m_sct_h2l = m_layerNumberTool->sctLayers();
    m_pix_h2l = m_layerNumberTool->pixelLayers();
    m_are_pixels.resize(m_layerNumberTool->maxNumberOfUniqueLayers(), true);
    for(const auto& l : *m_sct_h2l) m_are_pixels[l] = false;

    // parse connection 
    auto layerConnectionMap = Acts::Experimental::GbtsLayerConnectionMap::fromFile(m_finderCfg.connectorInputFile, m_finderCfg.lrtMode);

    // option that allows for adding custom eta binning (default is at 0.2)
    if (m_finderCfg.etaBinWidthOverride != 0.0f) {
      layerConnectionMap.etaBinWidth = m_finderCfg.etaBinWidthOverride;
    }
  
    // create geoemtry object that holds allowed pairing of allowed eta regions in each layer
    // holds all geometry information (m_layergeomtry and connection table)
    auto gbtsGeo = std::make_shared<Acts::Experimental::GbtsGeometry>(layers, layerConnectionMap);

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
    Acts::Experimental::GbtsNodeStorage nodeStorage = m_finder->makeNodeStorage(m_are_pixels);

    // add spacepoints to node storage
    for(std::size_t idx = 0; idx < tmpSpacePoints.size(); ++idx){
      // obtain module hash for spacepoint
      const xAOD::SpacePoint* sp = tmpSpacePoints[idx];
      const std::vector<xAOD::DetectorIDHashType>& elementlist = sp->elementIdList();

      const bool isPixel(elementlist.size() == 1);

	    const short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->operator[](static_cast<int>(elementlist[0]));

      float clusterWidth = 0.0f;
      float localPositionY = 0.0f;
      if (m_finderCfg.useClusterWidthCuts && isPixel) {
        assert(dynamic_cast<const xAOD::PixelCluster*>(sp->measurements().front())!=nullptr);
        const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(sp->measurements().front());
        clusterWidth = pCL->widthInEta();
        localPositionY = pCL->localPosition<2>().y();
      }

      if (m_finderCfg.beamSpotCorrection) {
        const float new_x = static_cast<float>(sp->x() - beamSpotPos[0]);
        const float new_y = static_cast<float>(sp->y() - beamSpotPos[1]);
        nodeStorage.insert(static_cast<Acts::SpacePointIndex>(idx), new_x, new_y, static_cast<float>(sp->z()),
          std::hypot(new_x, new_y), std::atan2(new_y, new_x),
          static_cast<std::uint32_t>(layer), clusterWidth, localPositionY);
      } else {
        const float new_x = static_cast<float>(sp->x());
        const float new_y = static_cast<float>(sp->y());
        nodeStorage.insert(static_cast<Acts::SpacePointIndex>(idx), new_x, new_y, static_cast<float>(sp->z()),
          std::hypot(new_x, new_y), static_cast<float>(std::atan2(sp->y(), sp->x())),
          static_cast<std::uint32_t>(layer), clusterWidth, localPositionY);
      }
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
  
  StatusCode GbtsSeedingTool::prepareConfiguration() {
    m_finderCfg.lrtMode = m_LRTmode;
    m_finderCfg.useClusterWidthCuts = m_useML;
    m_finderCfg.matchBeforeCreate = m_matchBeforeCreate;
    m_finderCfg.useOldTunings = m_useOldTunings;
    m_finderCfg.etaBinWidthOverride = m_etaBinWidthOverride;
    m_finderCfg.beamSpotCorrection = m_beamSpotCorrection;
    m_finderCfg.minPt = m_minPt;
    m_finderCfg.nMaxPhiSlice = m_nMaxPhiSlice;
    m_finderCfg.connectorInputFile = m_connectorInputFile;
    m_finderCfg.lutInputFile = m_lutFile;
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
    m_finderCfg.maxAbsEtaAddTripelts = m_maxEtaAddTriplets;
    m_finderCfg.cutDPhiMax = m_cutDPhiMax;
    m_finderCfg.cutDCurvMax = m_cutDCurvMax;
    m_finderCfg.minDeltaPhi = m_minDeltaPhi;
    m_finderCfg.maxOuterRadius = m_maxOuterRadius;

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
  ATH_MSG_DEBUG( "connectorInputFile: " << m_finderCfg.connectorInputFile);
  ATH_MSG_DEBUG( "lutInputFile: " << m_finderCfg.lutInputFile);
  ATH_MSG_DEBUG( "lrtMode: " << m_finderCfg.lrtMode);
  ATH_MSG_DEBUG( "useClusterWidthCuts: " << m_finderCfg.useClusterWidthCuts);
  ATH_MSG_DEBUG( "matchBeforeCreate: " << m_finderCfg.matchBeforeCreate);
  ATH_MSG_DEBUG( "useOldTunings: " << m_finderCfg.useOldTunings);
  ATH_MSG_DEBUG( "tauRatioPrecut: " << m_finderCfg.tauRatioPrecut);
  ATH_MSG_DEBUG( "tauRatioCut: " << m_finderCfg.tauRatioCut);
  ATH_MSG_DEBUG( "tauRatioCorr: " << m_finderCfg.tauRatioCorr);
  ATH_MSG_DEBUG( "etaBinWidthOverride: " << m_finderCfg.etaBinWidthOverride);
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
  ATH_MSG_DEBUG( "maxEtaAddTriplets: " << m_finderCfg.maxAbsEtaAddTripelts);
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
