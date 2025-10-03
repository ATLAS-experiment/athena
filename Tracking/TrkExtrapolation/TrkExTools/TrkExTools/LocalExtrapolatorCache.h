/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRKEXTOOLS_LOCALEXCACHE_H
#define TRKEXTOOLS_LOCALEXCACHE_H
#include "ExtrUniquePtrHolder.h"
#include "TrkParameters/TrackParameters.h"
#include "ParametersNextVolume.h"
#include "TrkExInterfaces/IMaterialEffectsUpdator.h"
#include "TrkGeometry/TrackingGeometry.h" //because of m_trackingGeometry-> in header
#include "TrkExInterfaces/INavigator.h"  //using navigator. in this header
#include <utility>
#include <vector>
#include <string>
#include <memory>

 namespace Trk{
   class Surface;
   class ExtrapolationCache;
   class EnergyLoss;
   class TrackStateOnSurface;
 }

namespace Dbg {
   // counters to gather statistics for abort conditions
   struct PropStat {
      std::atomic<unsigned int> m_maxRecursionCount{};
      std::atomic<unsigned int> m_maxPropagations{};
      std::atomic<unsigned int> m_maxMethodSequence{};
   };
}

namespace Trk{
struct Cache
  {
    using TrackParametersUVector = std::vector<std::unique_ptr<Trk::TrackParameters>>;
    using identifiedParameters_t = std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>>;
    using DestSurf = std::pair<const Surface*, BoundaryCheck>;
    //!< The class holding the unique ptr during the extrapolation loop.
    Trk::ExtrUniquePtrHolder<Trk::TrackParameters> m_ownedPtrs;
    //!< parameters to be used for final propagation in case of fallback
    Trk::TrackParameters* m_lastValidParameters = nullptr;
    //!< return helper for parameters and boundary
    ParametersNextVolume m_parametersAtBoundary{};
    //!< Caches per MaterialUpdator
    std::vector<Trk::IMaterialEffectsUpdator::ICache> m_MaterialUpCache;
    //!<  internal switch for resolved configuration
    bool m_dense = false;
    unsigned int m_layerResolved{};
    unsigned int m_methodSequence = 0;
    const Surface* m_destinationSurface = nullptr;
    //!< the boundary volume check
    const Volume* m_boundaryVolume = nullptr;
    //!< Destination Surface for recall
    const Surface* m_recallSurface = nullptr;
    //!< Destination Layer for recall
    const Layer* m_recallLayer = nullptr;
    //!< Destination TrackingVolume for recall
    const TrackingVolume* m_recallTrackingVolume = nullptr;
    const Trk::TrackingVolume* m_currentStatic = nullptr;
    const Trk::TrackingVolume* m_currentDense = nullptr;
    const Trk::TrackingVolume* m_highestVolume = nullptr;
    //Tracking Geometry ptr
    const Trk::TrackingGeometry *m_trackingGeometry = nullptr;
    //path
    double m_path{};
    //!< Pointer (not owning) pointing
    //to a vector of unique parameters of detector elements
    TrackParametersUVector* m_parametersOnDetElements = nullptr;
    //!< cache layer with last material update
    bool m_cacheLastMatLayer = false;
    const Layer* m_lastMaterialLayer = nullptr;
    //!< cache for collecting the total X0 ans Eloss
    Trk::ExtrapolationCache* m_extrapolationCache = nullptr;
    //!< cache pointer for Eloss
    const Trk::EnergyLoss* m_cacheEloss = nullptr;
    //!< cache of TrackStateOnSurfaces
    std::vector<const Trk::TrackStateOnSurface*>* m_matstates = nullptr;
    // for active volumes
    std::unique_ptr<identifiedParameters_t> m_identifiedParameters{};
    //
    std::pair<unsigned int, unsigned int> m_denseResolved{};
    //
    std::vector<DestSurf> m_staticBoundaries{};
    std::vector<DestSurf> m_detachedBoundaries{};
    std::vector<DestSurf> m_denseBoundaries{};
    std::vector<DestSurf> m_navigBoundaries{};
    std::vector<DestSurf> m_layers{};
    //
    std::vector<std::pair<const Trk::DetachedTrackingVolume*, unsigned int>> m_detachedVols{};
    std::vector<std::pair<const Trk::TrackingVolume*, unsigned int>> m_denseVols{};
    std::vector<std::pair<const Trk::TrackingVolume*, const Trk::Layer*>> m_navigLays{};
    std::vector<std::pair<const Trk::Surface*, Trk::BoundaryCheck>> m_navigSurfs{};
    std::vector<const Trk::DetachedTrackingVolume*> m_navigVols{};
    std::vector<std::pair<const Trk::TrackingVolume*, unsigned int>> m_navigVolsInt{};

    // To gather statistics to tune abort condition based for to large call depth, or too many propagations
    Dbg::PropStat *m_statPtr=nullptr;
    enum ERecursionValues {kCurrentRecursionCount,kMaxRecursionCount, kNRecursionValues};
    std::array<unsigned short,kNRecursionValues> m_recursionCount {}; // current-recursion-level, max
    unsigned int m_nPropagations {};
    enum EStatus {kContinue, kRecursionCountExceeded} m_status=kContinue;

    //methods
    Cache(Dbg::PropStat &stat);
    ~Cache();
    Cache(const std::vector<const IMaterialEffectsUpdator*> & updaters);

    void setTrackingGeometry(const Trk::INavigator& navigator,
                             const EventContext& ctx) {
      if (!m_trackingGeometry) {
        m_trackingGeometry = navigator.trackingGeometry(ctx);
      }
    }

    const Trk::TrackingVolume* volume(const EventContext&,
                                      const Amg::Vector3D& gp) const {
      assert(m_trackingGeometry);
      return m_trackingGeometry->lowestTrackingVolume(gp);
    }

    /** Get the IMaterialEffectsUpdator::ICache  for the MaterialEffectsUpdator*/
    IMaterialEffectsUpdator::ICache&
    subMaterialEffectsUpdatorCache(const TrackingVolume& tvol) ;

    IMaterialEffectsUpdator::ICache&
    subMaterialEffectsUpdatorCache() ;

    //
    void
    populateMatEffUpdatorCache(const std::vector<const IMaterialEffectsUpdator*> & updaters);

     /** Private method for setting recall Information */
    void
    setRecallInformation(const Surface&,const Layer&,const TrackingVolume&);

    void
    resetRecallInformation();

    ///String representation of cache
    std::string
    to_string(const std::string& txt) const;

    ///Check cache integrity
    bool
    elossPointerOverwritten() const;

    ///String error message if the cache has a problem
    std::string
    elossPointerErrorMsg(int lineNumber=0) const;

    ///Retrieve boundaries
    void
    retrieveBoundaries();

    ///Add one layer and navigLayer
    void
    addOneNavigationLayer(const Trk::TrackingVolume* pDetVol, const Trk::Layer* pLayer, bool boundaryCheck=true);

    ///Add one layer and navigLayer using the current static vol
    void
    addOneNavigationLayer(const Trk::Layer* pLayer, bool boundaryCheck=true);

    ///Insert navigation surfaces from layers, dense boundaries, navig boundaries and detached boundaries
    void
    copyToNavigationSurfaces();
  };
  }
  #endif


