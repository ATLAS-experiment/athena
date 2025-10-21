/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "AtlasDetDescr/AtlasDetectorID.h"

#include "PathResolver/PathResolver.h"

#include "GNN_TrackingFilter.h"

#include "IRegionSelector/IRegSelTool.h"

#include "SeedingToolBase.h"

#include "GNN_TrackingFilter.h"

#include <numeric>

StatusCode SeedingToolBase::initialize() {
  ATH_CHECK(AthAlgTool::initialize());

  ATH_CHECK(m_layerNumberTool.retrieve());


  ATH_CHECK(detStore()->retrieve(m_atlasId, "AtlasID"));

  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));

  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));

  std::string conn_fileName = PathResolver::find_file(m_connectionFile, "DATAPATH");
  if (conn_fileName.empty()) {
    ATH_MSG_FATAL("Cannot find layer connections file " << conn_fileName);
    return StatusCode::FAILURE;
  }
  else {

    std::ifstream ifs(conn_fileName.c_str());
    
    m_connector = std::make_unique<GNN_FASTRACK_CONNECTOR>(ifs, m_LRTmode);
    if (m_etaBinOverride != 0.0f) {
      m_connector->m_etaBin = m_etaBinOverride;
    }
    
    ATH_MSG_INFO("Layer connections are initialized from file " << conn_fileName);
  }

  const std::vector<TrigInDetSiLayer>* pVL = m_layerNumberTool->layerGeometry();
  
  std::copy(pVL->begin(),pVL->end(), std::back_inserter(m_layerGeometry));

  m_geo = std::make_unique<TrigFTF_GNN_Geometry>(m_layerGeometry, m_connector);


  if (m_useML) {
    std::string lut_fileName = PathResolver::find_file(m_lutFile, "DATAPATH");
    if (lut_fileName.empty()) {
      ATH_MSG_FATAL("Cannot find ML predictor LUT file " << lut_fileName);
      return StatusCode::FAILURE;
    }
    else {
      m_mlLUT.reserve(100);
      std::ifstream ifs(lut_fileName.c_str());
      while (!ifs.eof()) {
	float cl_width, min1, max1, min2, max2;
	ifs >> cl_width >> min1 >> max1 >> min2 >> max2;
	if (ifs.eof()) break;
	std::array<float, 5> lut_line = {cl_width, min1, max1, min2, max2};
	m_mlLUT.emplace_back(lut_line);
      }
      ifs.close();
      ATH_MSG_INFO("ML predictor is initialized from file " << lut_fileName<<" LUT has "<<m_mlLUT.size()<<" entries");
    }
  }
  
  m_phiSliceWidth = 2*M_PI/m_nMaxPhiSlice;
  
  ATH_MSG_INFO("SeedingToolBase initialized ");

  ATH_MSG_DEBUG("Property useML "<< m_useML);
  ATH_MSG_DEBUG("Property DoPhiFiltering "<<m_filter_phi);
  ATH_MSG_DEBUG("Property pTmin "<<m_minPt);
  ATH_MSG_DEBUG("Property LRTmode "<<m_LRTmode);
 
  return StatusCode::SUCCESS;
}

StatusCode SeedingToolBase::finalize() {
  StatusCode sc = AthAlgTool::finalize(); 
  return sc;
}

