/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsGeoUtils_TransformCache_H
#define ActsGeoUtils_TransformCache_H

#include <shared_mutex>
#include <type_traits>
#include <ActsGeoUtils/Defs.h>
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include <Identifier/IdentifierHash.h>
#include <CxxUtils/CachedUniquePtr.h>

namespace ActsTrk {
  /*** @brief: The TransformCache holds the local -> global transformations associated with a tracking surface 
   *           from the Readout geometry. The cache establishes the connection with the GeometryContext or more
   *           precisely with its DetectorAlignStore to provide the transformations of an aligned surface. 
   *           As soon as the alignment store is accessed, the nominal surface is released from memory.
   *           In order to be used for each detector technology, the virtual <fetchTransform> needs to
   *           be defined further downstream. The method is called everytime when a new cache is invoked. */
  class TransformCache {
      public:
          TransformCache(const IdentifierHash& cacheHash,
                         const DetectorType type);
          
          TransformCache(const TransformCache& other) = delete;
          TransformCache& operator=(const TransformCache& other) = delete;

          virtual ~TransformCache();


          /** @brief Returns the Identifier of the transform cache */
          virtual Identifier identify() const = 0;
          /** @brief Returns the sensor hash of this transformation cache */
          IdentifierHash hash() const;
          /** @brief Returns the parent IDetectorElement owning the cache*/
          virtual const IDetectorElement* parent() const = 0;
          
          /** @brief Returns the matching transformation from the alignment store. 
            *        If a nullptr is given, then it's equivalent to the case that the transformation
            *        is pointing to a perfectly aligned surface. In this case, the internal nominal
            *        transformation cache is invoked.
            * */
          const Amg::Transform3D& getTransform(const DetectorAlignStore* store) const;

#ifndef SIMULATIONBASE
          /** @brief returns the cached transform from the Acts Geometry context */
          const Amg::Transform3D& transform(const Acts::GeometryContext& gctx) const;
#endif
          /** @brief resets the nominal cache associated with the detector element*/
          void releaseNominalCache() const;
          /** @brief returns the detector type of the cache*/
          DetectorType detectorType() const;
      protected:
          virtual Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const = 0;
      private:
          const IdentifierHash m_hash{0};
          const DetectorType m_type{DetectorType::UnDefined};
          using TicketCounter = DetectorAlignStore::TrackingAlignStore;
          const unsigned int m_clientNo{TicketCounter::drawTicket(m_type)};
          mutable std::shared_mutex m_mutex ATLAS_THREAD_SAFE{};
          mutable CxxUtils::CachedUniquePtrT<Amg::Transform3D> m_nomCache ATLAS_THREAD_SAFE{};
  };



  template<typename CachingDetectorEle> 
  class TransformCacheDetEle: public TransformCache {
    public:
      /** @brief: Standard constructor taking the hash of the sensor element and 
       *          and the TransformMaker expressed usually as a lambda function
      **/
      TransformCacheDetEle(const IdentifierHash& hash, 
                           const CachingDetectorEle* parentEle);
      
      const IDetectorElement* parent() const override final;
      Identifier identify() const override final;
    private:
       Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const override final;
       const CachingDetectorEle* m_parent{nullptr};
  };

}
#include <ActsGeoUtils/TransformCache.icc>
#endif