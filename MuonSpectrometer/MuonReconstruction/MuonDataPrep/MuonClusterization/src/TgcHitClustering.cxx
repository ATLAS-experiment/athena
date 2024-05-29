/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonClusterization/TgcHitClustering.h"
namespace{
   using HitList = Muon::TgcHitClusteringObj::HitList;
   static const HitList dummy{};
}

namespace Muon {
    const HitList& TgcHitClusteringObj::bestEtaCluster() const {      
        return clustersEta.size() ? clustersEta.front() : dummy;
    }
    const HitList& TgcHitClusteringObj::bestPhiCluster() const {      
        return clustersPhi.size() ? clustersPhi.front() : dummy;
    }
    bool TgcHitClusteringObj::cluster(HitList& filteredHits,
                                      std::vector<HitList>& finalClusts) {
      if (filteredHits.empty()) return false;
      finalClusts.reserve(filteredHits.size());
      std::sort(filteredHits.begin(), filteredHits.end(), 
                [this](const TgcPrepData* h1,const TgcPrepData* h2) {
                  const int gap1{m_tgcIdHelper->gasGap(h1->identify())}, gap2{m_tgcIdHelper->gasGap(h2->identify())};
                  if (gap1 != gap2) return gap1 < gap2;
                  return m_tgcIdHelper->channel(h1->identify()) < m_tgcIdHelper->channel(h2->identify());
                });
      
      for (const TgcPrepData* prd : filteredHits) {
          /// Create new clusters if the cluster is more separated by more than one channel or if the gas gaps don't match
          if (finalClusts.empty() || 
              m_tgcIdHelper->channel(finalClusts.back().back()->identify()) +1 != m_tgcIdHelper->channel(prd->identify()) ||
              m_tgcIdHelper->gasGap(finalClusts.back().back()->identify()) != m_tgcIdHelper->gasGap(prd->identify())) {
              finalClusts.emplace_back();
          }  
          finalClusts.back().push_back(prd);
      }
      std::stable_sort(finalClusts.begin(),finalClusts.end(), 
                       []( const HitList& a, const HitList& b){
                            return a.size() < b.size();
                       });
      return true;
  }

  bool TgcHitClusteringObj::cluster( const std::vector<const TgcPrepData*>& col ){
    

    std::vector<const TgcPrepData*> etaHits{}, phiHits{};
    etaHits.reserve(col.size());
    phiHits.reserve(col.size());
    
    /// Split the hits in eta  & phi
    std::copy_if(col.begin(),col.end(), std::back_inserter(etaHits), [this](const TgcPrepData* prd) {
        return !m_tgcIdHelper->measuresPhi(prd->identify());
    });
    std::copy_if(col.begin(),col.end(), std::back_inserter(phiHits), [this](const TgcPrepData* prd){
        return m_tgcIdHelper->measuresPhi(prd->identify());
    });

    if (!cluster(etaHits, clustersEta)) return false;
    if (!cluster(phiHits, clustersPhi)) return false;
    
    return true;
  }
  bool TgcHitClusteringObj::buildClusters3D() {
    
    if( clustersPhi.empty() || clustersEta.empty() ) return false;
    
    const TgcPrepData* etaHit = bestEtaCluster().front(); 

    const MuonGM::TgcReadoutElement* detEl = etaHit->detectorElement();

    // now loop over eta and phi clusters and form space points
    for(HitList& eit : clustersEta) {      

        const TgcPrepData* firstEta = eit.front();
        const TgcPrepData* lastEta = eit.back();
      
        for(HitList& pit : clustersPhi) {

          const TgcPrepData* firstPhi = pit.front();
          const TgcPrepData* lastPhi = pit.back();

          TgcClusterObj3D cl3D{eit, pit};
          using Edge = TgcClusterObj3D::Edge;

          detEl->spacePointPosition( firstPhi->identify(), firstEta->identify(), cl3D.getEdge(Edge::LowEtaLowPhi) );
          if( lastPhi != firstPhi ) { 
            detEl->spacePointPosition( lastPhi->identify(), firstEta->identify(), cl3D.getEdge(Edge::LowEtaHighPhi)); 
          } else {
            cl3D.getEdge(Edge::LowEtaHighPhi) = cl3D.getEdge(Edge::LowEtaLowPhi);
          }
          if( lastEta != firstEta ) {
            detEl->spacePointPosition( firstPhi->identify(),lastEta->identify(),  cl3D.getEdge(Edge::HighEtaLowPhi) );
            if( lastPhi != firstPhi ) { 
              detEl->spacePointPosition( lastPhi->identify(),lastEta->identify(), cl3D.getEdge(Edge::HighEtaHighPhi) ); 
            } else { 
              cl3D.getEdge(Edge::HighEtaHighPhi) = cl3D.getEdge(Edge::HighEtaLowPhi);
            }
          }else{
            cl3D.getEdge(Edge::HighEtaLowPhi) = cl3D.getEdge(Edge::LowEtaLowPhi);
            cl3D.getEdge(Edge::HighEtaHighPhi) = cl3D.getEdge(Edge::LowEtaHighPhi);
          }
          clusters3D.push_back(std::move(cl3D));
      }
    }
    return true;
  }

 

}
