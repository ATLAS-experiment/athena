/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>
#include "TrigInDetEvent/TrigSiSpacePointBase.h"
#include "TrigInDetPattRecoEvent/TrigInDetTriplet.h"
#include "TrigInDetPattRecoEvent/TrigInDetSiLayer.h"
#include "GNN_DataStorage.h"
#include "GNN_TrackingFilter.h"

#include "TrigInDetPattRecoTools/TrigTrackSeedGenerator_ITk.h"
#include "TrigInDetPattRecoTools/FasTrackConnector.h"
#include "IRegionSelector/IRoiDescriptor.h"
#include "IRegionSelector/RoiUtil.h"
#include "InDetPrepRawData/PixelCluster.h"


TrigTrackSeedGeneratorITk::TrigTrackSeedGeneratorITk(const TrigCombinatorialSettings& tcs) 
  : m_settings(tcs), 
    m_minDeltaRadius(2.0)
{
  
  m_maxDeltaRadius = m_settings.m_doublet_dR_Max;
  m_phiSliceWidth = 2*M_PI/m_settings.m_nMaxPhiSlice;

  m_storage = new TrigFTF_GNN_DataStorage(*m_settings.m_geo);
  
  const double ptCoeff = 0.29997*1.9972/2.0;// ~0.3*B/2 - assumes nominal field of 2*T

  m_maxCurv = ptCoeff/m_settings.m_tripletPtMin;

}

TrigTrackSeedGeneratorITk::~TrigTrackSeedGeneratorITk() {
  delete m_storage;
  m_storage = nullptr;
}

void TrigTrackSeedGeneratorITk::loadSpacePoints(const std::vector<TrigSiSpacePointBase>& vSP) {

  //create GNN nodes

  bool useML = m_settings.m_useTrigSeedML > 0;
  
  for(std::vector<TrigSiSpacePointBase>::const_iterator it = vSP.begin();it != vSP.end();++it) {
 
    bool isPixel = (*it).isPixel();
    
    if (m_settings.m_LRTmode) {
      // Only Strip seeds in LRT mode
      if (isPixel) continue; 
    } else {
      // Only Pixel Seeds 
      if(!isPixel) continue;
    }
    m_storage->addSpacePoint(&(*it), useML);
  }
  m_storage->sortByPhi();
  m_storage->initializeNodes(useML);
  m_storage->generatePhiIndexing(1.5*m_phiSliceWidth);

}

