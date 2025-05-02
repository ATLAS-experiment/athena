/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include "PathResolver/PathResolver.h"

#include "IRegionSelector/IRegSelTool.h"

#include "TrigInDetTrackSeedingTool.h"

#include "GNN_TrackingFilter.h"

//for GPU offloading

#include "TrigAccelEvent/TrigITkAccelEDM.h"
#include "TrigAccelEvent/TrigInDetAccelCodes.h"

TrigInDetTrackSeedingTool::TrigInDetTrackSeedingTool(const std::string& t, 
					     const std::string& n,
					     const IInterface*  p ) : SeedingToolBase(t,n,p)
{

}

StatusCode TrigInDetTrackSeedingTool::initialize() {

  ATH_CHECK(SeedingToolBase::initialize());
  
  ATH_CHECK(m_regsel_pix.retrieve());
  ATH_CHECK(m_regsel_sct.retrieve());

  ATH_CHECK(m_beamSpotKey.initialize());

  if(m_useGPU) {//for GPU offloading
    ATH_CHECK(m_accelSvc.retrieve());
    ATH_CHECK(m_accelSvc->isReady());
  }
 
  if (!m_usePixelSpacePoints && !m_useSctSpacePoints) {
    ATH_MSG_FATAL("Both usePixelSpacePoints and useSctSpacePoints set to False. At least one needs to be True");
    return StatusCode::FAILURE;
  }

  if (!m_useSctSpacePoints) ATH_MSG_INFO("Only using Pixel spacepoints => Pixel seeds only");

  if (!m_usePixelSpacePoints) ATH_MSG_INFO("Only using SCT spacepoints => Strip seeds only");

  if (m_usePixelSpacePoints && m_useSctSpacePoints) ATH_MSG_INFO("Using SCT and Pixel spacepoints");

  ATH_CHECK(m_pixelSpacePointsContainerKey.initialize(m_usePixelSpacePoints));

  ATH_CHECK(m_sctSpacePointsContainerKey.initialize(m_useSctSpacePoints));
  
  ATH_MSG_INFO("TrigInDetTrackSeedingTool initialized ");
  
  return StatusCode::SUCCESS;
}

StatusCode TrigInDetTrackSeedingTool::finalize() {
  return SeedingToolBase::finalize();
}


