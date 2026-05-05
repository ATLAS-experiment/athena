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

    ATH_CHECK( prepareConfiguration() );
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

    // define new custom spacepoint container
    Acts::SpacePointContainer2 coreSpacePoints(
      Acts::SpacePointColumns::CopyFromIndex |
      Acts::SpacePointColumns::SourceLinks |
      Acts::SpacePointColumns::X |
      Acts::SpacePointColumns::Y |
      Acts::SpacePointColumns::Z |
      Acts::SpacePointColumns::R |
      Acts::SpacePointColumns::Phi
    );

    // add new column for layer ID and clusterwidth
    auto layerColumn = coreSpacePoints.createColumn<std::uint32_t>("layerId");
    auto clusterWidthColumn = coreSpacePoints.createColumn<float>("clusterWidth");
    auto localPositionColumn = coreSpacePoints.createColumn<float>("localPositionY");

    std::vector<const xAOD::SpacePoint*> tmpSpacePoints;
    std::size_t totalSpacePoints = 0;

    // add spacepoint pointers to singel container, this makes indexing them easier 
    for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
      for (const xAOD::SpacePoint* sp : *spacePoints) {
        tmpSpacePoints.emplace_back(sp);
      }
      totalSpacePoints += spacePoints->size();
    }

    coreSpacePoints.reserve(totalSpacePoints);

    // add spacepoints to new container
    for(std::size_t idx = 0; idx < tmpSpacePoints.size(); ++idx){
      // obtain module hash for spacepoint
      const xAOD::SpacePoint* sp = tmpSpacePoints[idx];
      const std::vector<xAOD::DetectorIDHashType>& elementlist = sp->elementIdList();

      const bool isPixel(elementlist.size() == 1);
      // In LRT mode use strip spacepoints; in pixel mode use pixel spacepoints
      if (isPixel == m_finderCfg.lrtMode) continue;
    
	    const short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->operator[](static_cast<int>(elementlist[0]));

      auto newSp = coreSpacePoints.createSpacePoint();
      newSp.copyFromIndex() = idx;

      // apply beamspot corrections if needed
      if (m_finderCfg.beamSpotCorrection) {
        const float new_x = static_cast<float>(sp->x() - beamSpotPos[0]);
        const float new_y = static_cast<float>(sp->y() - beamSpotPos[1]);
        newSp.x() = new_x;
        newSp.y() = new_y;
        newSp.z() = static_cast<float>(sp->z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(new_y, new_x);
      } else {
        const float new_x = static_cast<float>(sp->x());
        const float new_y = static_cast<float>(sp->y());
        newSp.x() = static_cast<float>(sp->x());
        newSp.y() = static_cast<float>(sp->y());
        newSp.z() = static_cast<float>(sp->z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(sp->y(), sp->x());
      }

      newSp.extra(layerColumn) = layer;

      if (m_finderCfg.useMl && isPixel) {
        assert(dynamic_cast<const xAOD::PixelCluster*>(sp->measurements().front())!=nullptr);
        const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(sp->measurements().front());
        newSp.extra(clusterWidthColumn) = pCL->widthInEta();
        newSp.extra(localPositionColumn) = pCL->localPosition<2>().y();
      }
    }

    ATH_MSG_VERBOSE("Spacepoints successfully added to new container");

    const int max_layers = m_are_pixels.size();
    // eta,etaMinus,etaPlus,phi,phiMinus,Phiplus,z,zMinus,zPlus
    const Acts::Experimental::GbtsRoiDescriptor internalRoi(0, -4.5, 4.5, 0, -std::numbers::pi, std::numbers::pi, 0, -150.0,150.0);
    Acts::SeedContainer2 seeds;
    m_finder->createSeeds(coreSpacePoints, internalRoi, max_layers, *m_filter, options, seeds);

    // add seeds to the output container
    seedContainer.reserve(seedContainer.size() + seeds.size(), 7.0f);
    for (auto seed : seeds) {
      seedContainer.push_back(
        seed.asConst(),
        [&](const Acts::SpacePointIndex2 spIndex) {
          return tmpSpacePoints[spIndex];
        });
    }

    ATH_MSG_VERBOSE("Number of seeds created is " << seedContainer.size());
    return StatusCode::SUCCESS;
  }

  // this is called in initialise
  // adds all veriables that may have been changed in the gaudi properties defined in headerfile 
  // TODO: ADD NEW VERIABLES, DELETE OLD ONES AND CHANGE CURRENT ONES TO NEW VALUES 
  StatusCode GbtsSeedingTool::prepareConfiguration() {
    m_finderCfg.lrtMode = m_LRTmode;
    m_finderCfg.useMl = m_useML;
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
    m_finderCfg.tauRatioCut = m_tau_ratio_cut; 
    m_finderCfg.tauRatioPrecut = m_tau_ratio_precut;
    m_finderCfg.edgeMaskMinEta = m_edge_mask_min_eta;
    m_finderCfg.hitShareThreshold = m_hit_share_threshold;
    m_finderCfg.maxEndcapClusterWidth = m_max_endcap_clusterwidth;
    m_finderCfg.d0Max = m_d0_max;
    m_finderCfg.validateTriplets = m_validateTriplets;
    m_finderCfg.useAdaptiveCuts = m_useAdaptiveCuts;
    m_finderCfg.tauRatioCorr = m_tau_ratio_corr;
    m_finderCfg.addTriplets = m_addTriplets;
    m_finderCfg.maxAbsEtaAddTripelts = m_maxEtaAddTriplets;
    m_filterCfg.sigmaMS = m_sigmaMS;
    m_filterCfg.radLen = m_radLen;
    m_filterCfg.sigmaX = m_sigma_x;
    m_filterCfg.sigmaY = m_sigma_y;
    m_filterCfg.weightX = m_weight_x;
    m_filterCfg.weightY = m_weight_y;
    m_filterCfg.maxDChi2X = m_maxDChi2_x;
    m_filterCfg.maxDChi2Y = m_maxDChi2_y;
    m_filterCfg.addHit = m_add_hit;
    m_filterCfg.maxCurvature = m_max_curvature;
    m_filterCfg.maxZ0 = m_max_z0;

    return StatusCode::SUCCESS;
  }

  // called in initialise, used to make sure all config settings look sensible
  void GbtsSeedingTool::printGbtsConfig() const {
    ATH_MSG_DEBUG("===== GBTS finder config =====");
    ATH_MSG_DEBUG( "beamSpotCorrection: " << m_finderCfg.beamSpotCorrection);
    ATH_MSG_DEBUG( "connectorInputFile: " << m_finderCfg.connectorInputFile);
    ATH_MSG_DEBUG( "lutInputFile: " << m_finderCfg.lutInputFile);
    ATH_MSG_DEBUG( "lrtMode: " << m_finderCfg.lrtMode);
    ATH_MSG_DEBUG( "useMl: " << m_finderCfg.useMl);
    ATH_MSG_DEBUG( "matchBeforeCreate: " << m_finderCfg.matchBeforeCreate);
    ATH_MSG_DEBUG( "useOldTunings: " << m_finderCfg.useOldTunings);
    ATH_MSG_DEBUG( "tauRatioPrecut: " << m_finderCfg.tauRatioPrecut);
    ATH_MSG_DEBUG( "tauRatioCut: " << m_finderCfg.tauRatioCut);
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
    ATH_MSG_DEBUG( "addTriplets: " << m_finderCfg.addTriplets);
    ATH_MSG_DEBUG( "maxEtaAddTriplets " << m_finderCfg.maxAbsEtaAddTripelts);
    
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
    ATH_MSG_DEBUG( "maxZ0: " << m_filterCfg.maxZ0);
  }

} // namespace ActsTrk