void TrigTrackSeedGeneratorITk::runGNN_TrackFinder(const IRoiDescriptor* roiDescriptor, std::vector<GNN_TrigTracklet>& vTracks, bool makeTriplets) {

  const int MaxEdges = 2000000;

  const float cut_dphi_max      = m_settings.m_LRTmode ? 0.07 : 0.012;
  const float cut_dcurv_max     = m_settings.m_LRTmode ? 0.015 : 0.001;
  const float cut_tau_ratio_max = m_settings.m_LRTmode ? 0.015 : 0.007;
  const float min_z0            = m_settings.m_LRTmode ? -600.0 : roiDescriptor->zedMinus();
  const float max_z0            = m_settings.m_LRTmode ? 600.0 : roiDescriptor->zedPlus();
  const float min_deltaPhi      = m_settings.m_LRTmode ? 0.01f : 0.001f;
  
  const float maxOuterRadius    = m_settings.m_LRTmode ? 1050.0 : 550.0;

  const float cut_zMinU = min_z0 + maxOuterRadius*roiDescriptor->dzdrMinus();
  const float cut_zMaxU = max_z0 + maxOuterRadius*roiDescriptor->dzdrPlus();

  const float maxKappa_high_eta          = m_settings.m_LRTmode ? 1.0*m_maxCurv : std::sqrt(0.8)*m_maxCurv;
  const float maxKappa_low_eta           = m_settings.m_LRTmode ? 1.0*m_maxCurv : std::sqrt(0.6)*m_maxCurv;

  float deltaPhi = 0.5f*m_phiSliceWidth;//the default sliding window along phi
 
  //1. loop over stages

  int currentStage = 0;
  unsigned int nConnections = 0;
  
  const FASTRACK_CONNECTOR& conn = *(m_settings.m_conn);

  std::vector<TrigFTF_GNN_Edge> edgeStorage;
  
  edgeStorage.reserve(MaxEdges);
  
  int nEdges = 0;

  for(std::map<int, std::vector<FASTRACK_CONNECTOR::LayerGroup> >::const_iterator it = conn.m_layerGroups.begin();it!=conn.m_layerGroups.end();++it, currentStage++) {
    
    //loop over L1 layers for the current stage

    for(const auto& layerGroup : (*it).second) {
      
      unsigned int dst = layerGroup.m_dst;//n1 : inner nodes
      
      const TrigFTF_GNN_Layer* pL1 = m_settings.m_geo->getTrigFTF_GNN_LayerByKey(dst);

      if (pL1==nullptr) {
	continue; 
      }

      for(const auto& conn : layerGroup.m_sources) {//loop over L2(L1) for the current stage

	unsigned int src = conn->m_src;//n2 : the new connectors

	const TrigFTF_GNN_Layer* pL2 = m_settings.m_geo->getTrigFTF_GNN_LayerByKey(src);

	if (pL2==nullptr) {
	  continue; 
	}

	int nDstBins = pL1->m_bins.size();
	int nSrcBins = pL2->m_bins.size();

	for(int b1=0;b1<nDstBins;b1++) {//loop over bins in Layer 1

	  TrigFTF_GNN_EtaBin& B1 = m_storage->getEtaBin(pL1->m_bins.at(b1));

	  if(B1.empty()) continue;

	  float rb1 = pL1->getMinBinRadius(b1);
	  
	  //3. loops over source eta-bins
	  
	  for(int b2=0;b2<nSrcBins;b2++) {//loop over bins in Layer 2
	  
	    if(m_settings.m_useEtaBinning && (nSrcBins+nDstBins > 2)) {
	      if(conn->m_binTable[b1 + b2*nDstBins] != 1) continue;//using precomputed LUT
	    }
	  
	    const TrigFTF_GNN_EtaBin& B2 = m_storage->getEtaBin(pL2->m_bins.at(b2));

	    if(B2.empty()) continue;

	    float rb2 = pL2->getMaxBinRadius(b2);

	    //calculated delta Phi for rb1 ---> rb2 extrapolation
	    
	    if(m_settings.m_useEtaBinning) {
	      deltaPhi = min_deltaPhi + m_maxCurv*std::fabs(rb2-rb1);
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
	      
		if(dr < m_minDeltaRadius) {
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
		
		if (m_settings.m_doubletFilterRZ) {
		  
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
		if(!isGood) continue;//no match found, skip creating [n1 <- n2] edge
		
		float dPhi2 = curv*r2;
                float dPhi1 = curv*r1;
		
		if(nEdges < MaxEdges) {

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

  if(nConnections == 0) return;

  const int maxIter = 15;

  int maxLevel = 0;

  int iter = 0;
  
  std::vector<TrigFTF_GNN_Edge*> v_old;
  
  for(int edgeIndex=0;edgeIndex<nEdges;edgeIndex++) {

    TrigFTF_GNN_Edge* pS = &(edgeStorage.at(edgeIndex));
    if(pS->m_nNei == 0) continue;
    v_old.push_back(pS);//TO-DO: increment level for segments as they already have at least one neighbour
  }

  for(;iter<maxIter;iter++) {

    //generate proposals
    std::vector<TrigFTF_GNN_Edge*> v_new;
    v_new.clear();

    for(auto pS : v_old) {
      
      int next_level = pS->m_level;
          
      for(int nIdx=0;nIdx<pS->m_nNei;nIdx++) {
            
        unsigned int nextEdgeIdx = pS->m_vNei[nIdx];
            
        TrigFTF_GNN_Edge* pN = &(edgeStorage.at(nextEdgeIdx));
            
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


  int minLevel = 3;//a triplet + 2 confirmation

  if(m_settings.m_LRTmode) {
    minLevel = 2;//a triplet + 1 confirmation
  }
  

  std::vector<TrigFTF_GNN_Edge*> vSeeds;

  vSeeds.reserve(MaxEdges/2);

  for(int edgeIndex=0;edgeIndex<nEdges;edgeIndex++) {
    TrigFTF_GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

    if(pS->m_level < minLevel) continue;

    vSeeds.push_back(pS);
  }

  m_triplets.clear();

  std::sort(vSeeds.begin(), vSeeds.end(), TrigFTF_GNN_Edge::CompareLevel());

  if(vSeeds.empty()) return;

  //backtracking

  TrigFTF_GNN_TRACKING_FILTER tFilter(m_settings.m_layerGeometry, edgeStorage);

  for(auto pS : vSeeds) {

    if(pS->m_level == -1) continue;

    TrigFTF_GNN_EDGE_STATE rs(false);
    
    tFilter.followTrack(pS, rs);

    if(!rs.m_initialized) {
      continue;
    }
    
    if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;

    std::vector<const TrigSiSpacePointBase*> vSP;
    
    for(std::vector<TrigFTF_GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {
            
      (*sIt)->m_level = -1;//mark as collected
      
      if(sIt == rs.m_vs.rbegin()) {
	vSP.push_back((*sIt)->m_n1);
      }
      vSP.push_back((*sIt)->m_n2);
    }

    if(vSP.size()<3) continue;

    //making triplets

    unsigned int nTriplets = 0;

    std::vector<TrigInDetTriplet> output;
    
    if(makeTriplets) {

      double maxCurv2 = m_maxCurv*m_maxCurv;
      
      for(unsigned int idx_m = 1;idx_m < vSP.size()-1;idx_m++) {

	const TrigSiSpacePointBase& spM = *vSP.at(idx_m);
	const double pS_r = spM.r();
	const double pS_x = spM.x();
	const double pS_y = spM.y();
	const double cosA = pS_x/pS_r;
	const double sinA = pS_y/pS_r;
	
	for(unsigned int idx_o = idx_m+1; idx_o < vSP.size(); idx_o++) {
	  
	  const TrigSiSpacePointBase& spO = *vSP.at(idx_o);
	  
	  double dx = spO.x() - pS_x;
	  double dy = spO.y() - pS_y;
	  double R2inv = 1.0/(dx*dx+dy*dy);
	  double xn = dx*cosA + dy*sinA;
	  double yn =-dx*sinA + dy*cosA;
	  
	  const double uo = xn*R2inv;
	  const double vo = yn*R2inv;
	  
	  for(unsigned int idx_i = 0; idx_i < idx_m; idx_i++) {
	    const TrigSiSpacePointBase& spI = *vSP.at(idx_i);
	    
	    dx = spI.x() - pS_x;
	    dy = spI.y() - pS_y;
	    R2inv = 1.0/(dx*dx+dy*dy);
	  
	    xn = dx*cosA + dy*sinA;
	    yn =-dx*sinA + dy*cosA;
    
	    const double ui = xn*R2inv;
	    const double vi = yn*R2inv;
	    
	    //1. pT estimate
    
	    const double du = uo - ui;
	    if(du==0.0) continue;
	    const double A = (vo - vi)/du;
	    const double B = vi - A*ui;
	    
	    const double triplet_curv2 = B*B/(1 + A*A);
	    
	    if(triplet_curv2 > maxCurv2) {
	      continue;
	    }
	    
	    //2. d0 cut
	    
	    const double fabs_d0 = std::abs(pS_r*(B*pS_r - A));
	    
	    if(fabs_d0 > m_settings.m_tripletD0Max) {
	      continue;
	    }
	    
	    //3. phi0 cut
	  
	    if (!roiDescriptor->isFullscan()) {
	      const double uc = 2*B*pS_r - A;
	      const double phi0 = std::atan2(sinA - uc*cosA, cosA + uc*sinA);
	      if ( !RoiUtil::containsPhi( *roiDescriptor, phi0 ) ) {
		continue;
	      }
	    }

	    //4. add new triplet

	    const double Q = fabs_d0*fabs_d0;

	    output.emplace_back(spI, spM, spO, Q);

	    nTriplets++;

	    if(nTriplets >= m_settings.m_maxTripletBufferLength) break;
	  }
	  if(nTriplets >= m_settings.m_maxTripletBufferLength) break;
	}
	if(nTriplets >= m_settings.m_maxTripletBufferLength) break;
      }
      if(output.empty()) continue;
    }
    
    vTracks.emplace_back(vSP, output);
  }

}

void TrigTrackSeedGeneratorITk::createSeeds(const IRoiDescriptor* roiDescriptor) {

  std::vector<GNN_TrigTracklet> vTracks;

  vTracks.reserve(5000);

  runGNN_TrackFinder(roiDescriptor, vTracks, true);

  if(vTracks.empty()) return;

  m_triplets.clear();

  for(auto& track : vTracks) {
    for(auto& seed : track.m_seeds) {

      float newQ = seed.Q();

      if (m_settings.m_LRTmode) {
	// In LRT mode penalize pixels in Triplets
	if(seed.s1().isPixel()) newQ+=1000;
	if(seed.s2().isPixel()) newQ+=1000;
	if(seed.s3().isPixel()) newQ+=1000;
      } else {
	// In normal (non LRT) mode penalise SSS by 1000, PSS (if enabled) and PPS by 10000
	if(seed.s3().isSCT()) {
	  newQ += seed.s1().isSCT() ? 1000.0 : 10000.0;
	} 
      }
      seed.Q(newQ);
      m_triplets.emplace_back(seed);
    }
  }
  vTracks.clear();

}

void TrigTrackSeedGeneratorITk::getTracklets(const IRoiDescriptor* roiDescriptor, std::vector<GNN_TrigTracklet>& vTracks, bool makeTriplets) {
  runGNN_TrackFinder(roiDescriptor, vTracks, makeTriplets);
}

void TrigTrackSeedGeneratorITk::createSeedsZv() {


}

void TrigTrackSeedGeneratorITk::getSeeds(std::vector<TrigInDetTriplet>& vs) {
  vs.clear();
  std::sort(m_triplets.begin(), m_triplets.end(), 
    [](const TrigInDetTriplet& A, const TrigInDetTriplet& B) {
      return A.Q() < B.Q();
    }
  );
  vs = std::move(m_triplets);
}


