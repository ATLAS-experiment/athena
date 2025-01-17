/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include "PathResolver/PathResolver.h"

#include "GNN_TrackingFilter.h"

#include "IRegionSelector/IRegSelTool.h"

#include "TrigInDetTrackSeedingTool.h"

TrigInDetTrackSeedingTool::TrigInDetTrackSeedingTool(const std::string& t, 
					     const std::string& n,
					     const IInterface*  p ) : 
  SeedingToolBase(t,n,p)
{

}

StatusCode TrigInDetTrackSeedingTool::initialize() {
  ATH_CHECK(SeedingToolBase<const Trk::SpacePoint*>::initialize());
  
  ATH_CHECK(m_regsel_pix.retrieve());
  ATH_CHECK(m_regsel_sct.retrieve());

  ATH_CHECK(m_beamSpotKey.initialize());

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
  return SeedingToolBase<const Trk::SpacePoint*>::finalize();
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
  
  std::vector<std::vector<GNN_Node> > trigSpStorage[2];
  
  if (m_useSctSpacePoints) {

    SG::ReadHandle<SpacePointContainer> sctHandle(m_sctSpacePointsContainerKey, ctx);

    if(!sctHandle.isValid()) {
      ATH_MSG_WARNING("Invalid Strip Spacepoints handle: "<<sctHandle);
      return seedStats;
      
    }
    const SpacePointContainer* sctSpacePointsContainer = sctHandle.ptr();
    
    std::vector<IdentifierHash> listOfSctIds;

    m_regsel_sct->lookup(ctx)->HashIDList( internalRoI, listOfSctIds );

    const std::vector<short>* h2l = m_layerNumberTool->sctLayers();

    trigSpStorage[1].resize(h2l->size());
 
    for(const auto& idx : listOfSctIds) {

      short layerIndex = h2l->at(static_cast<int>(idx));

      std::vector<GNN_Node>& tmpColl = trigSpStorage[1].at(static_cast<int>(idx));
    
      auto input_coll = sctSpacePointsContainer->indexFindPtr(idx);

      if(input_coll == nullptr) continue;

      createGraphNodes(input_coll, tmpColl, layerIndex, shift_x, shift_y);//TO-DO: if(m_useBeamTilt) SP full transform functor

      nStrips += storage->loadStripGraphNodes(layerIndex, tmpColl);
    }
  }

  if (m_usePixelSpacePoints) {
    
    const SpacePointContainer* pixelSpacePointsContainer = nullptr;
    SG::ReadHandle<SpacePointContainer> pixHandle(m_pixelSpacePointsContainerKey, ctx);

    if(!pixHandle.isValid()) {
      ATH_MSG_WARNING("Invalid Pixel Spacepoints handle: "<<pixHandle);
      return seedStats;
    }
    
    pixelSpacePointsContainer = pixHandle.ptr();
    
    std::vector<IdentifierHash> listOfPixIds;

    m_regsel_pix->lookup(ctx)->HashIDList( internalRoI, listOfPixIds );

    const std::vector<short>* h2l = m_layerNumberTool->pixelLayers();

    trigSpStorage[0].resize(h2l->size());
    
    for(const auto& idx : listOfPixIds) {

      short layerIndex = h2l->at(static_cast<int>(idx));

      std::vector<GNN_Node>& tmpColl = trigSpStorage[0].at(static_cast<int>(idx));
    
      auto input_coll = pixelSpacePointsContainer->indexFindPtr(idx);

      if(input_coll == nullptr) continue;

      createGraphNodes(input_coll, tmpColl, layerIndex, shift_x, shift_y);//TO-DO: SP full transform functor

      nPixels += storage->loadPixelGraphNodes(layerIndex, tmpColl, m_useML);
    }

  }
   
  storage->sortByPhi();

  storage->initializeNodes(m_useML);

  storage->generatePhiIndexing(1.5*m_phiSliceWidth);

  seedStats.m_nPixelSPs = nPixels;
  seedStats.m_nStripSPs = nStrips;
  
  ATH_MSG_DEBUG("Loaded "<<nPixels<< " Pixel Spacepoints and "<<nStrips<< " Strip SpacePoints");

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

  TrigFTF_GNN_TrackingFilter<const Trk::SpacePoint*> tFilter(m_layerGeometry, edgeStorage);

  output.reserve(vSeeds.size());
  
  for(auto pS : vSeeds) {

    if(pS->m_level == -1) continue;

    TrigFTF_GNN_EdgeState<const Trk::SpacePoint*> rs(false);
    
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
      output[lastIdx].addSpacePoint(n->m_pSP);
    }
  }

  ATH_MSG_DEBUG("Found "<<output.size()<<" tracklets");
  
  return seedStats;
}

void TrigInDetTrackSeedingTool::createGraphNodes(const SpacePointCollection* spColl, std::vector<GNN_Node>& tmpColl, unsigned short layer, float shift_x, float shift_y) const {
  tmpColl.resize(spColl->size(), GNN_Node(layer));//all nodes belong to the same layer
  
  int idx = 0;
  for(const auto sp : *spColl) {
    const auto& pos = sp->globalPosition();
    float xs = pos.x() - shift_x;
    float ys = pos.y() - shift_y;
    float zs = pos.z();
    tmpColl[idx].m_x = xs;
    tmpColl[idx].m_y = ys;
    tmpColl[idx].m_z = zs;
    tmpColl[idx].m_r = std::sqrt(xs*xs + ys*ys);
    tmpColl[idx].m_phi = std::atan2(ys,xs);
    tmpColl[idx].m_pSP = sp;

    const InDet::PixelCluster* pCL = dynamic_cast<const InDet::PixelCluster*>(sp->clusterList().first);
    if(pCL != nullptr){
      tmpColl[idx].m_pcw = pCL->width().widthPhiRZ().y();
    }

    idx++;
  }
}

