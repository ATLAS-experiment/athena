/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  base_class(t,n,p)
{

}

StatusCode TrigInDetTrackSeedingTool::initialize() {

  StatusCode sc = AthAlgTool::initialize();
  
  ATH_CHECK(m_regsel_pix.retrieve());
  ATH_CHECK(m_regsel_sct.retrieve());

  sc=m_layerNumberTool.retrieve();

  if(sc.isFailure()) {
    ATH_MSG_ERROR("Could not retrieve "<<m_layerNumberTool);
    return sc;
  } else {
    ATH_MSG_DEBUG("Retrieved "<<m_layerNumberTool);
  }

  ATH_CHECK(detStore()->retrieve(m_atlasId, "AtlasID"));

  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));

  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));

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

  std::string conn_fileName = PathResolver::find_file(m_connectionFile, "DATAPATH");
  if (conn_fileName.empty()) {
    ATH_MSG_FATAL("Cannot find layer connections file " << conn_fileName);
    return StatusCode::FAILURE;
  }
  else {

    std::ifstream ifs(conn_fileName.c_str());
    
    m_connector = std::make_unique<GNN_FASTRACK_CONNECTOR>(ifs, m_LRTmode);
    
    ATH_MSG_INFO("Layer connections are initialized from file " << conn_fileName);
  }

  const std::vector<TrigInDetSiLayer>* pVL = m_layerNumberTool->layerGeometry();
  
  std::copy(pVL->begin(),pVL->end(), std::back_inserter(m_layerGeometry));

  m_geo = std::make_unique<TrigFTF_GNN_Geometry>(m_layerGeometry, m_connector);

  m_phiSliceWidth = 2*M_PI/m_nMaxPhiSlice;
  
  ATH_MSG_INFO("TrigInDetTrackSeedingTool initialized ");

  return sc;
}

StatusCode TrigInDetTrackSeedingTool::finalize() {
  
  StatusCode sc = AthAlgTool::finalize(); 
  return sc;
}


