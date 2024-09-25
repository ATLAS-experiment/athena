/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkParticleCreator/DetailedHitInfo.h"
#include "CxxUtils/checker_macros.h"
#include <numeric>

namespace{
  
  template<typename InputIterator,typename AccumulateType,typename BinaryOperation,typename Predicate>
  const AccumulateType 
  accumulate_if(InputIterator first,const InputIterator last,
    AccumulateType init, BinaryOperation&& binary_op,Predicate&& predicate){
    for (; first != last; ++first){
      if (predicate(*first)) init = binary_op(init, *first);
    }
    return init;
  }
  
  enum TupleItem{ ThisLayer, NumLayers, NumHits};
  
  using OneDetail = std::tuple<int, int, int>;
  using DetailVector = std::vector<OneDetail>;
  
  template<int q>
  int addUp(int l, const std::pair < Trk::DetectorRegion, DetailVector > & p){
    for (const auto &t:p.second){
      l+=std::get<q>(t);
    }
    return l;
  }
  
  auto fromRegion=[](Trk::DetectorRegion r){
    return [r](const std::pair < Trk::DetectorRegion, DetailVector > & p){
      return p.first == r;
    };
  };
  
  auto fromStrips ATLAS_THREAD_SAFE =[](const std::pair < Trk::DetectorRegion, DetailVector > & p) {
    return (p.first==Trk::DetectorRegion::stripBarrel) or (p.first==Trk::DetectorRegion::stripEndcap);
  };

  auto fromPixels ATLAS_THREAD_SAFE =[](const std::pair < Trk::DetectorRegion, DetailVector > & p) {
    return (p.first==Trk::DetectorRegion::pixelBarrelFlat) or 
      (p.first==Trk::DetectorRegion::pixelBarrelInclined) or 
      (p.first==Trk::DetectorRegion::pixelEndcap);
  };
}



const std::vector < std::pair < Trk::DetectorRegion, std::vector < std::tuple <int , int , int> > > > &
Trk::DetailedHitInfo::getHitInfo(){
  return m_detailedHitInfo;
}

void 
Trk::DetailedHitInfo::addHit(Trk::DetectorRegion region, int layer, int eta_module, int hit) {
  for (auto& [thisRegion,thisHitInfoVector] : m_detailedHitInfo) {
    if (thisRegion==region) {
      for (auto& [thisLayer, nLayers, nHits] : thisHitInfoVector) {
        if (thisLayer==layer) {
          // increase the contributing layers only for inclined/endcap modules
          if (region!=pixelBarrelFlat and region!=stripBarrel and eta_module!=m_prevEta) { 
            nLayers++;
            m_prevEta = eta_module;
          }
          nHits+=hit;
          return;
        }
      }
    }
  }
  // if reaches this point the element is not found, then you add it to m_detailedHitInfo  
  m_prevEta = eta_module;
  std::vector < std::tuple <int, int, int> > counts = {{layer, 1, hit}};
  m_detailedHitInfo.emplace_back(region, counts);  
}

int 
Trk::DetailedHitInfo::getHits(Trk::DetectorRegion region, int layer){
  for (auto& [thisRegion,thisHitInfoVector] : m_detailedHitInfo) {
    if (thisRegion==region) {
      for (auto& [thisLayer, nLayers, nHits] : thisHitInfoVector) {
        if (thisLayer==layer) {
          return nHits;
        }
      }
    }
  }
  return 0;  
}

int 
Trk::DetailedHitInfo::getContributionFromRegion(Trk::DetectorRegion region){
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumLayers>, fromRegion(region));
}

int 
Trk::DetailedHitInfo::getAllContributions(){
  return std::accumulate(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumLayers>);
}

int 
Trk::DetailedHitInfo::getHitsFromRegion(Trk::DetectorRegion region){
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumHits>, fromRegion(region));
}

int 
Trk::DetailedHitInfo::getAllHits(){
  return std::accumulate(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumHits>);
}

int 
Trk::DetailedHitInfo::getPixelContributions() {
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumLayers>, fromPixels);
}

int 
Trk::DetailedHitInfo::getPixelHits() {
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumHits>, fromPixels);

}

int 
Trk::DetailedHitInfo::getStripContributions() {
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumLayers>, fromStrips);
}

int 
Trk::DetailedHitInfo::getStripHits() {
  return accumulate_if(m_detailedHitInfo.begin(), m_detailedHitInfo.end(), 0, addUp<NumHits>, fromStrips);
}
