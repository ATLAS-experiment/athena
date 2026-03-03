/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
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
    printGbtsConfig(m_cfg);

     // layer geometry creation 
    const std::vector<TrigInDetSiLayer>* pVL = m_layerNumberTool->layerGeometry(); 
    
    m_layerGeometry.clear();
    m_layerGeometry.reserve(pVL->size());

    //convert from trigindetsilayer to acts::experimental::trigindetsilayer
    
    for (const TrigInDetSiLayer& s : *pVL) {
      m_layerGeometry.emplace_back(s.m_subdet, s.m_type, s.m_refCoord, s.m_minBound, s.m_maxBound);
    }
    
    //fill which has id for each module belongs to what layer
    m_sct_h2l = m_layerNumberTool->sctLayers();
    m_pix_h2l = m_layerNumberTool->pixelLayers();
    m_are_pixels.resize(m_layerNumberTool->maxNumberOfUniqueLayers(), true);
    for(const auto& l : *m_sct_h2l) m_are_pixels[l] = false;
    
    //parse connection 

    m_connector =  std::make_unique<Acts::Experimental::GbtsConnector>(m_cfg.connectorInputFile, m_cfg.lrtMode);

    // option that allows for adding custom eta binning (default is at 0.2)
    if (m_cfg.etaBinOverride != 0.0f) {

      m_connector->etaBin = m_cfg.etaBinOverride;
    }
  
    //create geoemtry object that holds allowed pairing of allowed eta regions in each layer 
    m_gbtsGeo = std::make_unique<Acts::Experimental::GbtsGeometry>( m_layerGeometry, m_connector);

    m_finder = std::make_unique<Acts::Experimental::GraphBasedTrackSeeder>(m_cfg, std::move(m_gbtsGeo), m_layerGeometry, logger().cloneWithSuffix("gbtsFinder"));
    
    return StatusCode::SUCCESS;
}

  //create seeds
  ATH_FLATTEN
  StatusCode GbtsSeedingTool::createSeeds2(
    const EventContext& ctx,
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
    ActsTrk::SeedContainer& seedContainer) const
  {
  //to avoid compile issues with unused veriables 
  (void) ctx;
  (void) bFieldInZ;
    //define new custom spacepoint container
    Acts::SpacePointContainer2 coreSpacePoints(
      Acts::SpacePointColumns::CopyFromIndex |
      Acts::SpacePointColumns::SourceLinks |
      Acts::SpacePointColumns::X |
      Acts::SpacePointColumns::Y |
      Acts::SpacePointColumns::Z |
      Acts::SpacePointColumns::R |
      Acts::SpacePointColumns::Phi
    );

    //add new column for layer ID and clusterwidth
    auto layerColumn = coreSpacePoints.createColumn<std::uint32_t>("layerId");
    auto clusterWidthColumn = coreSpacePoints.createColumn<float>("clusterWidth");
    auto localPositionColumn = coreSpacePoints.createColumn<float>("localPositionY");

    std::vector<const xAOD::SpacePoint*> tmpSpacePoints;
    std::size_t totalSpacePoints = 0;

    //add spacepoint pointers to singel container, this makes indexing them easier 
    for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
      for (const xAOD::SpacePoint* sp : *spacePoints) {
        tmpSpacePoints.emplace_back(sp);
      }
      totalSpacePoints += spacePoints->size();
    }

    coreSpacePoints.reserve(totalSpacePoints);

    //add spacepoints to new container
    for(std::size_t idx = 0; idx < tmpSpacePoints.size(); ++idx){
      //obtain module hash for spacepoint
      const xAOD::SpacePoint* sp = tmpSpacePoints[idx];
      const std::vector<xAOD::DetectorIDHashType>& elementlist = sp->elementIdList() ;

      bool isPixel(elementlist.size() == 1);
      if(isPixel == false) continue; //as currently strip hits are not used for seeding
    
	    short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->operator[](static_cast<int>(elementlist[0]));
      //obtain coordinates	

      auto newSp = coreSpacePoints.createSpacePoint();
      newSp.copyFromIndex() = idx;

      //apply beamspot corrections if needed
      if (m_cfg.beamSpotCorrection) {
        float new_x = static_cast<float>(sp->x() - beamSpotPos[0]);
        float new_y = static_cast<float>(sp->y() - beamSpotPos[1]);
        newSp.x() = new_x;
        newSp.y() = new_y;
        newSp.z() = static_cast<float>(sp->z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(new_y, new_x);
      } else {
        float new_x = static_cast<float>(sp->x());
        float new_y = static_cast<float>(sp->y());
        newSp.x() = static_cast<float>(sp->x());
        newSp.y() = static_cast<float>(sp->y());
        newSp.z() = static_cast<float>(sp->z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(sp->y(), sp->x());
      }

      newSp.extra(layerColumn) = layer;

      if (m_cfg.useMl) {
          assert(dynamic_cast<const xAOD::PixelCluster*>(sp->measurements().front())!=nullptr);
          const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(sp->measurements().front());
          newSp.extra(clusterWidthColumn) = pCL->widthInEta();
          newSp.extra(localPositionColumn) = pCL->localPosition<2>().y();
      }
    }
    ATH_MSG_VERBOSE("Spacepoints successfully added to new container");

    //compute seeds
    int max_layers = m_are_pixels.size(); 
    Acts::Experimental::RoiDescriptor internalRoi(0, -4.5, 4.5, 0, -std::numbers::pi, std::numbers::pi, 0, -150.0,150.0); //(eta,etaMinus,etaPlus,phi,phiMinus,Phiplus,z,zMinus,zPlus)
    Acts::SeedContainer2 seeds = m_finder->createSeeds(internalRoi, coreSpacePoints, max_layers); 

    //add seeds to the output container
    seedContainer.reserve(seedContainer.size() + seeds.size(), 7.0f);
    for (Acts::MutableSeedProxy2 seed : seeds) {
      seedContainer.push_back(
        Acts::ConstSeedProxy2(seed),
        [&](const Acts::SpacePointIndex2 spIndex) {
          return tmpSpacePoints[spIndex];
        });
    }

    ATH_MSG_VERBOSE("Number of seeds created is " << seedContainer.size());
    return StatusCode::SUCCESS;
  }

  
  //this is called in initialise
  //adds all veriables that may have been changed in the gaudi properties defined in headerfile 
  //TO DO: ADD NEW VERIABLES, DELETE OLD ONES AND CHANGE CURRENT ONES TO NEW VALUES 
  StatusCode
  GbtsSeedingTool::prepareConfiguration()
  {

    m_cfg.lrtMode = m_LRTmode;
    m_cfg.useMl = m_useML;
    m_cfg.matchBeforeCreate = m_matchBeforeCreate;
    m_cfg.useOldTunings = m_useOldTunings;
    m_cfg.etaBinOverride = m_etaBinOverride;
    m_cfg.beamSpotCorrection = m_BeamSpotCorrection;
    m_cfg.sigmaMS = m_sigmaMS;
    m_cfg.radLen = m_radLen;
    m_cfg.sigmaX = m_sigma_x;
    m_cfg.sigmaY = m_sigma_y;
    m_cfg.weightX = m_weight_x;
    m_cfg.weightY = m_weight_y;
    m_cfg.maxDChi2X = m_maxDChi2_x;
    m_cfg.maxDChi2Y = m_maxDChi2_y;
    m_cfg.addHit = m_add_hit;
    m_cfg.minPt = m_minPt;
    m_cfg.phiSliceWidth = m_phiSliceWidth ;
    m_cfg.nMaxPhiSlice = m_nMaxPhiSlice;
    m_cfg.connectorInputFile = m_connectorInputFile;
    m_cfg.lutInputFile = m_lutFile;
    m_cfg.useEtaBinning = m_useEtaBinning;
    m_cfg.doubletFilterRZ = m_doubletFilterRZ ;
    m_cfg.minDeltaRadius = m_minDeltaRadius;
    m_cfg.nMaxEdges = m_nMaxEdges;
    m_cfg.tauRatioCut = m_tau_ratio_cut; 
    m_cfg.tauRatioPrecut = m_tau_ratio_precut;
    m_cfg.ptCoeff = m_ptCoeff;
    m_cfg.maxCurvature = m_max_curvature;
    m_cfg.maxZ0 = m_max_z0;
    m_cfg.edgeMaskMinEta = m_edge_mask_min_eta;
    m_cfg.hitShareThreshold = m_hit_share_threshold;
    m_cfg.maxEndcapClusterWidth = m_max_endcap_clusterwidth;

    return StatusCode::SUCCESS;
  }
  //called in initialise, used to make sure all config settings look sensible
  void GbtsSeedingTool::printGbtsConfig(const Acts::Experimental::GbtsConfig& cfg) {
  ATH_MSG_DEBUG("===== GbtsConfig =====");
  ATH_MSG_DEBUG( "beamSpotCorrection: " << cfg.beamSpotCorrection << " (default: false)");
  ATH_MSG_DEBUG( "connectorInputFile: " << cfg.connectorInputFile << " (default: empty string)");
  ATH_MSG_DEBUG( "lutInputFile: " << cfg.lutInputFile << " (default: empty string)");
  ATH_MSG_DEBUG( "lrtMode: " << cfg.lrtMode << " (default: false)");
  ATH_MSG_DEBUG( "useMl: " << cfg.useMl << " (default: false)");
  ATH_MSG_DEBUG( "matchBeforeCreate: " << cfg.matchBeforeCreate << " (default: false)");
  ATH_MSG_DEBUG( "useOldTunings: " << cfg.useOldTunings << " (default: false)");
  ATH_MSG_DEBUG( "tauRatioPrecut: " << cfg.tauRatioPrecut << " (default: 0.009f)");
  ATH_MSG_DEBUG( "tauRatioCut: " << cfg.tauRatioCut << " (default: 0.007)");
  ATH_MSG_DEBUG( "etaBinOverride: " << cfg.etaBinOverride << " (default: 0.0)");
  ATH_MSG_DEBUG( "nMaxPhiSlice: " << cfg.nMaxPhiSlice << " (default: 53)");
  ATH_MSG_DEBUG( "minPt: " << cfg.minPt << " (default: 1000. MeV)");
  ATH_MSG_DEBUG( "phiSliceWidth: " << cfg.phiSliceWidth << " (default: null)");
  ATH_MSG_DEBUG( "ptCoeff: " << cfg.ptCoeff << " (default: 0.29955)");
  ATH_MSG_DEBUG( "useEtaBinning: " << cfg.useEtaBinning << " (default: true)");
  ATH_MSG_DEBUG( "doubletFilterRZ: " << cfg.doubletFilterRZ << " (default: true)");
  ATH_MSG_DEBUG( "nMaxEdges: " << cfg.nMaxEdges << " (default: 2000000)");
  ATH_MSG_DEBUG( "minDeltaRadius: " << cfg.minDeltaRadius << " (default: 2.0)");
  ATH_MSG_DEBUG( "sigmaMS: " << cfg.sigmaMS << " (default: 0.016)");
  ATH_MSG_DEBUG( "radLen: " << cfg.radLen << " (default: 0.025)");
  ATH_MSG_DEBUG( "sigmaX: " << cfg.sigmaX << " (default: 0.08)");
  ATH_MSG_DEBUG( "sigmaY: " << cfg.sigmaY << " (default: 0.25)");
  ATH_MSG_DEBUG( "weightX: " << cfg.weightX << " (default: 0.5)");
  ATH_MSG_DEBUG( "weightY: " << cfg.weightY << " (default: 0.5)");
  ATH_MSG_DEBUG( "maxDChi2X: " << cfg.maxDChi2X << " (default: 5.0)");
  ATH_MSG_DEBUG( "maxDChi2Y: " << cfg.maxDChi2Y << " (default: 6.0)");
  ATH_MSG_DEBUG( "addHit: " << cfg.addHit << " (default: 14.0)");
  ATH_MSG_DEBUG( "maxCurvature: " << cfg.maxCurvature << " (default: 1e-3f)");
  ATH_MSG_DEBUG( "maxZ0: " << cfg.maxZ0 << " (default: 170.0)");
  ATH_MSG_DEBUG( "edgeMaskMinEta: " << cfg.edgeMaskMinEta << " (default: 1.5)");
  ATH_MSG_DEBUG( "hitShareThreshold: " << cfg.hitShareThreshold << " (default: 0.49)");
  ATH_MSG_DEBUG( "maxEndcapClusterWidth: " << cfg.maxEndcapClusterWidth << " (default: 0.35)");

}

} // namespace ActsTrk