TrigInDetTrackSeedingResult TrigInDetTrackSeedingTool::findSeeds(const IRoiDescriptor& internalRoI, std::vector<TrigInDetTracklet>& output, const EventContext& ctx) const {

  TrigInDetTrackSeedingResult seedStats;
  
  output.clear();

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };  
  const Amg::Vector3D &vertex = beamSpotHandle->beamPos();
  float shift_x = vertex.x() - beamSpotHandle->beamTilt(0)*vertex.z();
  float shift_y = vertex.y() - beamSpotHandle->beamTilt(1)*vertex.z();

  std::unique_ptr<TrigFTF_GNN_DataStorage> storage = std::make_unique<TrigFTF_GNN_DataStorage>(*m_geo);
  
  int nPixels = 0;
  int nStrips = 0;
  
  std::vector<std::vector<TrigFTF_GNN_Node> > trigSpStorage[2];
  
  if (m_useSctSpacePoints) {

    SG::ReadHandle<SpacePointContainer> sctHandle(m_sctSpacePointsContainerKey, ctx);

    if(!sctHandle.isValid()) {
      ATH_MSG_WARNING("Invalid Strip Spacepoints handle: "<<sctHandle);
      return seedStats;
      
    }
    const SpacePointContainer* sctSpacePointsContainer = sctHandle.ptr();
    
    std::vector<IdentifierHash> listOfSctIds;

    m_regsel_sct->HashIDList( internalRoI, listOfSctIds );

    const std::vector<short>* h2l = m_layerNumberTool->sctLayers();

    trigSpStorage[1].resize(h2l->size());
 
    for(const auto& idx : listOfSctIds) {

      short layerIndex = h2l->at(static_cast<int>(idx));

      std::vector<TrigFTF_GNN_Node>& tmpColl = trigSpStorage[1].at(static_cast<int>(idx));
    
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

    m_regsel_pix->HashIDList( internalRoI, listOfPixIds );

    const std::vector<short>* h2l = m_layerNumberTool->pixelLayers();

    trigSpStorage[0].resize(h2l->size());
    
    for(const auto& idx : listOfPixIds) {

      short layerIndex = h2l->at(static_cast<int>(idx));

      std::vector<TrigFTF_GNN_Node>& tmpColl = trigSpStorage[0].at(static_cast<int>(idx));
    
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

  std::vector<TrigFTF_GNN_Edge> edgeStorage;

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
  
  std::vector<TrigFTF_GNN_Edge*> vSeeds;

  vSeeds.reserve(graphStats.first/2);

  for(int edgeIndex=0;edgeIndex<graphStats.first;edgeIndex++) {
    TrigFTF_GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

    if(pS->m_level < minLevel) continue;

    vSeeds.push_back(pS);
  }

  if(vSeeds.empty()) return seedStats;
 
  std::sort(vSeeds.begin(), vSeeds.end(), TrigFTF_GNN_Edge::CompareLevel());

  //backtracking

  TrigFTF_GNN_TRACKING_FILTER tFilter(m_layerGeometry, edgeStorage);

  output.reserve(vSeeds.size());
  
  for(auto pS : vSeeds) {

    if(pS->m_level == -1) continue;

    TrigFTF_GNN_EDGE_STATE rs(false);
    
    tFilter.followTrack(pS, rs);

    if(!rs.m_initialized) {
      continue;
    }
    
    if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;

    std::vector<const TrigFTF_GNN_Node*> vN;
    
    for(std::vector<TrigFTF_GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {
            
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

void TrigInDetTrackSeedingTool::createGraphNodes(const SpacePointCollection* spColl, std::vector<TrigFTF_GNN_Node>& tmpColl, unsigned short layer, float shift_x, float shift_y) const {
  tmpColl.resize(spColl->size(), TrigFTF_GNN_Node(layer));//all nodes belong to the same layer
  
  int idx = 0;
  for(const auto& sp : *spColl) {
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
    idx++;
  }
}

std::pair<int, int> TrigInDetTrackSeedingTool::buildTheGraph(const IRoiDescriptor& roi, const std::unique_ptr<TrigFTF_GNN_DataStorage>& storage, std::vector<TrigFTF_GNN_Edge>& edgeStorage) const {

  const float cut_dphi_max      = m_LRTmode ? 0.07 : 0.012;
  const float cut_dcurv_max     = m_LRTmode ? 0.015 : 0.001;
  const float cut_tau_ratio_max = m_LRTmode ? 0.015 : 0.007;
  const float min_z0            = m_LRTmode ? -600.0 : roi.zedMinus();
  const float max_z0            = m_LRTmode ? 600.0 : roi.zedPlus();
  const float min_deltaPhi      = m_LRTmode ? 0.01f : 0.001f;
  
  const float maxOuterRadius    = m_LRTmode ? 1050.0 : 550.0;

  const float cut_zMinU = min_z0 + maxOuterRadius*roi.dzdrMinus();
  const float cut_zMaxU = max_z0 + maxOuterRadius*roi.dzdrPlus();

  const float ptCoeff = 0.29997*1.9972/2.0;// ~0.3*B/2 - assuming nominal field of 2*T

  float tripletPtMin = 0.8*m_minPt;//correction due to limited pT resolution
  
  float maxCurv = ptCoeff/tripletPtMin;
 
  const float maxKappa_high_eta          = m_LRTmode ? 1.0*maxCurv : std::sqrt(0.8)*maxCurv;
  const float maxKappa_low_eta           = m_LRTmode ? 1.0*maxCurv : std::sqrt(0.6)*maxCurv;

  const float minDeltaRadius = 2.0;
    
  float deltaPhi = 0.5f*m_phiSliceWidth;//the default sliding window along phi
 
  //1. loop over stages

  int currentStage = 0;

  unsigned int nConnections = 0;
  
  edgeStorage.reserve(m_nMaxEdges);
  
  int nEdges = 0;

  for(std::map<int, std::vector<GNN_FASTRACK_CONNECTOR::LayerGroup> >::const_iterator it = m_connector->m_layerGroups.begin();it!=m_connector->m_layerGroups.end();++it, currentStage++) {
    
    //loop over L1 layers for the current stage

    for(const auto& layerGroup : (*it).second) {
      
      unsigned int dst = layerGroup.m_dst;//n1 : inner nodes
      
      const TrigFTF_GNN_Layer* pL1 = m_geo->getTrigFTF_GNN_LayerByKey(dst);

      if (pL1==nullptr) {
	continue; 
      }

      for(const auto& conn : layerGroup.m_sources) {//loop over L2(L1) for the current stage

	unsigned int src = conn->m_src;//n2 : the new connectors

	const TrigFTF_GNN_Layer* pL2 = m_geo->getTrigFTF_GNN_LayerByKey(src);

	if (pL2==nullptr) {
	  continue; 
	}

	int nDstBins = pL1->m_bins.size();
	int nSrcBins = pL2->m_bins.size();

	for(int b1=0;b1<nDstBins;b1++) {//loop over bins in Layer 1

	  TrigFTF_GNN_EtaBin& B1 = storage->getEtaBin(pL1->m_bins.at(b1));

	  if(B1.empty()) continue;

	  float rb1 = pL1->getMinBinRadius(b1);
	  
	  //3. loops over source eta-bins
	  
	  for(int b2=0;b2<nSrcBins;b2++) {//loop over bins in Layer 2
	  
	    if(m_useEtaBinning && (nSrcBins+nDstBins > 2)) {
	      if(conn->m_binTable[b1 + b2*nDstBins] != 1) continue;//using precomputed LUT
	    }
	  
	    const TrigFTF_GNN_EtaBin& B2 = storage->getEtaBin(pL2->m_bins.at(b2));

	    if(B2.empty()) continue;

	    float rb2 = pL2->getMaxBinRadius(b2);

	    //calculate delta Phi for rb1 ---> rb2 extrapolation
	    
	    if(m_useEtaBinning) {
	      deltaPhi = min_deltaPhi + maxCurv*std::fabs(rb2-rb1);
	    }
	    
	    unsigned int first_it = 0;

	    for(unsigned int n1Idx = 0;n1Idx<B1.m_vn.size();n1Idx++) {//loop over nodes in Layer 1
                
              if(B1.m_in[n1Idx].size() >= MAX_SEG_PER_NODE) continue;
              
              const std::array<float, 5>& n1pars = B1.m_params[n1Idx];

              float phi1 = n1pars[2];
              float r1 = n1pars[3];
              float z1 = n1pars[4];
	      
	      //sliding window phi1 +/- deltaPhi
	      
	      float minPhi = phi1 - deltaPhi;
	      float maxPhi = phi1 + deltaPhi;
	      
	      for(unsigned int n2PhiIdx = first_it; n2PhiIdx<B2.m_vPhiNodes.size();n2PhiIdx++) {//sliding window over nodes in Layer 2
		
		float phi2 = B2.m_vPhiNodes.at(n2PhiIdx).first;

		if(phi2 < minPhi) {
		  first_it = n2PhiIdx;
		  continue;
		}
		if(phi2 > maxPhi) break;
		
		unsigned int n2Idx = B2.m_vPhiNodes[n2PhiIdx].second;
		
                const std::vector<unsigned int>& v2In = B2.m_in[n2Idx];
                const std::array<float, 5>& n2pars = B2.m_params[n2Idx];
                
                if(v2In.size() >= MAX_SEG_PER_NODE) continue;
		
                float r2 = n2pars[3];

		float dr = r2 - r1;
	      
		if(dr < minDeltaRadius) {
		  continue;
		}
	      
		float z2 = n2pars[4];

		float dz = z2 - z1;
		float tau = dz/dr;
		float ftau = std::fabs(tau);
		if (ftau > 36.0) {
		  continue;
		}
		
		if(ftau < n1pars[0]) continue;
                if(ftau < n2pars[0]) continue;
                if(ftau > n1pars[1]) continue;
                if(ftau > n2pars[1]) continue;
		
		if (m_doubletFilterRZ) {
		  
		  float z0 = z1 - r1*tau;
		  
		  if(z0 < min_z0 || z0 > max_z0) continue;
		  
		  float zouter = z0 + maxOuterRadius*tau;
		  
		  if(zouter < cut_zMinU || zouter > cut_zMaxU) continue;                
		}
		
		float curv = (phi2-phi1)/dr;
		float abs_curv = std::abs(curv);
		
		if(ftau < 4.0) {//eta = 2.1
		  if(abs_curv > maxKappa_low_eta) {
		    continue;
		  }
		  
		}
		else {
		  if(abs_curv > maxKappa_high_eta) {
		    continue;
		  }
		}

		//match edge candidate against edges incoming to n2

		float exp_eta = std::sqrt(1+tau*tau)-tau;

		bool isGood = v2In.size() <= 2;//we must have enough incoming edges to decide

		if(!isGood) {

		  float uat_1 = 1.0f/exp_eta;
		    
		  for(const auto& n2_in_idx : v2In) {
		    
		    float tau2 = edgeStorage.at(n2_in_idx).m_p[0]; 
		    float tau_ratio = tau2*uat_1 - 1.0f;
		    
		    if(std::fabs(tau_ratio) > cut_tau_ratio_max){//bad match
		      continue;
		    }
		    isGood = true;//good match found
		    break;
		  }
		}
		
		if(!isGood) {//no match found, skip creating [n1 <- n2] edge
		  continue;
		}
		
		float dPhi2 = curv*r2;
                float dPhi1 = curv*r1;
		
		if(nEdges < m_nMaxEdges) {

		  edgeStorage.emplace_back(B1.m_vn[n1Idx], B2.m_vn[n2Idx], exp_eta, curv, phi1 + dPhi1);

		  std::vector<unsigned int>& v1In = B1.m_in[n1Idx];
                  
                  if(v1In.size() < MAX_SEG_PER_NODE) v1In.push_back(nEdges);
		  
                  int outEdgeIdx = nEdges;

		  float uat_2  = 1/exp_eta;
                  float Phi2  = phi2 + dPhi2;
                  float curv2 = curv;
		  
		  for(const auto& inEdgeIdx : v2In) {//looking for neighbours of the new edge

		    TrigFTF_GNN_Edge* pS = &(edgeStorage.at(inEdgeIdx));

                    if(pS->m_nNei >= N_SEG_CONNS) continue;
		    
		    float tau_ratio = pS->m_p[0]*uat_2 - 1.0f;
                    
                    if(std::abs(tau_ratio) > cut_tau_ratio_max){//bad match
                      continue;
                    }
                    
                    float dPhi =  Phi2 - pS->m_p[2];
        
                    if(dPhi<-M_PI) dPhi += 2*M_PI;
                    else if(dPhi>M_PI) dPhi -= 2*M_PI;
        
                    if(dPhi < -cut_dphi_max || dPhi > cut_dphi_max) {
                      continue;
                    }
                    
                    float dcurv = curv2 - pS->m_p[1];
                
                    if(dcurv < -cut_dcurv_max || dcurv > cut_dcurv_max) {
                      continue;
                    }
                
                    pS->m_vNei[pS->m_nNei++] = outEdgeIdx;

		    nConnections++;
		    
                  }
		  nEdges++;		
		}
	      } //loop over n2 (outer) nodes
	    } //loop over n1 (inner) nodes 
	  } //loop over source eta bins
	} //loop over dst eta bins
      } //loop over L2(L1) layers
    } //loop over dst layers
  } //loop over the stages of doublet making

  return std::make_pair(nEdges, nConnections);

}

int TrigInDetTrackSeedingTool::runCCA(int nEdges, std::vector<TrigFTF_GNN_Edge>& edgeStorage) const {

  const int maxIter = 15;

  int maxLevel = 0;

  int iter = 0;
  
  std::vector<TrigFTF_GNN_Edge*> v_old;
  
  for(int edgeIndex=0;edgeIndex<nEdges;edgeIndex++) {

    TrigFTF_GNN_Edge* pS = &(edgeStorage[edgeIndex]);
    if(pS->m_nNei == 0) continue;
    
    v_old.push_back(pS);//TO-DO: increment level for segments as they already have at least one neighbour
  }

  for(;iter<maxIter;iter++) {

    //generate proposals
    std::vector<TrigFTF_GNN_Edge*> v_new;
    v_new.clear();
    v_new.reserve(v_old.size());
    
    for(auto pS : v_old) {
      
      int next_level = pS->m_level;
          
      for(int nIdx=0;nIdx<pS->m_nNei;nIdx++) {
	
        unsigned int nextEdgeIdx = pS->m_vNei[nIdx];
            
        TrigFTF_GNN_Edge* pN = &(edgeStorage[nextEdgeIdx]);
            
        if(pS->m_level == pN->m_level) {
          next_level = pS->m_level + 1;
          v_new.push_back(pS);
          break;
        }
      }
      
      pS->m_next = next_level;//proposal
    }
  
    //update

    int nChanges = 0;
      
    for(auto pS : v_new) {
      if(pS->m_next != pS->m_level) {
        nChanges++;
        pS->m_level = pS->m_next;
        if(maxLevel < pS->m_level) maxLevel = pS->m_level;
      }
    }

    if(nChanges == 0) break;


    v_old = std::move(v_new);
    v_new.clear();
  }

  return maxLevel;  
}

