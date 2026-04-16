/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "AtlasDetDescr/AtlasDetectorID.h"

#include "PathResolver/PathResolver.h"

#include "GNN_TrackingFilter.h"

#include "IRegionSelector/IRegSelTool.h"

#include "SeedingToolBase.h"

#include "GNN_TrackingFilter.h"

#include <cmath>

#include <algorithm> //for std::sort

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

  
  struct GBTS_SlidingWindow {

    GBTS_SlidingWindow() : m_first_it(0), m_deltaPhi(0.0), m_has_nodes(false), m_bin(nullptr) {};
    
    unsigned int m_first_it;// sliding window position
    float m_deltaPhi;       // window half-width;
    bool m_has_nodes;       // active or not

    const TrigFTF_GNN_EtaBin* m_bin;//associated eta bin
  };
  
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
    
  float deltaPhi0 = 0.5f*m_phiSliceWidth;//the default sliding window along phi
  
  unsigned int nConnections = 0;
  
  edgeStorage.reserve(m_nMaxEdges);
  
  int nEdges = 0;

  float z0_histo_coeff = 16/(max_z0 - min_z0 + 1e-6);//assuming 16-bit z0 bitmask

  for(const auto& bg : m_geo->bin_groups()) {//loop over bin groups
    
    TrigFTF_GNN_EtaBin& B1 = storage->getEtaBin(bg.first);

    if(B1.empty()) continue;

    float rb1 = B1.getMinBinRadius();
    
    const unsigned int lk1 = B1.m_layerKey;

    const bool isBarrel1 = (lk1 / 10000) == 8;
    
    //prepare a sliding window for each bin2 in the group 

    std::vector<GBTS_SlidingWindow> vSLW;

    vSLW.resize(bg.second.size());//initialization using default ctor

    int win_idx = 0;
    
    for(const auto& b2_idx : bg.second) { //loop over n2 eta-bins in L2 layers

      const TrigFTF_GNN_EtaBin& B2 = storage->getEtaBin(b2_idx);

      if(B2.empty()) {
        win_idx++;
        continue;
      }
      
      float rb2 = B2.getMaxBinRadius();

      float deltaPhi = deltaPhi0;//the default
      
      if(m_useEtaBinning) { //override the default window width
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

      vSLW[win_idx].m_bin = &B2;
      vSLW[win_idx].m_has_nodes = true;
      vSLW[win_idx].m_deltaPhi = deltaPhi;
      win_idx++;
    }

    for(unsigned int n1Idx = 0;n1Idx<B1.m_vn.size();n1Idx++) {//in GBTSv3 the outer loop goes over n1 nodes in the Layer 1 bin

      B1.m_vFirstEdge[n1Idx] = nEdges;//initialization using the top watermark of the edge storage

      unsigned short num_created_edges = 0;//the counter for the incoming graph edges created for n1

      bool is_connected = false;

      std::array<unsigned char, 16> z0_histo = {};
  
      const std::array<float, 5>& n1pars = B1.m_params[n1Idx];

      float phi1 = n1pars[2];
      float r1 = n1pars[3];
      float z1 = n1pars[4];

      for(unsigned int winIdx = 0; winIdx < vSLW.size(); winIdx++) {//the intermediate loop over sliding windows

        GBTS_SlidingWindow& slw = vSLW[winIdx];
 
        if (!slw.m_has_nodes) continue;

        const TrigFTF_GNN_EtaBin& B2 = *slw.m_bin;

	const unsigned int lk2 = B2.m_layerKey;

	const bool isBarrel2 = (lk2 / 10000) == 8;
 
        float deltaPhi = slw.m_deltaPhi;
      
        //sliding window phi1 +/- deltaPhi
      
        float minPhi = phi1 - deltaPhi;
        float maxPhi = phi1 + deltaPhi;
      
	for(unsigned int n2PhiIdx = slw.m_first_it; n2PhiIdx<B2.m_vPhiNodes.size();n2PhiIdx++) {//the inner loop over n2 nodes using sliding window
	
	  float phi2 = B2.m_vPhiNodes[n2PhiIdx].first;

	  if(phi2 < minPhi) {
            slw.m_first_it = n2PhiIdx; //update the window position
            continue;
          }
          if(phi2 > maxPhi) break; //break and go to the next window
	  
	  unsigned int n2Idx = B2.m_vPhiNodes[n2PhiIdx].second;

	  unsigned short node_info = B2.m_vIsConnected[n2Idx];

	  if ((lk1 == 80000) && (node_info == 0) ) continue;//skip isolated nodes as their incoming edges lead to nowhere
 
	  unsigned int   n2_first_edge = B2.m_vFirstEdge[n2Idx];
          unsigned short n2_num_edges  = B2.m_vNumEdges[n2Idx];
	  unsigned int   n2_last_edge  = n2_first_edge + n2_num_edges;
	  
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

	  float z0 = z1 - r1*tau;

	  if (lk1 == 80000) {//check against non-empty z0 histogram
	    
	    if ( !check_z0_bitmask(node_info, z0, min_z0, z0_histo_coeff) ) {
	      continue;
	    }
	  }
	  
	  if (m_doubletFilterRZ) {
	    
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

	  float exp_eta = std::sqrt(1.f+tau*tau) - tau;
        
	  if (m_matchBeforeCreate && (lk1 == 80000 || lk1 == 81000) ) {//match edge candidate against edges incoming to n2

	    bool isGood = n2_num_edges <= 2;//we must have enough incoming edges to decide
	    
	    if(!isGood) {

	      float uat_1 = 1.0f/exp_eta;
		    
	      for(unsigned int n2_in_idx = n2_first_edge; n2_in_idx < n2_last_edge; n2_in_idx++) {
		    
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

	    num_created_edges++;
		  
	    int outEdgeIdx = nEdges;
	  
	    float uat_2  = 1.f/exp_eta;
	    float Phi2  = phi2 + dPhi2;
	    float curv2 = curv;
	    
	    for(unsigned int inEdgeIdx = n2_first_edge; inEdgeIdx < n2_last_edge; inEdgeIdx++) {//looking for neighbours of the new edge
	      
	      TrigFTF_GNN_Edge* pS = &(edgeStorage.at(inEdgeIdx));
	      
	      if(pS->m_nNei >= N_SEG_CONNS) continue;

	      const unsigned int lk3 = m_geo->getTrigFTF_GNN_LayerKeyByIndex(pS->m_n2->m_layer);

	      const bool isBarrel3 = (lk3 / 10000) == 8;
	      
	      float abs_tau_ratio = std::abs(pS->m_p[0]*uat_2 - 1.0f);
	      float add_tau_ratio_corr = 0;
	      
	      if (m_useAdaptiveCuts) {

		if (isBarrel1 && isBarrel2 && isBarrel3) {
		  bool no_gap = ((lk3-lk2) == 1000) && ((lk2-lk1) == 1000);
		  if(!no_gap) {
		    add_tau_ratio_corr = m_tau_ratio_corr;//assume more scattering due to the layer in between
		  }
		}
		else {
		  bool mixed_triplet = isBarrel1 && isBarrel2 && !isBarrel3;
		  if (mixed_triplet) {
		    add_tau_ratio_corr = m_tau_ratio_corr;
		  }
		}
	      }
	      
	      if(abs_tau_ratio > cut_tau_ratio_max + add_tau_ratio_corr){//bad match
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

	      //final check: cuts on pT and d0
	      
	      if (isBarrel1 && isBarrel2 && isBarrel3) {//Pixel barrel

                std::array<const GNN_Node*, 3> sps = {B1.m_vn[n1Idx], B2.m_vn[n2Idx], pS->m_n2};

                if (!validate_triplet(sps, tripletPtMin, abs_tau_ratio, cut_tau_ratio_max) ) continue;
		
              }
            
	      pS->m_vNei[pS->m_nNei++] = outEdgeIdx;

	      is_connected = true;//there is at least one good match

	      //edge confirmed - update z0 histogram

	      int z0_bin_index = z0_histo_coeff*(z0 - min_z0);

	      ++z0_histo[z0_bin_index];
	      
	      nConnections++;
	    
	    }
	    nEdges++;		
	  }
	} //loop over n2 (outer) nodes inside a sliding window on n2 bin
      } //loop over sliding windows associated with n2 bins

      //updating the n1 node attributes
      
      B1.m_vNumEdges[n1Idx] = num_created_edges;

      if (is_connected) {

        unsigned short z0_bitmask = 0x0;

        for(unsigned int bIdx = 0; bIdx < 16; bIdx++) {

	  if (z0_histo[bIdx] == 0) continue;

	  z0_bitmask |= (1 << bIdx);
        }

        B1.m_vIsConnected[n1Idx] = z0_bitmask;//non-zero mask indicates that there is at least one connected edge
      }
      
    } //loop over n1 (inner) nodes
  } //loop over bin groups: a single n1 bin and multiple n2 bins

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

void SeedingToolBase::extractSeedsFromTheGraph(int maxLevel, int nEdges, int nHits, std::vector<GNN_Edge>& edgeStorage, std::vector<std::pair<float, std::vector<unsigned int> > >& vOutputSeeds) const {

  const float edge_mask_min_eta      = 1.5;
  const float hit_share_threshold    = 0.49;
  const float max_inv_rad_diff       = 0.7e-2;//in inverse meters

  int minLevel = 3;//a triplet + 1 confirmation

  if(m_LRTmode) {
    minLevel = 2;//a triplet + no confirmation
  }

  if(maxLevel < minLevel) return;

  std::vector<GNN_Edge*> vChainHeads;

  vChainHeads.reserve(nEdges/2);

  for(int edgeIndex = 0; edgeIndex < nEdges; edgeIndex++) {
    
    GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

    if (m_LRTmode || !m_addTriplets) {
      if(pS->m_level < minLevel) continue;
    }
    else { //eta-dependent cut
      float edge_eta = std::abs(-std::log(pS->m_p[0]));

      if (edge_eta > m_max_eta_add_triplets) {
        if(pS->m_level < minLevel) continue;
      }
      else {
        if(pS->m_level < minLevel - 1) continue;
      }
    }
    
    vChainHeads.push_back(pS);
  }
  
  if(vChainHeads.empty()) return;
  
  std::sort(vChainHeads.begin(), vChainHeads.end(), GNN_Edge::CompareLevel());
    
  //backtracking

  std::vector<std::tuple<float, int, std::vector<const GNN_Node*>, int > > vSeedCandidates;

  vSeedCandidates.reserve(vChainHeads.size());

  std::vector<std::pair<float, unsigned int> > vArgSort;

  vArgSort.reserve(vChainHeads.size());

  unsigned int seed_counter = 0;
  
  auto tFilter = std::make_unique<TrigFTF_GNN_TrackingFilter>(m_layerGeometry, edgeStorage);

  for(auto pS : vChainHeads) {

    if(pS->m_level == -1) continue;

    TrigFTF_GNN_EdgeState rs(false);

    tFilter->followTrack(pS, rs);

    if(!rs.m_initialized) {
      continue;
    }
    
    float seed_eta = std::abs(-std::log(pS->m_p[0]));

    int chain_length = static_cast<int>(rs.m_vs.size());

    if (m_LRTmode || !m_addTriplets) {
      if(chain_length < minLevel) continue;
    }
    else {
      if (seed_eta > m_max_eta_add_triplets) {
        if(chain_length < minLevel) continue;
      }
      else {
        if(chain_length < minLevel - 1) continue;
      }
    }

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

    if(vN.size()<3) continue; //a triplet are accepted if it makes upto this point 

    unsigned int orig_seed_size = vN.size();

    float orig_seed_quality = -rs.m_J/orig_seed_size;
    
    int seed_split_flag = (seed_eta < m_max_eta_for_seed_split) && (orig_seed_size > 3) && (orig_seed_size <= 5) ? 1 : 0;

    if (seed_split_flag) {//split the seed by dropping spacepoints
      
      std::array< std::array<const GNN_Node*, 3>, 3> triplets;//2 "drop-outs" and the original seed candidate
      
      std::array<float, 3> inv_rads;//triplet parameter estimate

      triplets[0] = {vN[0], vN[orig_seed_size/2], vN[orig_seed_size-1]};

      std::vector<const GNN_Node*> drop_out1 = {vN.begin()+1, vN.end()}; //all but the first one
      
      triplets[1] = {drop_out1[0], drop_out1[(orig_seed_size-1)/2], drop_out1[orig_seed_size-2]};
      
      std::vector<const GNN_Node*> drop_out2;

      drop_out2.reserve(orig_seed_size-1);
      
      for(unsigned int k = 0; k < orig_seed_size; k++) {

        if (k == orig_seed_size/2) continue;//drop the middle SP in the original seed
        
        drop_out2.emplace_back(vN[k]);
      }

      triplets[2] = {drop_out2[0], drop_out2[(orig_seed_size-1)/2], drop_out2[orig_seed_size-2]};

      for (unsigned int k = 0; k < inv_rads.size(); k++) {

        inv_rads[k] = estimate_curvature(triplets[k]);
	
      }

      float diffs[3] = {std::abs(inv_rads[1] - inv_rads[0]), std::abs(inv_rads[2] - inv_rads[0]), std::abs(inv_rads[2] - inv_rads[1])};

      bool confirmed = diffs[0] < max_inv_rad_diff && diffs[1] < max_inv_rad_diff && diffs[2] < max_inv_rad_diff;

      if (confirmed) {
        seed_split_flag = 0;//reset the flag
      }
      
    }
        
    vSeedCandidates.emplace_back(orig_seed_quality, 0, vN, seed_split_flag);
    
    vArgSort.emplace_back(orig_seed_quality, seed_counter);

    ++seed_counter;
    
  }
  
  //clone removal code goes below ...

  std::sort(vArgSort.begin(), vArgSort.end());
  
  std::vector<int> H2T(nHits + 1, 0);//hit to track associations

  int trackId = 0;

  
  for(const auto& ags : vArgSort) {

    const auto& seed = vSeedCandidates[ags.second];
    
    trackId++;
    
    for(const auto& h : std::get<2>(seed) ) {//loop over spacepoints indices
	
      unsigned int hit_id = h->sp_idx() + 1;
      
      int tid     = H2T[hit_id];
      
      if(tid == 0 || tid > trackId) {//unused hit or used by a lesser track
	
	H2T[hit_id] = trackId;//overwrite
	
      }
    }      
  }

  unsigned int trackIdx = 0;
 
  for(const auto& ags : vArgSort) {

    const auto& seed = std::get<2>(vSeedCandidates[ags.second]);
    
    int nTotal = seed.size();
    
    int nOther = 0;
    
    int trackId = trackIdx + 1;

    ++trackIdx;

    for(const auto& h : seed ) {

      unsigned int hit_id = h->sp_idx() + 1;
      
      int tid = H2T[hit_id];

      if(tid != trackId) {//taken by a better candidate
	nOther++;
      }
    }

    if (nOther > hit_share_threshold*nTotal) {
      std::get<1>(vSeedCandidates[ags.second]) = -1;//reject
    }

  }

  vOutputSeeds.reserve(vSeedCandidates.size());
  
  //drop the clones and split seeds if need be

  for(const auto& ags : vArgSort) {

    const auto& seed = vSeedCandidates[ags.second];
    
    if (std::get<1>(seed) != 0) continue;//identified as a clone of a better candidate

    const auto& vN = std::get<2>(seed);
 
    if (std::get<3>(seed) == 0) {
      
      //add seed to output

      std::vector<unsigned int> vSpIdx;
      
      vSpIdx.resize(vN.size());
    
      for(unsigned int k = 0; k < vSpIdx.size(); k++) {
	vSpIdx[k] = vN[k]->sp_idx();
      }

      vOutputSeeds.emplace_back(std::get<0>(seed), vSpIdx);

      continue;

    }

    //seed split into "drop-out" seeds 

    unsigned int seedSize = vN.size();
        
    std::array<std::size_t, 2> indices2drop = {0, seedSize / 2ul};//the first and the middle

    for(const auto& skipIdx : indices2drop) {

      std::vector<unsigned int> new_seed;

      new_seed.reserve(seedSize-1);
        
      for (unsigned int k = 0; k < seedSize; k++) {
          
	if (k ==  skipIdx) continue;
          
	new_seed.emplace_back(vN[k]->sp_idx());         
      }

      vOutputSeeds.emplace_back(std::get<0>(seed), new_seed);        

    }
    
  }
  
}

bool SeedingToolBase::check_z0_bitmask(const unsigned short& z0_bitmask, const float& z0, const float& min_z0, const float& z0_histo_coeff) const {

  if (z0_bitmask == 0) return true;

  float dz = z0 - min_z0; 
  int z0_bin_index = z0_histo_coeff*dz;

  if ((z0_bitmask >> z0_bin_index) & 1) return true;

  //check adjacent bins as well
            
  const float z0_resolution = 2.5;
  
  float dzm = dz - z0_resolution;

  int next_bin  = z0_histo_coeff*dzm;

  if (next_bin >= 0 && next_bin != z0_bin_index) {
      
    if ((z0_bitmask >> next_bin) & 1) return true;

  }				  

  float dzp = dz + z0_resolution;

  next_bin  = z0_histo_coeff*dzp;

  if (next_bin < 16 && next_bin != z0_bin_index) {
		    
    if ((z0_bitmask >> next_bin) & 1) return true;
      
  }
    
  return false;
}


float SeedingToolBase::estimate_curvature(const std::array<const GNN_Node*, 3>& sps) const {

  //conformal mapping with the center at the last spacepoint

  float u[2], v[2];

  float x0 = sps[2]->x();
  float y0 = sps[2]->y();

  float r0 = sps[2]->r();
  
  float cosA = x0/r0;
  
  float sinA = y0/r0;

  
  for(unsigned int k=0;k<2;k++) {

    float dx = sps[k]->x() - x0;

    float dy = sps[k]->y() - y0;

    float r2_inv = 1.0/(dx*dx+dy*dy);
    
    float xn = dx*cosA + dy*sinA;
    
    float yn =-dx*sinA + dy*cosA;

    u[k] = xn*r2_inv;
    v[k] = yn*r2_inv;    
  }

  float du = u[0] - u[1];

  if(du==0.0) return 0.0;
  
  float A = (v[0] - v[1])/du;

  float B = v[1] - A*u[1];

  return 1000.0*B/std::sqrt(1 + A*A); //inverse meters
  
}

bool SeedingToolBase::validate_triplet(std::array<const GNN_Node*, 3>& sps, const float min_pT, const float tau_ratio, const float tau_ratio_cut) const {
  
  //conformal mapping with the center at the middle spacepoint

  float u[2], v[2];

  const float x0 = sps[1]->x();
  const float y0 = sps[1]->y();

  const float r0 = sps[1]->r();
  
  const float cosA = x0/r0;
  
  const float sinA = y0/r0;
  
  for(unsigned int k=0;k<2;k++) {

    int sp_idx = (k==1) ? 2 : k;
    
    const float dx = sps[sp_idx]->x() - x0;

    const float dy = sps[sp_idx]->y() - y0;

    const float r2_inv = 1.0/(dx*dx+dy*dy);
    
    const float xn = dx*cosA + dy*sinA;
    
    const float yn =-dx*sinA + dy*cosA;

    u[k] = xn*r2_inv;
    v[k] = yn*r2_inv;    
  }

  const float du = u[0] - u[1];

  if ( du == 0.0 ) return false;
  
  const float A = (v[0] - v[1])/du;

  const float B = v[1] - A*u[1];

  const float d0 = r0*(B*r0 - A);

  if (std::abs(d0) > m_d0_max) return false;
  
  if (B != 0.0) {//straight-line track is OK
  
    const float R = std::sqrt(1 + A*A)/B; //signed radius in mm

    const float pT = std::abs(0.3*R); //asssuming uniform 2T field

    if (pT < min_pT) return false;

    if (pT > 5*min_pT) {//relatively high-pT track

      if (tau_ratio > 0.9*tau_ratio_cut) return false;

    }
    
  }

  return true;

}
