/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_IPP
#define TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_IPP

#include "InDetPrepRawData/PixelCluster.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"

#include "TrigInDetPattRecoEvent/TrigInDetSiLayer.h"
#include "GNN_Geometry.h"

#include<cmath>
#include<cstring>
#include<algorithm>

template<typename externalSP>
TrigFTF_GNN_EtaBin<externalSP>::TrigFTF_GNN_EtaBin(): m_minRadius(0), m_maxRadius(0) {

  m_in.clear();
  m_vn.clear();
  m_params.clear();
  m_vn.reserve(1000);
}

template<typename externalSP>
TrigFTF_GNN_EtaBin<externalSP>::~TrigFTF_GNN_EtaBin() {
  m_in.clear();
  m_vn.clear();
  m_params.clear();
}

template<typename externalSP>
void TrigFTF_GNN_EtaBin<externalSP>::sortByPhi() {
  
  std::vector<std::pair<float, const TrigFTF_GNN_Node<externalSP>*> > phiBuckets[32];

  int nBuckets = 31;

  for(const auto& n : m_vn) {

    int bIdx = (int)(0.5*nBuckets*(n->phi()/(float)M_PI + 1.0f));
    phiBuckets[bIdx].push_back(std::make_pair(n->phi(), n));
  }

  for(auto& b : phiBuckets) {
    std::sort(b.begin(), b.end());
  }

  int idx = 0;
  for(const auto& b : phiBuckets) {
    for(const auto& p : b) {
      m_vn[idx++] = p.second;
    }
  }

}

template<typename externalSP>
void TrigFTF_GNN_EtaBin<externalSP>::initializeNodes() {
  if(m_vn.size() ==0) return;
  
  m_params.resize(m_vn.size());
  
  m_in.resize(m_vn.size());
  
  for(unsigned int nIdx=0;nIdx<m_vn.size();nIdx++) {
    m_in[nIdx].reserve(50);//reasonably high number of incoming edges per node
    m_params[nIdx][0] = -100.0;//default cut on cot(theta)
    m_params[nIdx][1] = 100.0; //default cut on cot(theta)
    const TrigFTF_GNN_Node<externalSP>* pN = m_vn.at(nIdx);
    m_params[nIdx][2] = pN->phi();
    m_params[nIdx][3] = pN->r();
    m_params[nIdx][4] = pN->z();
  }
  
  auto [min_iter, max_iter] = std::minmax_element(m_vn.begin(), m_vn.end(),
						  [](const TrigFTF_GNN_Node<externalSP>* s, const TrigFTF_GNN_Node<externalSP>* s1) { return (s->r() < s1->r()); });
  m_maxRadius = (*max_iter)->r();
  m_minRadius = (*min_iter)->r();
}

template<typename externalSP>
void TrigFTF_GNN_EtaBin<externalSP>::generatePhiIndexing(float dphi) {

  for(unsigned int nIdx=0;nIdx<m_vn.size();nIdx++) {

    float phi = m_params[nIdx][2];
    if(phi <= M_PI-dphi) continue;    
    m_vPhiNodes.push_back(std::pair<float, unsigned int>(phi - 2*M_PI, nIdx));
    
  }

  for(unsigned int nIdx=0;nIdx<m_vn.size();nIdx++) {
    float phi = m_params[nIdx][2];
    m_vPhiNodes.push_back(std::pair<float, unsigned int>(phi, nIdx));
  }

  for(unsigned int nIdx=0;nIdx<m_vn.size();nIdx++) {

    float phi = m_params[nIdx][2];
    if(phi >= -M_PI + dphi) break;
    m_vPhiNodes.push_back(std::pair<float, unsigned int>(phi + 2*M_PI, nIdx));
  }
  
}

template<typename externalSP>
TrigFTF_GNN_DataStorage<externalSP>::TrigFTF_GNN_DataStorage(const TrigFTF_GNN_Geometry& g) : m_geo(g) {
  m_etaBins.resize(g.num_bins());
}

template<typename externalSP>
TrigFTF_GNN_DataStorage<externalSP>::~TrigFTF_GNN_DataStorage() {

}

