/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "TrkSurfaces/Surface.h"
#include "TrkExUtils/ExtrapolationCache.h"
#include "TrkExTools/LocalExtrapolatorCache.h"

namespace {
   inline void setMaxAtomic(unsigned int new_value, std::atomic<unsigned int> &destination) {
      for(;;) {
         unsigned int is_value = destination;
         unsigned int max_value = std::max(is_value,new_value);
         if (max_value == is_value) break;
         else if (destination.compare_exchange_weak(is_value,max_value)) { break; }
      }
   }
}
namespace Trk{

  Cache::Cache(Dbg::PropStat &stat)
  : m_ownedPtrs(128)
  , m_statPtr(&stat){
    m_navigSurfs.reserve(1024);
    m_navigVols.reserve(64);
    m_navigVolsInt.reserve(64);
   }

  Cache::Cache(const std::vector<const IMaterialEffectsUpdator*> & updaters)
  : m_ownedPtrs(128){
    m_navigSurfs.reserve(1024);
    m_navigVols.reserve(64);
    m_navigVolsInt.reserve(64);
    populateMatEffUpdatorCache(updaters);
  }

  Cache::~Cache(){
     if (m_statPtr) {
        if (m_recursionCount[kMaxRecursionCount]>10) {
           // only consider cases with some depth
           setMaxAtomic(m_recursionCount[kMaxRecursionCount], m_statPtr->m_maxRecursionCount);
        }
        if (m_nPropagations > 200 ) {
           // only consider cases with some propagations.
           setMaxAtomic(m_nPropagations, m_statPtr->m_maxPropagations);
        }
        if (m_methodSequence>100 ) {
           setMaxAtomic(m_methodSequence, m_statPtr->m_maxMethodSequence);
        }
     }
  }

  IMaterialEffectsUpdator::ICache&
  Cache::subMaterialEffectsUpdatorCache( const TrackingVolume& tvol){
    return m_MaterialUpCache[tvol.geometrySignature()];
  }

  IMaterialEffectsUpdator::ICache&
  Cache::subMaterialEffectsUpdatorCache() {
    return m_MaterialUpCache[m_currentStatic->geometrySignature()];
  }

  void
  Cache::populateMatEffUpdatorCache(const std::vector<const IMaterialEffectsUpdator*> & updaters) {
    const size_t numUpdaters = updaters.size();
    m_MaterialUpCache.reserve(numUpdaters);
    for (const auto & thisUpdater : updaters) {
      m_MaterialUpCache.emplace_back(thisUpdater->getCache());
    }
  }

  void
  Cache::setRecallInformation(const Surface& rsf,const Layer& rlay,const TrackingVolume& rvol) {
    m_recallSurface = &rsf;
    m_recallLayer = &rlay;
    m_recallTrackingVolume = &rvol;
  }

  void
  Cache::resetRecallInformation() {
    m_recallSurface = nullptr;
    m_recallLayer = nullptr;
    m_recallTrackingVolume = nullptr;
  }

  std::string
  Cache::to_string(const std::string& txt) const{
    std::string result;
    if (elossPointerOverwritten()) {
      result = elossPointerErrorMsg();
    } else {
      result = txt + " X0 " +std::to_string(m_extrapolationCache->x0tot())  + " Eloss deltaE "
                      + std::to_string(m_extrapolationCache->eloss()->deltaE()) + " Eloss sigma "
                      +  std::to_string(m_extrapolationCache->eloss()->sigmaDeltaE()) + " meanIoni "
                      + std::to_string(m_extrapolationCache->eloss()->meanIoni()) + " sigmaIoni "
                      + std::to_string(m_extrapolationCache->eloss()->sigmaIoni()) + " meanRad "
                      + std::to_string(m_extrapolationCache->eloss()->meanRad()) + " sigmaRad "
                      + std::to_string(m_extrapolationCache->eloss()->sigmaRad());
    }
    return result;
  }


  bool
  Cache::elossPointerOverwritten() const{
    return (m_cacheEloss != nullptr && m_cacheEloss != m_extrapolationCache->eloss());
  }

  std::string
  Cache::elossPointerErrorMsg(int lineNumber) const{
  std::string result;
  if (lineNumber !=0) result = "Line " + std::to_string(lineNumber)+": ";
  result += " PROBLEM Eloss cache pointer overwritten " + std::to_string(reinterpret_cast<std::uintptr_t>(m_cacheEloss))
                        + " from extrapolationCache " + std::to_string(reinterpret_cast<std::uintptr_t>(m_extrapolationCache->eloss()));
  return result;
  }

  void
  Cache::retrieveBoundaries(){
   m_staticBoundaries.clear();
   const auto& bounds = m_currentStatic->boundarySurfaces();
   for (size_t ib=0; ib< bounds.size(); ++ib){
     const Trk::Surface& surf = bounds[ib]->surfaceRepresentation();
     m_staticBoundaries.emplace_back(&surf, true);
   }
  }

  void
  Cache::addOneNavigationLayer(const Trk::TrackingVolume* pDetVol, const Trk::Layer* pLayer, bool boundaryCheck){
    m_layers.emplace_back(&(pLayer->surfaceRepresentation()), boundaryCheck);
    m_navigLays.emplace_back(pDetVol, pLayer);
  }

  void
  Cache::addOneNavigationLayer(const Trk::Layer* pLayer, bool boundaryCheck){
    m_layers.emplace_back(&(pLayer->surfaceRepresentation()), boundaryCheck);
    m_navigLays.emplace_back(m_currentStatic, pLayer);
  }

  void
  Cache::copyToNavigationSurfaces(){
    if (!m_layers.empty()) {
      m_navigSurfs.insert(m_navigSurfs.end(), m_layers.begin(), m_layers.end());
    }
    if (!m_denseBoundaries.empty()) {
      m_navigSurfs.insert(m_navigSurfs.end(), m_denseBoundaries.begin(), m_denseBoundaries.end());
    }
    if (!m_navigBoundaries.empty()) {
      m_navigSurfs.insert(m_navigSurfs.end(), m_navigBoundaries.begin(), m_navigBoundaries.end());
    }
    if (!m_detachedBoundaries.empty()) {
      m_navigSurfs.insert(m_navigSurfs.end(),m_detachedBoundaries.begin(),m_detachedBoundaries.end());
    }
  }


}
