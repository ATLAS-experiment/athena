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
    printSeedFinderGbtsConfig(m_finderCfg);

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

    m_connector =  std::make_unique<Acts::Experimental::GbtsConnector>(m_finderCfg.connectorInputFile, m_finderCfg.LRTmode);

    // option that allows for adding custom eta binning (default is at 0.2)
    if (m_finderCfg.etaBinOverride != 0.0f) {

      m_connector->m_etaBin = m_finderCfg.etaBinOverride;
    }
  
    //create geoemtry object that holds allowed pairing of allowed eta regions in each layer 
    m_gbtsGeo = std::make_unique<Acts::Experimental::GbtsGeometry>( m_layerGeometry, m_connector);

    m_finder = std::make_unique<Acts::Experimental::SeedFinderGbts>(m_finderCfg, std::move(m_gbtsGeo), &m_layerGeometry, logger().cloneWithSuffix("gbtsFinder"));
    
    return StatusCode::SUCCESS;
}

  //create seeds
  ATH_FLATTEN
  StatusCode
  GbtsSeedingTool::createSeeds(const EventContext& ctx,
			       const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
			       const Acts::Vector3& beamSpotPos,
			       const Acts::Vector3& bField,
			       ActsTrk::SeedContainer& seedContainer ) const
  {
  //to avoid compile issues with unused veriables 
  (void) ctx;
  (void) bField;
    //define new custom spacepoint container
    Acts::SpacePointContainer2 coreSpacePoints(
      Acts::SpacePointColumns::SourceLinks |
      Acts::SpacePointColumns::X |
      Acts::SpacePointColumns::Y |
      Acts::SpacePointColumns::Z |
      Acts::SpacePointColumns::R |
      Acts::SpacePointColumns::Phi
    );

    //add new coloumn for layer ID and clusterwidth
    auto LayerColoumn = coreSpacePoints.createColumn<int>("LayerID");
    auto ClusterWidthColoumn = coreSpacePoints.createColumn<float>("Cluster_Width");
    auto LocalPositionColoumn = coreSpacePoints.createColumn<float>("LocalPositionY");
    coreSpacePoints.reserve(spContainer.size());

    //add spacepoints to new container and seedContainer
    seedContainer.spacePoints().reserve(spContainer.size());
    for(size_t idx=0; idx<spContainer.size(); idx++){
      //obtain module hash for spacepoint
      const auto & sp = spContainer.at(idx);
      const auto & extSP = sp.externalSpacePoint();
      seedContainer.spacePoints().push_back(&extSP);
      const std::vector<xAOD::DetectorIDHashType>& elementlist = extSP.elementIdList() ;

      bool isPixel(elementlist.size() == 1);
      if(isPixel == false) continue; //as currently strip hits are not used for seeding
    
	    short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->at(static_cast<int>(elementlist[0]));
      //obtain coordinates	

      auto newSp = coreSpacePoints.createSpacePoint();

      //apply beamspot corrections if needed
      if(m_finderCfg.BeamSpotCorrection){
        
        float new_x = static_cast<float>(extSP.x() - beamSpotPos[0]);
        float new_y = static_cast<float>(extSP.y() - beamSpotPos[1]);
        newSp.x() = new_x;
        newSp.y() = new_y;
        newSp.z() = static_cast<float>(extSP.z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(new_y, new_x);
        
      }else{
        float new_x = static_cast<float>(extSP.x());
        float new_y = static_cast<float>(extSP.y());
        newSp.x() = static_cast<float>(extSP.x());
        newSp.y() = static_cast<float>(extSP.y());
        newSp.z() = static_cast<float>(extSP.z());
        newSp.r() = std::hypot(new_x, new_y);
        newSp.phi() = std::atan2(extSP.y(), extSP.x());
      }
      
      newSp.extra(LayerColoumn) = layer;
      
      if(m_finderCfg.useML){
          
          assert(dynamic_cast<const xAOD::PixelCluster*>(extSP.measurements().front())!=nullptr);
          const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(extSP.measurements().front());
          newSp.extra(ClusterWidthColoumn) = pCL->widthInEta();
          newSp.extra(LocalPositionColoumn) = pCL->localPosition<2>().y();
          
        }else{
          newSp.extra(ClusterWidthColoumn) = 0;
          newSp.extra(LocalPositionColoumn) = 0;
          
        }
    }    
    ATH_MSG_VERBOSE("Spacepoints successfully added to new container");
    
    //collect all spacepoint containers objects so they can be passed into the seedfinder
    auto SPContainerComponents = std::make_tuple(std::move(coreSpacePoints), LayerColoumn.asConst(), ClusterWidthColoumn.asConst(), LocalPositionColoumn.asConst());

    //compute seeds
    int max_layers = m_are_pixels.size(); 
    Acts::Experimental::RoiDescriptor internalRoi(0, -4.5, 4.5, 0, -std::numbers::pi, std::numbers::pi, 0, -150.0,150.0); //(eta,etaMinus,etaPlus,phi,phiMinus,Phiplus,z,zMinus,zPlus)
    Acts::SeedContainer2 seeds = m_finder->createSeeds(internalRoi, SPContainerComponents, max_layers); 
    
    
    //add seeds to the output container
    seedContainer.reserve(seeds.size(), 7.0f);
    for (Acts::MutableSeedProxy2 seed : seeds) {
    
      seedContainer.push_back(seed);
    } 
    ATH_MSG_VERBOSE("Number of seeds created is" << seedContainer.size());
    return StatusCode::SUCCESS;
  }

  
  //this is called in initialise
  //adds all veriables that may have been changed in the gaudi properties defined in headerfile 
  //TO DO: ADD NEW VERIABLES, DELETE OLD ONES AND CHANGE CURRENT ONES TO NEW VALUES 
  StatusCode
  GbtsSeedingTool::prepareConfiguration()
  {

    m_finderCfg.LRTmode = m_LRTmode;
    m_finderCfg.useML = m_useML;
    m_finderCfg.matchBeforeCreate = m_matchBeforeCreate;
    m_finderCfg.useOldTunings = m_useOldTunings;
    m_finderCfg.etaBinOverride = m_etaBinOverride;
    m_finderCfg.BeamSpotCorrection = m_BeamSpotCorrection;
    m_finderCfg.sigmaMS = m_sigmaMS;
    m_finderCfg.radLen = m_radLen;
    m_finderCfg.sigma_x = m_sigma_x;
    m_finderCfg.sigma_y = m_sigma_y;
    m_finderCfg.weight_x = m_weight_x;
    m_finderCfg.weight_y = m_weight_y;
    m_finderCfg.maxDChi2_x = m_maxDChi2_x;
    m_finderCfg.maxDChi2_y = m_maxDChi2_y;
    m_finderCfg.add_hit = m_add_hit;
    m_finderCfg.minPt = m_minPt;
    m_finderCfg.phiSliceWidth = m_phiSliceWidth ;
    m_finderCfg.nMaxPhiSlice = m_nMaxPhiSlice;
    m_finderCfg.useML = m_useML;
    m_finderCfg.connectorInputFile = m_connectorInputFile;
    m_finderCfg.lutInputFile = m_lutFile;
    m_finderCfg.useEtaBinning = m_useEtaBinning;
    m_finderCfg.doubletFilterRZ = m_doubletFilterRZ ;
    m_finderCfg.minDeltaRadius = m_minDeltaRadius;
    m_finderCfg.nMaxEdges = m_nMaxEdges;
    m_finderCfg.tau_ratio_cut = m_tau_ratio_cut; 
    m_finderCfg.tau_ratio_precut = m_tau_ratio_precut;
    m_finderCfg.ptCoeff = m_ptCoeff;
    m_finderCfg.max_curvature = m_max_curvature;
    m_finderCfg.max_z0 = m_max_z0;
    m_finderCfg.edge_mask_min_eta = m_edge_mask_min_eta;
    m_finderCfg.hit_share_threshold = m_hit_share_threshold;
    m_finderCfg.max_endcap_clusterwidth = m_max_endcap_clusterwidth;
    m_finderCfg = m_finderCfg.toInternalUnits();



    return StatusCode::SUCCESS;
  }
  //called in initialise, used to make sure all config settings look sensible
  void GbtsSeedingTool::printSeedFinderGbtsConfig(const Acts::Experimental::SeedFinderGbtsConfig& cfg) {
  ATH_MSG_DEBUG("===== SeedFinderGbtsConfig =====");
  ATH_MSG_DEBUG( "BeamSpotCorrection: " << cfg.BeamSpotCorrection << " (default: false)");
  ATH_MSG_DEBUG( "connectorInputFile: " << cfg.connectorInputFile << " (default: empty string)");
  ATH_MSG_DEBUG( "lutInputFile: " << cfg.lutInputFile << " (default: empty string)");
  ATH_MSG_DEBUG( "LRTmode: " << cfg.LRTmode << " (default: false)");
  ATH_MSG_DEBUG( "useML: " << cfg.useML << " (default: false)");
  ATH_MSG_DEBUG( "matchBeforeCreate: " << cfg.matchBeforeCreate << " (default: false)");
  ATH_MSG_DEBUG( "useOldTunings: " << cfg.useOldTunings << " (default: false)");
  ATH_MSG_DEBUG( "tau_ratio_precut: " << cfg.tau_ratio_precut << " (default: 0.009f)");
  ATH_MSG_DEBUG( "tau_ratio_cut: " << cfg.tau_ratio_cut << " (default: 0.007)");
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
  ATH_MSG_DEBUG( "sigma_x: " << cfg.sigma_x << " (default: 0.08)");
  ATH_MSG_DEBUG( "sigma_y: " << cfg.sigma_y << " (default: 0.25)");
  ATH_MSG_DEBUG( "weight_x: " << cfg.weight_x << " (default: 0.5)");
  ATH_MSG_DEBUG( "weight_y: " << cfg.weight_y << " (default: 0.5)");
  ATH_MSG_DEBUG( "maxDChi2_x: " << cfg.maxDChi2_x << " (default: 5.0)");
  ATH_MSG_DEBUG( "maxDChi2_y: " << cfg.maxDChi2_y << " (default: 6.0)");
  ATH_MSG_DEBUG( "add_hit: " << cfg.add_hit << " (default: 14.0)");
  ATH_MSG_DEBUG( "max_curvature: " << cfg.max_curvature << " (default: 1e-3f)");
  ATH_MSG_DEBUG( "max_z0: " << cfg.max_z0 << " (default: 170.0)");
  ATH_MSG_DEBUG( "edge_mask_min_eta: " << cfg.edge_mask_min_eta << " (default: 1.5)");
  ATH_MSG_DEBUG( "hit_share_threshold: " << cfg.hit_share_threshold << " (default: 0.49)");
  ATH_MSG_DEBUG( "max_endcap_clusterwidth: " << cfg.max_endcap_clusterwidth << " (default: 0.35)");

}

} // namespace ActsTrk
