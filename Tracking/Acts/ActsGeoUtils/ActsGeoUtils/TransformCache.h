/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsGeoUtils_TransformCache_H
#define ActsGeoUtils_TransformCache_H

#include <shared_mutex>
#include <ActsGeoUtils/Defs.h>
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include <Identifier/IdentifierHash.h>
#include <CxxUtils/CachedUniquePtr.h>

namespace ActsTrk {

  class TransformCache {
    public:
      /** @brief: Functional to initialize the cache for a given alignment. Ideally, this functional is defined
       *          by each of the MuonReadoutElement and calls the protected
       * 
       *              Amg::Transform3D toStation(ActsTrk::RawGeomAlignStore* alignStore) const
       *
       *          method which is applying the alignment transformations via the GeoModel kernel. The IdentifierHash
       *          encodes the active sensor number within the MuonReadoutElement. Hence, it  can be used 
       *          to find the transformation to go from the origin of  the GeoModel detector coordinate system 
       *          to the origin of the sensor coordinate system.
      */
      using TransformMaker = std::function<Amg::Transform3D(const ActsTrk::DetectorAlignStore* store, const IdentifierHash& hash)>;

      /** @brief: Standard constructor taking the hash of the sensor element and 
       *          and the TransformMaker expressed usually as a lambda function
      **/
      TransformCache(const IdentifierHash& hash, 
                     TransformMaker maker,
                     const IDetectorElement* parentEle);
      
      TransformCache(const IdentifierHash& hash, 
                     std::shared_ptr<const TransformMaker> maker,
                     const IDetectorElement* parentEle);
      /** @brief: Delete copy constructor & assignment operator*/
      TransformCache(const TransformCache& other) = delete;
      TransformCache& operator=(const TransformCache& other) = delete;
      /** @brief Destructor releasing the ticket for later reuse*/
      ~TransformCache();
      
      /** @brief Returns the matching transformation from the alignment store. 
       *         If a nullptr is given, then it's equivalent to the case that the transformation
       *         is pointing to a perfectly aligned surface. In this case, the internal nominal
       *         transformation cache is invoked.
       * */
      const Amg::Transform3D& getTransform(const DetectorAlignStore* store) const;
      /** @brief Returns the sensor hash of this transformation cache */
      IdentifierHash hash() const;
      /** @brief Returns the transform maker function of this transformation cache*/
      const TransformMaker& transformMaker() const;
      /** @brief Returns the parent IDetectorElement owning the cache*/
      const IDetectorElement* parent() const;
      /** @brief resets the nominal cache associated with the detector element*/
      void releaseNominalCache() const;
    
    private:
       const IDetectorElement* m_parent{nullptr};
       const DetectorType m_type{m_parent->detectorType()};
       using TicketCounter = DetectorAlignStore::TrackingAlignStore;
       const unsigned int m_clientNo{TicketCounter::drawTicket(m_type)};
       IdentifierHash m_hash{0};
       std::shared_ptr<const TransformMaker> m_transform{};
       mutable std::shared_mutex m_mutex ATLAS_THREAD_SAFE{};
       mutable CxxUtils::CachedUniquePtrT<Amg::Transform3D> m_nomCache ATLAS_THREAD_SAFE{};
  };

inline bool operator<(const std::unique_ptr<TransformCache>& a,
                      const std::unique_ptr<TransformCache>& b) {
    return a->hash() < b->hash();
}
inline bool operator<(const IdentifierHash& a,
                      const std::unique_ptr<TransformCache>& b) {
    return a < b->hash();
}
inline bool operator<(const std::unique_ptr<TransformCache>& a,
                      const IdentifierHash& b) {
    return a->hash() < b;
}
using TransformCacheSet = std::set<std::unique_ptr<TransformCache>, std::less<>>;
}
#include <ActsGeoUtils/TransformCache.icc>
#endif