TrigInDetTrackSeedingResult TrigInDetTrackSeedingTool::findSeeds(const IRoiDescriptor& internalRoI, std::vector<TrigInDetTracklet>& output, const EventContext& ctx) const {

  TrigInDetTrackSeedingResult seedStats;
  
  output.clear();

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };  
  const Amg::Vector3D &vertex = beamSpotHandle->beamPos();
  float shift_x = vertex.x() - beamSpotHandle->beamTilt(0)*vertex.z();
  float shift_y = vertex.y() - beamSpotHandle->beamTilt(1)*vertex.z();

  std::unique_ptr<GNN_DataStorage> storage = std::make_unique<GNN_DataStorage>(*m_geo);
  
  int nPixels = 0;
  int nStrips = 0;

  const SpacePointContainer* sctSpacePointsContainer = nullptr;
  const SpacePointContainer* pixelSpacePointsContainer = nullptr;

  std::map<short, std::vector<IdentifierHash> > detIdMap;//relates global detector layer ID to constituent detector elements

  if (m_useSctSpacePoints) {
    
    SG::ReadHandle<SpacePointContainer> sctHandle(m_sctSpacePointsContainerKey, ctx);

    if(!sctHandle.isValid()) {
      ATH_MSG_WARNING("Invalid Strip Spacepoints handle: "<<sctHandle);
      return seedStats;
      
    }
    
    sctSpacePointsContainer = sctHandle.ptr();

    std::vector<IdentifierHash> listOfSctIds;
    m_regsel_sct->lookup(ctx)->HashIDList( internalRoI, listOfSctIds );

    const std::vector<short>* h2l = m_layerNumberTool->sctLayers();
    
    for(const auto& hashId : listOfSctIds) {
      
      short layerIndex = h2l->at(static_cast<int>(hashId));

      auto it = detIdMap.find(layerIndex);
      if(it != detIdMap.end()) (*it).second.push_back(hashId);
      else {
        std::vector<IdentifierHash> v = {hashId};
        detIdMap.insert(std::make_pair(layerIndex,v));
      }
    }
  }

  if (m_usePixelSpacePoints) {
    
    SG::ReadHandle<SpacePointContainer> pixHandle(m_pixelSpacePointsContainerKey, ctx);

    if(!pixHandle.isValid()) {
      ATH_MSG_WARNING("Invalid Pixel Spacepoints handle: "<<pixHandle);
      return seedStats;
    }
    
    pixelSpacePointsContainer = pixHandle.ptr();
    
    std::vector<IdentifierHash> listOfPixIds;
    
    m_regsel_pix->lookup(ctx)->HashIDList( internalRoI, listOfPixIds );

    const std::vector<short>* h2l = m_layerNumberTool->pixelLayers();
 
    for(const auto& hashId : listOfPixIds) {
      
      short layerIndex = h2l->at(static_cast<int>(hashId));

      auto it = detIdMap.find(layerIndex);
      if(it != detIdMap.end()) (*it).second.push_back(hashId);
      else {
        std::vector<IdentifierHash> v = {hashId};
        detIdMap.insert(std::make_pair(layerIndex,v));
      }
    }
  }

  if(!m_useGPU) {

    std::unique_ptr<GNN_DataStorage> storage = std::make_unique<GNN_DataStorage>(*m_geo);

    std::vector<const Trk::SpacePoint*> vSP;

    vSP.reserve(m_nMaxEdges);
    
    std::vector<std::vector<GNN_Node> > trigSpStorage[2];
    
    trigSpStorage[1].resize(m_layerNumberTool->sctLayers()->size());

    trigSpStorage[0].resize(m_layerNumberTool->pixelLayers()->size());

    for(const auto& lColl : detIdMap) {

      short layerIndex = lColl.first;

      int layerKey = m_geo->getTrigFTF_GNN_LayerByIndex(layerIndex)->m_layer.m_subdet;
      
      bool isPixel = layerKey > 20000;

      auto pCont = isPixel ? pixelSpacePointsContainer : sctSpacePointsContainer;

      int contIdx= isPixel ? 0 : 1;

      int nNewNodes = 0;
      
      for(const auto& idx : lColl.second) {
      
        std::vector<GNN_Node>& tmpColl = trigSpStorage[contIdx].at(static_cast<int>(idx));

        auto input_coll = pCont->indexFindPtr(idx);

        if(input_coll == nullptr) continue;

        createGraphNodes(input_coll, tmpColl, vSP, layerIndex, shift_x, shift_y);//TO-DO: if(m_useBeamTilt) SP full transform functor

        nNewNodes += (isPixel) ? storage->loadPixelGraphNodes(layerIndex, tmpColl, m_useML) : storage->loadStripGraphNodes(layerIndex, tmpColl);
      }

      if(isPixel) nPixels += nNewNodes;
      else nStrips += nNewNodes;      
    }

    seedStats.m_nPixelSPs = nPixels;
    seedStats.m_nStripSPs = nStrips;
  
    storage->sortByPhi();
    storage->initializeNodes(m_useML);
    storage->generatePhiIndexing(1.5*m_phiSliceWidth);

    std::vector<GNN_Edge> edgeStorage;

    std::pair<int, int> graphStats = buildTheGraph(internalRoI, storage, edgeStorage);

    ATH_MSG_DEBUG("Created graph with "<<graphStats.first<<" edges and "<<graphStats.second<< " edge links");

    seedStats.m_nGraphEdges = graphStats.first;
    seedStats.m_nEdgeLinks  = graphStats.second;
  
    if(graphStats.second == 0) return seedStats;
    
    int maxLevel = runCCA(graphStats.first, edgeStorage);

    ATH_MSG_DEBUG("Reached Level "<<maxLevel<<" after GNN iterations");

    int minLevel = 3;//a triplet + 2 confirmation

    if(m_LRTmode) {
      minLevel = 2;//a triplet + 1 confirmation
    }
  
    if(maxLevel < minLevel) return seedStats;
  
    std::vector<GNN_Edge*> vSeeds;

    vSeeds.reserve(graphStats.first/2);

    for(int edgeIndex=0;edgeIndex<graphStats.first;edgeIndex++) {
      GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

      if(pS->m_level < minLevel) continue;
      
      vSeeds.push_back(pS);
    }
  
    if(vSeeds.empty()) return seedStats;
 
    std::sort(vSeeds.begin(), vSeeds.end(), GNN_Edge::CompareLevel());

    //backtracking

    TrigFTF_GNN_TrackingFilter tFilter(m_layerGeometry, edgeStorage);

    output.reserve(vSeeds.size());
  
    for(auto pS : vSeeds) {
      
      if(pS->m_level == -1) continue;
      
      TrigFTF_GNN_EdgeState rs(false);
      
      tFilter.followTrack(pS, rs);
      
      if(!rs.m_initialized) {
	continue;
      }
      
      if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;
      
      std::vector<const GNN_Node*> vN;
      
      for(std::vector<GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {
        
	(*sIt)->m_level = -1;//mark as collected
	
	if(sIt == rs.m_vs.rbegin()) {
	  vN.push_back((*sIt)->m_n1);
	}
	vN.push_back((*sIt)->m_n2);
      }
      
      if(vN.size()<3) continue;
      
      unsigned int lastIdx = output.size();
      output.emplace_back(rs.m_J);
      
      for(const auto& n : vN) {
	output[lastIdx].addSpacePoint(vSP[n->m_idx]);
      }
    }
  
    ATH_MSG_DEBUG("Found "<<output.size()<<" tracklets");  
  }
  else {//GPU-accelerated graph building

    //1. data export

    std::vector<const Trk::SpacePoint*> vSP;
    std::vector<short> vL;

    vSP.reserve(TrigAccel::ITk::GBTS_MAX_NUMBER_SPACEPOINTS);
    vL.reserve(TrigAccel::ITk::GBTS_MAX_NUMBER_SPACEPOINTS);

    TrigAccel::DATA_EXPORT_BUFFER* dataBuffer = new TrigAccel::DATA_EXPORT_BUFFER(5000);
    
    size_t dataTypeSize = sizeof(TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA);
    const size_t bufferOffset = 256;
    size_t totalSize = bufferOffset+dataTypeSize;//make room for the header
    if(!dataBuffer->fit(totalSize)) dataBuffer->reallocate(totalSize);

    TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA* pJobData = reinterpret_cast<TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA*>(dataBuffer->m_buffer + bufferOffset);

    unsigned int spIdx = 0;
    int sp_offset = 0;
    int nLayers = 0;
    int MaxEtaBin = 0;
    int nEtaBins  = 0;
    
    for(const auto& lColl : detIdMap) {

      short layerIndex = lColl.first;
      
      const TrigFTF_GNN_Layer* pL = m_geo->getTrigFTF_GNN_LayerByIndex(layerIndex);

      int layerKey = pL->m_layer.m_subdet;
      
      bool isPixel = layerKey > 20000;
      bool isBarrel = (pL->m_layer.m_type == 0);
      
      auto pCont = isPixel ? pixelSpacePointsContainer : sctSpacePointsContainer;
      
      pJobData->m_layerIdx[nLayers]        = layerIndex;

      pJobData->m_layerInfo[4*nLayers    ]   = spIdx;//layerInfo.x
      pJobData->m_layerInfo[4*nLayers + 2]   = pL->num_bins();//layerInfo.z
      pJobData->m_layerInfo[4*nLayers + 3]   = pL->m_bins[0];//layerInfo.w
      
      pJobData->m_layerGeo[2*nLayers    ]    = pL->m_minEta;//layerGeo.x
      pJobData->m_layerGeo[2*nLayers + 1]    = pL->m_etaBin;//layerGeo.y
      
      nEtaBins += pL->m_bins.size();
      
      for(auto b : pL->m_bins) {
        if(b > MaxEtaBin) MaxEtaBin = b;
      }

      for(const auto& idx : lColl.second) {
   
        auto input_coll = pCont->indexFindPtr(idx);

        if(input_coll == nullptr) continue;

        for(const auto sp : *input_coll) {

          float cw = -1.0;
          
          const InDet::PixelCluster* pCL = dynamic_cast<const InDet::PixelCluster*>(sp->clusterList().first);
          if(pCL != nullptr){
            cw = pCL->width().widthPhiRZ().y();
            if(!isBarrel && m_useML) {
              if(cw > 0.2) continue;
              cw = -1.0;//set it to -1 so that it can be skipped later in the ML code on GPU
            }
          }
          
          vSP.emplace_back(sp);
	  vL.emplace_back(layerIndex);
	  
          const auto& p = sp->globalPosition();

          float params[4] = {(float)(p.x() - shift_x), float(p.y() - shift_y), (float)(p.z()), cw};

          memcpy(&pJobData->m_params[sp_offset], &params[0], sizeof(params));
          
          sp_offset += 4;
          spIdx++;
          if(spIdx >= TrigAccel::ITk::GBTS_MAX_NUMBER_SPACEPOINTS) break;
        }       
        if(spIdx >= TrigAccel::ITk::GBTS_MAX_NUMBER_SPACEPOINTS) break;
      }

      pJobData->m_layerInfo[4*nLayers + 1] = spIdx;//layerInfo.y

      if (isPixel) nPixels += spIdx - pJobData->m_layerInfo[4*nLayers];
      else nStrips += spIdx - pJobData->m_layerInfo[4*nLayers];
      
      nLayers++;
    }
  
    pJobData->m_nSpacepoints = spIdx;
    pJobData->m_nLayers      = nLayers;
    pJobData->m_nEtaBins     = nEtaBins;
    pJobData->m_maxEtaBin    = MaxEtaBin;
    pJobData->m_nMaxEdges    = m_nMaxEdges;
    
    //load bin pairs

    int pairIdx = 0;
    
    for(const auto& bg : m_geo->bin_groups()) {//loop over bin groups
    
      int bin1_idx = bg.first;
        
      for(const auto& bin2_idx : bg.second) {
        pJobData->m_bin_pairs[2*pairIdx  ] = bin1_idx;
        pJobData->m_bin_pairs[2*pairIdx+1] = bin2_idx;
        pairIdx++;
      }
    }

    pJobData->m_nBinPairs = pairIdx;
    
    //add algorithm parameters
    
    const float ptCoeff = 0.29997*1.9972/2.0;// ~0.3*B/2 - assuming nominal field of 2*T

    float tripletPtMin = 0.8*m_minPt;//correction due to limited pT resolution
  
    float maxCurv = ptCoeff/tripletPtMin;
  
    const float min_deltaPhi      = 0.001;
    const float dphi_coeff        = 0.68*maxCurv;
    const float cut_dphi_max      = m_LRTmode ? 0.07 : 0.012;
    const float cut_dcurv_max     = m_LRTmode ? 0.015 : 0.001;
    const float cut_tau_ratio_max = m_LRTmode ? 0.015 : 0.007;
    const float min_z0            = m_LRTmode ? -600.0 : internalRoI.zedMinus();
    const float max_z0            = m_LRTmode ? 600.0 : internalRoI.zedPlus();
  
    const float maxOuterRadius    = m_LRTmode ? 1050.0 : 550.0;  
    const float minDeltaRadius    = 2.0;
        
    const float cut_zMinU = min_z0 + maxOuterRadius*internalRoI.dzdrMinus();
    const float cut_zMaxU = max_z0 + maxOuterRadius*internalRoI.dzdrPlus();
  
    const float maxKappa_high_eta          = m_LRTmode ? 1.0*maxCurv : std::sqrt(0.8)*maxCurv;
    const float maxKappa_low_eta           = m_LRTmode ? 1.0*maxCurv : std::sqrt(0.6)*maxCurv;

    pJobData->m_algo_params[0] = min_deltaPhi;
    pJobData->m_algo_params[1] = dphi_coeff;
    pJobData->m_algo_params[2] = minDeltaRadius;
    pJobData->m_algo_params[3] = min_z0;
    pJobData->m_algo_params[4] = max_z0;
    pJobData->m_algo_params[5] = maxOuterRadius;
    pJobData->m_algo_params[6] = cut_zMinU;
    pJobData->m_algo_params[7] = cut_zMaxU;
    pJobData->m_algo_params[8] = maxKappa_low_eta;
    pJobData->m_algo_params[9] = maxKappa_high_eta;
    pJobData->m_algo_params[10]= cut_dphi_max;
    pJobData->m_algo_params[11]= cut_dcurv_max;
    pJobData->m_algo_params[12]= cut_tau_ratio_max;

    seedStats.m_nPixelSPs = nPixels;
    seedStats.m_nStripSPs = nStrips;

    ATH_MSG_DEBUG("Loaded "<<nPixels<< " Pixel Spacepoints and "<<nStrips<< " Strip SpacePoints");
    
    std::shared_ptr<TrigAccel::OffloadBuffer> pBuff = std::make_shared<TrigAccel::OffloadBuffer>(dataBuffer);
    
    TrigAccel::Work* pWork = m_accelSvc->createWork(TrigAccel::InDetJobControlCode::RUN_GBTS, pBuff);
    
    if(!pWork) {
      ATH_MSG_INFO("Failed to create a work item for task "<<TrigAccel::InDetJobControlCode::RUN_GBTS);
      return seedStats;
    }

    ATH_MSG_DEBUG("Work item created for task "<<TrigAccel::InDetJobControlCode::RUN_GBTS);

    pWork->run();
    
    std::shared_ptr<TrigAccel::OffloadBuffer> pOutput = pWork->getOutput();

    TrigAccel::ITk::COMPRESSED_GRAPH* pGraph = reinterpret_cast<TrigAccel::ITk::COMPRESSED_GRAPH*>(pOutput->m_rawBuffer);

    unsigned int nEdges = pGraph->m_nEdges;
    unsigned int nMaxNei = pGraph->m_nMaxNeighbours;

    //populating the edgeStorage

    std::vector<GNN_Node> nodes;

    nodes.reserve(vSP.size());

    for(unsigned int idx = 0;idx < vSP.size(); idx++) {

      nodes.emplace_back(vL[idx]);
      
      const auto& pos = vSP[idx]->globalPosition();
      float xs = pos.x() - shift_x;
      float ys = pos.y() - shift_y;
      float zs = pos.z();
      
      nodes[idx].m_x = xs;
      nodes[idx].m_y = ys;
      nodes[idx].m_z = zs;
      nodes[idx].m_r = std::sqrt(xs*xs + ys*ys);

      nodes[idx].m_idx = idx;

    }
    
    std::vector<GNN_Edge> edgeStorage;

    std::pair<int, int> graphStats(0,0);

    edgeStorage.resize(nEdges);

    unsigned int edgeSize = nMaxNei + 1 + 2;//neigbours, num_neighbours, 2 nodes

    for(unsigned int idx=0;idx<nEdges;idx++) {
      unsigned int pos = idx*edgeSize;
      
      int node1Idx = pGraph->m_graphArray[pos];
      int node2Idx = pGraph->m_graphArray[pos+1];
      int nNei     = pGraph->m_graphArray[pos+2];

      if(nNei > N_SEG_CONNS) nNei = N_SEG_CONNS;
      
      edgeStorage[idx].m_n1 = &nodes[node1Idx];
      edgeStorage[idx].m_n2 = &nodes[node2Idx];
      edgeStorage[idx].m_level = 1;
      edgeStorage[idx].m_nNei = nNei;
      for(int k=0;k<nNei;k++) {
        edgeStorage[idx].m_vNei[k] = pGraph->m_graphArray[pos+3+k];
      }
    }
    
    graphStats.first = nEdges;
    graphStats.second = pGraph->m_nLinks;
    
    delete[] pGraph->m_graphArray;

    pGraph->m_graphArray = nullptr;

    delete pWork;
    
    delete dataBuffer;

    //run the rest of the GBTS workflow

    int maxLevel = runCCA(graphStats.first, edgeStorage);

    ATH_MSG_DEBUG("Reached Level "<<maxLevel<<" after GNN iterations");

    int minLevel = 3;//a triplet + 2 confirmation

    if(m_LRTmode) {
      minLevel = 2;//a triplet + 1 confirmation
    }
  
    if(maxLevel < minLevel) return seedStats;
  
    std::vector<GNN_Edge*> vSeeds;

    vSeeds.reserve(graphStats.first/2);

    for(int edgeIndex=0;edgeIndex<graphStats.first;edgeIndex++) {
      GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

      if(pS->m_level < minLevel) continue;
      
      vSeeds.push_back(pS);
    }
  
    if(vSeeds.empty()) return seedStats;
 
    std::sort(vSeeds.begin(), vSeeds.end(), GNN_Edge::CompareLevel());

    //backtracking

    TrigFTF_GNN_TrackingFilter tFilter(m_layerGeometry, edgeStorage);

    output.reserve(vSeeds.size());
  
    for(auto pS : vSeeds) {
      
      if(pS->m_level == -1) continue;
      
      TrigFTF_GNN_EdgeState rs(false);
      
      tFilter.followTrack(pS, rs);
      
      if(!rs.m_initialized) {
	continue;
      }
      
      if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;
      
      std::vector<const GNN_Node*> vN;
      
      for(std::vector<GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {
        
	(*sIt)->m_level = -1;//mark as collected
	
	if(sIt == rs.m_vs.rbegin()) {
	  vN.push_back((*sIt)->m_n1);
	}
	vN.push_back((*sIt)->m_n2);
      }
      
      if(vN.size()<3) continue;
      
      unsigned int lastIdx = output.size();
      output.emplace_back(rs.m_J);
      
      for(const auto& n : vN) {
	output[lastIdx].addSpacePoint(vSP[n->m_idx]);
      }
    }
  
    ATH_MSG_DEBUG("Found "<<output.size()<<" tracklets");
  }
  
  return seedStats;
  
}

void TrigInDetTrackSeedingTool::createGraphNodes(const SpacePointCollection* spColl, std::vector<GNN_Node>& tmpColl, std::vector<const Trk::SpacePoint*>& vSP, unsigned short layer, float shift_x, float shift_y) const {
  
  tmpColl.resize(spColl->size(), GNN_Node(layer));//all nodes belong to the same layer
  
  int idx = 0;
  int init_size = vSP.size();
  for(const auto sp : *spColl) {
    const auto& pos = sp->globalPosition();
    vSP.emplace_back(sp);
    float xs = pos.x() - shift_x;
    float ys = pos.y() - shift_y;
    float zs = pos.z();
    tmpColl[idx].m_x = xs;
    tmpColl[idx].m_y = ys;
    tmpColl[idx].m_z = zs;
    tmpColl[idx].m_r = std::sqrt(xs*xs + ys*ys);
    tmpColl[idx].m_phi = std::atan2(ys,xs);
    tmpColl[idx].m_idx = init_size + idx;

    const InDet::PixelCluster* pCL = dynamic_cast<const InDet::PixelCluster*>(sp->clusterList().first);
    if(pCL != nullptr){
      tmpColl[idx].m_pcw = pCL->width().z();
    }

    idx++;
  }
}