template<typename externalSP>
int TrigFTF_GNN_DataStorage<externalSP>::loadPixelGraphNodes(short layerIndex, const std::vector<TrigFTF_GNN_Node<externalSP>>& coll, bool useML) {

  int nLoaded = 0;

  const TrigFTF_GNN_Layer* pL = m_geo.getTrigFTF_GNN_LayerByIndex(layerIndex);

  if(pL == nullptr) {
    return -1;
  }
  
  bool isBarrel = (pL->m_layer.m_type == 0);
 
  for(const auto& node : coll) {

    int binIndex = pL->getEtaBin(node.z(), node.r());

    if(binIndex == -1) {
      continue;
    }
    
    if(isBarrel) {
      m_etaBins.at(binIndex).m_vn.push_back(&node);
    }
    else {
      if (useML) {
	float cluster_width = node.pixelClusterWidth();
	if(cluster_width > 0.2) continue;
      }
      m_etaBins.at(binIndex).m_vn.push_back(&node);
    }
    
    nLoaded++;
    
  }
  
  return nLoaded;
}

template<typename externalSP>
int TrigFTF_GNN_DataStorage<externalSP>::loadStripGraphNodes(short layerIndex, const std::vector<TrigFTF_GNN_Node<externalSP>>& coll) {

  int nLoaded = 0;

  const TrigFTF_GNN_Layer* pL = m_geo.getTrigFTF_GNN_LayerByIndex(layerIndex);

  if(pL == nullptr) {
    return -1;
  }
   
  for(const auto& node : coll) {

    int binIndex = pL->getEtaBin(node.z(), node.r());

    if(binIndex == -1) {
      continue;
    }

    m_etaBins.at(binIndex).m_vn.push_back(&node);
    nLoaded++;
  }
  
  return nLoaded;
}

template<typename externalSP>
unsigned int TrigFTF_GNN_DataStorage<externalSP>::numberOfNodes() const {

  unsigned int n=0;
  
  for(const auto& b : m_etaBins) {
    n += b.m_vn.size();
  }
  return n;
}

template<typename externalSP>
void TrigFTF_GNN_DataStorage<externalSP>::sortByPhi() {
    
  for(auto& b : m_etaBins) b.sortByPhi();
}

template<typename externalSP>
void TrigFTF_GNN_DataStorage<externalSP>::initializeNodes(bool useML) {
  
  for(auto& b : m_etaBins) {
    b.initializeNodes();
  }
  
  if(!useML) return;
  
  unsigned int nL = m_geo.num_layers();

  for(unsigned int layerIdx=0;layerIdx<nL;layerIdx++) {

    const TrigFTF_GNN_Layer* pL = m_geo.getTrigFTF_GNN_LayerByIndex(layerIdx);

    if(pL->m_layer.m_subdet < 20000) {//skip strips volumes: layers in range [1200X-1400X]
      continue;
    }
    
    bool isBarrel = (pL->m_layer.m_type == 0);

    if(!isBarrel) continue;

    int nBins = pL->m_bins.size();

    for(int b=0;b<nBins;b++) {//loop over eta-bins in Layer

      TrigFTF_GNN_EtaBin<externalSP>& B = m_etaBins.at(pL->m_bins.at(b));

      if(B.empty()) continue;
      
      for(unsigned int nIdx=0;nIdx<B.m_vn.size();nIdx++) {
        float cluster_width = B.m_vn[nIdx]->pixelClusterWidth();
	//adjusting cuts using fitted boundaries of |cot(theta)| vs. cluster z-width distribution 
        float min_tau = 6.7*(cluster_width - 0.2);//linear fit
        float max_tau = 1.6 + 0.15/(cluster_width + 0.2) + 6.1*(cluster_width - 0.2);//linear fit + correction for short clusters

        B.m_params[nIdx][0] = min_tau;
        B.m_params[nIdx][1] = max_tau;
        
      }
    }
  }
}

template<typename externalSP>
void TrigFTF_GNN_DataStorage<externalSP>::generatePhiIndexing(float dphi) {
  for(auto& b : m_etaBins) b.generatePhiIndexing(dphi);
}

#endif