std::pair<int, int> SeedingToolBase::buildTheGraph(const IRoiDescriptor& roi, const std::unique_ptr<TrigFTF_GNN_DataStorage>& storage, std::vector<TrigFTF_GNN_Edge>& edgeStorage) const {

  constexpr float M_2PI = 2.0*M_PI;
  
  const float cut_dphi_max      = m_LRTmode ? 0.07f : 0.012f;
  const float cut_dcurv_max     = m_LRTmode ? 0.015f : 0.001f;
  const float cut_tau_ratio_max = m_LRTmode ? 0.015f : static_cast<float>(m_tau_ratio_cut);
  const float min_z0            = m_LRTmode ? -600.0 : roi.zedMinus();
  const float max_z0            = m_LRTmode ? 600.0 : roi.zedPlus();
  const float min_deltaPhi      = m_LRTmode ? 0.01f : 0.001f;
  const float tau_ratio_precut  = 0.009f;
  
  const float maxOuterRadius    = m_LRTmode ? 1050.0 : 550.0;

  const float cut_zMinU = min_z0 + maxOuterRadius*roi.dzdrMinus();
  const float cut_zMaxU = max_z0 + maxOuterRadius*roi.dzdrPlus();

  constexpr float ptCoeff = 0.29997*1.9972/2.0;// ~0.3*B/2 - assuming nominal field of 2*T

  float tripletPtMin = 0.8f*m_minPt;//correction due to limited pT resolution
  const float pt_scale     = 900.0f/m_minPt;//to re-scale original tunings done for the 900 MeV pT cut
  
  float maxCurv = ptCoeff/tripletPtMin;
 
  float maxKappa_high_eta          = m_LRTmode ? 1.0f*maxCurv : std::sqrt(0.8f)*maxCurv;
  float maxKappa_low_eta           = m_LRTmode ? 1.0f*maxCurv : std::sqrt(0.6f)*maxCurv;
  
  if(!m_useOldTunings && !m_LRTmode) {//new settings for curvature cuts
    maxKappa_high_eta          = 4.75e-4f*pt_scale;
    maxKappa_low_eta           = 3.75e-4f*pt_scale;
  }

  const float dphi_coeff                 = m_LRTmode ? 1.0f*maxCurv : 0.68f*maxCurv;
  
  const float minDeltaRadius = 2.0;
    
  float deltaPhi = 0.5f*m_phiSliceWidth;//the default sliding window along phi
 
  unsigned int nConnections = 0;
  
  edgeStorage.reserve(m_nMaxEdges);
  
  int nEdges = 0;

  for(const auto& bg : m_geo->bin_groups()) {//loop over bin groups
    
    TrigFTF_GNN_EtaBin& B1 = storage->getEtaBin(bg.first);

    if(B1.empty()) continue;

    float rb1 = B1.getMinBinRadius();
    
    const unsigned int lk1 = B1.m_layerKey;

    for(const auto& b2_idx : bg.second) {

      const TrigFTF_GNN_EtaBin& B2 = storage->getEtaBin(b2_idx);

      if(B2.empty()) continue;
      
      float rb2 = B2.getMaxBinRadius();
      
      if(m_useEtaBinning) {
	float abs_dr = std::fabs(rb2-rb1);
	if (m_useOldTunings) {
	  deltaPhi = min_deltaPhi + dphi_coeff*abs_dr;
	}
	else {
	  if(abs_dr < 60.0) {
	    deltaPhi = 0.002f + 4.33e-4f*pt_scale*abs_dr;
	  } else {
	    deltaPhi = 0.015f + 2.2e-4f*pt_scale*abs_dr;
	  }
	}
      }

      unsigned int first_it = 0;

      for(unsigned int n1Idx = 0;n1Idx<B1.m_vn.size();n1Idx++) {//loop over nodes in Layer 1

	std::vector<unsigned int>& v1In = B1.m_in[n1Idx];   

	if(v1In.size() >= MAX_SEG_PER_NODE) continue;
      
	const std::array<float, 5>& n1pars = B1.m_params[n1Idx];

	float phi1 = n1pars[2];
	float r1 = n1pars[3];
	float z1 = n1pars[4];
      
	//sliding window phi1 +/- deltaPhi
      
	float minPhi = phi1 - deltaPhi;
	float maxPhi = phi1 + deltaPhi;
      
	for(unsigned int n2PhiIdx = first_it; n2PhiIdx<B2.m_vPhiNodes.size();n2PhiIdx++) {//sliding window over nodes in Layer 2
	
	  float phi2 = B2.m_vPhiNodes[n2PhiIdx].first;
	
	  if(phi2 < minPhi) {
	    first_it = n2PhiIdx;
	    continue;
	  }
	  if(phi2 > maxPhi) break;
	
	  unsigned int n2Idx = B2.m_vPhiNodes[n2PhiIdx].second;
	
	  const std::vector<unsigned int>& v2In = B2.m_in[n2Idx];
        
	  if(v2In.size() >= MAX_SEG_PER_NODE) continue;
		
	  const std::array<float, 5>& n2pars = B2.m_params[n2Idx];
	
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
	  if(ftau > n1pars[1]) continue;

	  if(ftau < n2pars[0]) continue;
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

	  float exp_eta = std::sqrt(1.f+tau*tau)-tau;
        
	  if (m_matchBeforeCreate && (lk1 == 80000 || lk1 == 81000) ) {//match edge candidate against edges incoming to n2

	    bool isGood = v2In.size() <= 2;//we must have enough incoming edges to decide

	    if(!isGood) {

	      float uat_1 = 1.0f/exp_eta;
		    
	      for(const auto& n2_in_idx : v2In) {
		    
		float tau2 = edgeStorage.at(n2_in_idx).m_p[0]; 
		float tau_ratio = tau2*uat_1 - 1.0f;
		
		if(std::fabs(tau_ratio) > tau_ratio_precut){//bad match
		  continue;
		}
		isGood = true;//good match found
		break;
	      }
	    }
	    
	    if(!isGood) {//no match found, skip creating [n1 <- n2] edge
	      continue;
	    }
	  }
	  
	  float dPhi2 = curv*r2;
	  float dPhi1 = curv*r1;
	
	  if(nEdges < m_nMaxEdges) {
	  
	    edgeStorage.emplace_back(B1.m_vn[n1Idx], B2.m_vn[n2Idx], exp_eta, curv, phi1 + dPhi1);
	    
	    if(v1In.size() < MAX_SEG_PER_NODE) v1In.push_back(nEdges);
		  
	    int outEdgeIdx = nEdges;
	  
	    float uat_2  = 1.f/exp_eta;
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
	      
	      if(dPhi<-M_PI) dPhi += M_2PI;
	      else if(dPhi>M_PI) dPhi -= M_2PI;
	      
	      if(std::abs(dPhi) > cut_dphi_max) {
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
    } //loop over bins in Layer 2
  } //loop over bin groups

  if(nEdges >= m_nMaxEdges) {
    ATH_MSG_WARNING("Maximum number of graph edges exceeded - possible efficiency loss "<< nEdges);
  }

  return std::make_pair(nEdges, nConnections);
}

int SeedingToolBase::runCCA(int nEdges, std::vector<TrigFTF_GNN_Edge>& edgeStorage) const {

  constexpr int maxIter = 15;

  int maxLevel = 0;

  int iter = 0;
  
  std::vector<TrigFTF_GNN_Edge*> v_old;
  
  for(int edgeIndex=0;edgeIndex<nEdges;edgeIndex++) {

    TrigFTF_GNN_Edge* pS = &(edgeStorage[edgeIndex]);
    if(pS->m_nNei == 0) continue;
    
    v_old.push_back(pS);//TO-DO: increment level for segments as they already have at least one neighbour
  }

  std::vector<TrigFTF_GNN_Edge*> v_new;
  v_new.reserve(v_old.size());

  for(;iter<maxIter;iter++) {

    //generate proposals
    v_new.clear();
    
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


    v_old.swap(v_new);
    v_new.clear();
  }

  return maxLevel;  
}

void SeedingToolBase::extractSeedsFromTheGraph(int maxLevel, int nEdges, int nHits, std::vector<GNN_Edge>& edgeStorage, std::vector<std::tuple<float, int, std::vector<unsigned int> > >& vSeedCandidates) const {

  const float edge_mask_min_eta = 1.5;
  const float hit_share_threshold = 0.49;
  
  vSeedCandidates.clear();

  int minLevel = 3;//a triplet + 2 confirmation

  if(m_LRTmode) {
    minLevel = 2;//a triplet + 1 confirmation
  }

  if(maxLevel < minLevel) return;
  
  std::vector<GNN_Edge*> vSeeds;

  vSeeds.reserve(nEdges/2);

  for(int edgeIndex = 0; edgeIndex < nEdges; edgeIndex++) {
    
    GNN_Edge* pS = &(edgeStorage.at(edgeIndex));
    
    if(pS->m_level < minLevel) continue;
    
    vSeeds.push_back(pS);
  }
  
  if(vSeeds.empty()) return;
  
  std::sort(vSeeds.begin(), vSeeds.end(), GNN_Edge::CompareLevel());
    
  //backtracking

  vSeedCandidates.reserve(vSeeds.size());
  
  TrigFTF_GNN_TrackingFilter tFilter(m_layerGeometry, edgeStorage);

  for(auto pS : vSeeds) {

    if(pS->m_level == -1) continue;

    TrigFTF_GNN_EdgeState rs(false);

    tFilter.followTrack(pS, rs);

    if(!rs.m_initialized) {
      continue;
    }

    if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;

    float seed_eta = std::abs(-std::log(pS->m_p[0]));
    
    std::vector<const GNN_Node*> vN;

    for(std::vector<GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {

      if (seed_eta > edge_mask_min_eta) {
	(*sIt)->m_level = -1;//mark as collected
      }
      
      if(sIt == rs.m_vs.rbegin()) {
	vN.push_back((*sIt)->m_n1);
      }

      vN.push_back((*sIt)->m_n2);
	    
    }

    if(vN.size()<3) continue;

    std::vector<unsigned int> vSpIdx;

    vSpIdx.resize(vN.size());

    for(unsigned int k = 0; k < vN.size(); k++) {
      vSpIdx[k] = vN[k]->sp_idx();
    }
    
    vSeedCandidates.emplace_back(-rs.m_J/vN.size(), 0, vSpIdx);
    
  }

  //clone removal code goes below ...

  std::sort(vSeedCandidates.begin(), vSeedCandidates.end());

  std::vector<int> vTrackIds(vSeedCandidates.size());

  // fills the vector from 1 to N
    
  std::iota(vTrackIds.begin(), vTrackIds.end(), 1);

  std::vector<int> H2T(nHits + 1, 0);//hit to track associations

  int seedIdx = 0;
    
  for(const auto& seed : vSeedCandidates) {
    
    for(const auto& h : std::get<2>(seed) ) {//loop over spacepoints indices
	
      unsigned int hit_id = h + 1;
      
      int tid     = H2T[hit_id];
      int trackId = vTrackIds[seedIdx];
      
      if(tid == 0 || tid > trackId) {//un-used hit or used by a lesser track
	
	H2T[hit_id] = trackId;//overwrite
	
      }
    }
    
    seedIdx++;
      
  }

  for(unsigned int trackIdx = 0; trackIdx < vSeedCandidates.size(); trackIdx++) {

    int nTotal = std::get<2>(vSeedCandidates[trackIdx]).size();
    int nOther = 0;
    
    int trackId = vTrackIds[trackIdx];

    for(const auto& h : std::get<2>(vSeedCandidates[trackIdx]) ) {

      unsigned int hit_id = h + 1;
      
      int tid = H2T[hit_id];

	if(tid != trackId) {//taken by a better candidate
          nOther++;
	}
    }

    if (nOther > hit_share_threshold*nTotal) {
        std::get<1>(vSeedCandidates[trackIdx]) = -1;//reject
    }

  }
}
