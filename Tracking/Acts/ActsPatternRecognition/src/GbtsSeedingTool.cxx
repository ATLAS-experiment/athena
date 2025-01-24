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
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK( detStore()->retrieve(m_pixelId, "PixelID") );
    ATH_CHECK( detStore()->retrieve(m_pixelManager, "ITkPixel") );    

    ATH_CHECK( m_pixelDetEleCollKey.initialize() );

    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG(" *  Used by SeedFinderGbtsConfig");

    // Make the logger And Propagate to ACTS routines
    m_logger = makeActsAthenaLogger(this, "Acts");

    ATH_CHECK( prepareConfiguration() );

     // input trig vector
    m_finderCfg.m_layerGeometry = LayerNumbering();

    std::ifstream input_ifstream(
         m_finderCfg.ConnectorInputFile.c_str(), std::ifstream::in); //change to connector input file 
    // connector
    std::unique_ptr<Acts::GbtsConnector> inputConnector =  
        std::make_unique<Acts::GbtsConnector>(input_ifstream);
        
    m_gbtsGeo = std::make_unique<Acts::GbtsGeometry<xAOD::SpacePoint>>( 
      m_finderCfg.m_layerGeometry, inputConnector);

    return StatusCode::SUCCESS;
  }

  ATH_FLATTEN
  StatusCode
  GbtsSeedingTool::createSeeds(const EventContext& ctx,
			       const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
			       const Acts::Vector3& beamSpotPos,
			       const Acts::Vector3& bField,
			       ActsTrk::SeedContainer& seedContainer ) const
  {
    // Seed Finder Options
    Acts::SeedFinderOptions finderOpts;
    finderOpts.beamPos = Acts::Vector2(beamSpotPos[Amg::x],
                                       beamSpotPos[Amg::y]);
    finderOpts.bFieldInZ = bField[2];
    finderOpts = finderOpts.toInternalUnits().calculateDerivedQuantities(m_finderCfg);
    
    // // // Compute seeds
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle = SG::makeHandle(m_pixelDetEleCollKey, ctx);
    ATH_CHECK(pixelDetEleHandle.isValid()) ;
    const InDetDD::SiDetectorElementCollection* pixelElements = pixelDetEleHandle.cptr();
  

    std::vector<Acts::GbtsSP<xAOD::SpacePoint>> GbtsSpacePoints;
    GbtsSpacePoints.reserve(
        spContainer.size()); 

    // for loop filling space
    for (const auto& spacePoint : spContainer) { //xaod space points
        // loop over space points, get necessary info from athena: 
      const std::vector<xAOD::DetectorIDHashType>& elementlist = spacePoint.externalSpacePoint().elementIdList() ;

        for (const xAOD::DetectorIDHashType element : elementlist) { 

          const InDetDD::SiDetectorElement* pixelElement = pixelElements->getDetectorElement(element);

          Identifier Identifier = pixelElement->identify() ; 

          int eta_mod = m_pixelId->eta_module(Identifier); 
          short barrel_ec = m_pixelId->barrel_ec(Identifier); 
          int lay_id = m_pixelId->layer_disk(Identifier);
          int combined_id = getCombinedID(eta_mod,barrel_ec,lay_id).first ; 
          int Gbts_id = getCombinedID(eta_mod,barrel_ec,lay_id).second ; 
        
          // fill Gbts vector with current sapce point and ID
          float ClusterWidth = 0; //for now trying to fill this  
        
          // fill Gbts vector with current sapce point and ID
          GbtsSpacePoints.emplace_back(&spacePoint.externalSpacePoint(), Gbts_id, combined_id, ClusterWidth); 
          //constructor takes (const space_point_t *sp, 

        }
      
    }

    ATH_MSG_VERBOSE("Space points successfully assigned Gbts ID");

    Acts::SeedFinderGbts<xAOD::SpacePoint> finder = Acts::SeedFinderGbts<xAOD::SpacePoint>(m_finderCfg,*m_gbtsGeo);  

    finder.loadSpacePoints(GbtsSpacePoints);
    //temporary solution until trigger ROIs implemented 
    Acts::RoiDescriptor internalRoi(0, -4.5, 4.5, 0, -std::numbers::pi, std::numbers::pi, 0, -150.0,150.0); //(eta,etaMinus,etaPlus,phi,phiMinus,Phiplus,z,zMinus,zPlus)

    std::vector<Acts::Seed<xAOD::SpacePoint, 3ul>> groupSeeds = finder.createSeeds(internalRoi, *m_gbtsGeo);

    // Store seeds

    seedContainer.reserve(groupSeeds.size());
    for( Acts::Seed<xAOD::SpacePoint, 3ul>& seed: groupSeeds) {
      //turn interim into group seeds 

      const auto& spacepoints = seed.sp() ; 
      assert(spacepoints.size()==3) ;  
      const xAOD::SpacePoint* sp1 = spacepoints[0] ; 
      const xAOD::SpacePoint* sp2 = spacepoints[1] ; 
      const xAOD::SpacePoint* sp3 = spacepoints[2] ; 

      std::unique_ptr<seed_type> to_add = std::make_unique<seed_type>(*sp1, *sp2, *sp3);
      to_add->setVertexZ(seed.z());
      to_add->setQuality(seed.seedQuality());
      seedContainer.push_back(std::move(to_add));  

    }

    return StatusCode::SUCCESS;
  }

  // own class functions
  std::vector<Acts::TrigInDetSiLayer>
  GbtsSeedingTool::LayerNumbering() const {
    std::vector<std::size_t> count_vector;
    std::vector<Acts::TrigInDetSiLayer> input_vector;

    for(int hash = 0; hash<static_cast<int>(m_pixelId->wafer_hash_max()); hash++) {
      const Identifier offlineId = m_pixelId->wafer_id(hash); 
      const int eta_mod = m_pixelId->eta_module(offlineId); 
      const short barrel_ec = m_pixelId->barrel_ec(offlineId); 
      const int lay_id = m_pixelId->layer_disk(offlineId);     

      const int combined_id = getCombinedID(eta_mod,barrel_ec,lay_id).first ; 

      float rc = 0.0;
      float minBound = std::numeric_limits<float>::max(); 
      float maxBound = -std::numeric_limits<float>::max(); 

      //want center and bounds! 
      const InDetDD::SiDetectorElement *p = m_pixelManager->getDetectorElement(offlineId);
      const Amg::Vector3D C = p->center() ;

      if(barrel_ec == 0) {
        rc += std::sqrt(C(0)*C(0)+C(1)*C(1));
        if(p->zMin() < minBound) minBound = p->zMin();
        if(p->zMax() > maxBound) maxBound = p->zMax();
      }
      else {
        rc += C(2);
        if(p->rMin() < minBound) minBound = p->rMin();
        if(p->rMax() > maxBound) maxBound = p->rMax();	
      }

    
      auto current_index =
          find_if(input_vector.begin(), input_vector.end(),
                  [combined_id](auto n) { return n.m_subdet == combined_id; });
      if (current_index != input_vector.end()) {  // not end so does exist
        std::size_t index = std::distance(input_vector.begin(), current_index);
        input_vector[index].m_refCoord += rc;
        input_vector[index].m_minBound += minBound;
        input_vector[index].m_maxBound += maxBound;
        count_vector[index] += 1;  // increase count at the index

      } else {  // end so doesn't exists
        // make new if one with Gbts ID doesn't exist:
        Acts::TrigInDetSiLayer new_Gbts_ID(combined_id, barrel_ec, rc, minBound,
                                          maxBound);
        input_vector.push_back(new_Gbts_ID);
        count_vector.push_back(
            1);  // so the element exists and not divinding by 0
      }

    }
    for (std::size_t i = 0; i < input_vector.size(); ++i) {
      assert(count_vector[i] != 0);
      input_vector[i].m_refCoord = input_vector[i].m_refCoord / count_vector[i];
    }

    return input_vector;
  }
  
  //this is called in initialise 
  StatusCode
  GbtsSeedingTool::prepareConfiguration()
  {
    // Configuration Acts::SeedFinderGbts

    m_finderCfg.minPt = m_minPt;
    m_finderCfg.sigmaScattering = m_sigmaScattering;
    m_finderCfg.highland = m_highland;
    m_finderCfg.maxScatteringAngle2 = m_maxScatteringAngle2;
    m_finderCfg.helixCutTolerance = m_helixCutTolerance ;
    m_finderCfg.m_phiSliceWidth = m_phiSliceWidth ;
    m_finderCfg.m_nMaxPhiSlice = m_nMaxPhiSlice;
    m_finderCfg.m_useClusterWidth = m_useClusterWidth;
    m_finderCfg.ConnectorInputFile = m_ConnectorInputFile;
    m_finderCfg.m_useEtaBinning = m_useEtaBinning;
    m_finderCfg.m_doubletFilterRZ = m_doubletFilterRZ ;
    m_finderCfg.m_minDeltaRadius = m_minDeltaRadius;
    m_finderCfg.m_tripletD0Max = m_tripletD0Max;
    m_finderCfg.m_maxTripletBufferLength = m_maxTripletBufferLength;
    m_finderCfg.MaxEdges = m_MaxEdges;
    m_finderCfg.cut_dphi_max = m_cut_dphi_max;
    m_finderCfg.cut_dcurv_max = m_cut_dcurv_max;
    m_finderCfg.cut_tau_ratio_max = m_cut_tau_ratio_max;
    m_finderCfg.maxOuterRadius = m_maxOuterRadius;
    m_finderCfg.m_PtMin = m_PtMin;
    m_finderCfg.m_tripletPtMinFrac = m_tripletPtMinFrac;
    m_finderCfg.m_tripletPtMin = m_tripletPtMin;
    m_finderCfg.ptCoeff = m_ptCoeff;

    m_finderCfg = m_finderCfg.toInternalUnits();


    return StatusCode::SUCCESS;
  }

  std::pair<int,int> GbtsSeedingTool::getCombinedID(const int eta_mod, const short barrel_ec, const int lay_id) const { 
    int vol_id = -1 ;         
    if(barrel_ec== 0) vol_id = 8;
    if(barrel_ec==-2) vol_id = 7;
    if(barrel_ec== 2) vol_id = 9;
        
    int new_vol=0, new_lay=0;
    if(vol_id == 7 || vol_id == 9) {
      new_vol = 10*vol_id + lay_id;
      new_lay = eta_mod;
    }
    else if(vol_id == 8) {
      new_lay = 0;
      new_vol = 10*vol_id + lay_id;
    }
    //make into the form needed for acts 
    int Gbts_id = new_vol ; 
    int combined_id = new_vol * 1000 + new_lay;

    return std::make_pair(combined_id,Gbts_id) ; 
  }

} // namespace ActsTrk
