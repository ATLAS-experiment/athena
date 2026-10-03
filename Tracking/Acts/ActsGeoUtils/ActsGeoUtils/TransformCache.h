/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsGeoUtils_TransformCache_H
#define ActsGeoUtils_TransformCache_H

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
///
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include <Identifier/IdentifierHash.h>
#include <CxxUtils/CachedUniquePtr.h>

// Forward declaration
namespace Acts {
    class Surface;
}
namespace ActsTrk{
  class SurfacePlacement;  
}


namespace ActsTrk {
  /*** @brief The AlignableGeoPositioning manages the alignable transform handling of a surface from the 
   *          readout geometry, of a volume portal surface or of a volume. If the transform belongs to
   *          a surface, it always represents the transform from the local surface frame to the global
   *          ATLAS frame. In the case of volumes, both directions are possible.
   *
   *          The cache usually interacts with the @ref DetectorAlignStore to forward the aligned 
   *          transform to the requesting client. In case that there is no @DetectorAlignStore passed
   *          the class also holds an internal cache for a transform which is lazily populated with the
   *          nominal transform if needed, but also immediatley deleted as soon as a new DetectorAlignStore
   *          instance is shown to the class. If the refernce to the cached transform is queried via
   *
   *               const Amg::Isometry3D& getTransform(const DetectorAlignStore* store) const;
   * 
   *          it is checked whether the (internal) store already hold the transform. If not, the asembly
   *          of the transform is queried via calling
   *
   *                     Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const
   *
   *          which  is a pure interface function and needs to be implemented downstream */
  class AlignableGeoPositioning {
      public:
          /** @brief Standard constrcutor
           *  @param cacheHash: Identifier hash which may be used to identify which transform shall
           *                    be constructed 
           *  @param type: The ATLAS subdetector type defining in which subsystem's detector 
           *               align store the transform is written */
          explicit AlignableGeoPositioning(const IdentifierHash& cacheHash,
                                           const DetectorType type);
          /** @brief Delete the copy constructor */
          AlignableGeoPositioning(const AlignableGeoPositioning& other) noexcept = delete;
          /** @brief Delete the copy assignment operator */
          AlignableGeoPositioning& operator=(const AlignableGeoPositioning& other) noexcept = delete;
          /** @brief Delete the move constructor */
          AlignableGeoPositioning(AlignableGeoPositioning&& other) noexcept = delete;
          /** @brief Delete the move assignment operator */
          AlignableGeoPositioning& operator=(AlignableGeoPositioning& other) noexcept = delete;
          /** @brief Default destructors */
          virtual ~AlignableGeoPositioning();
          /** @brief Returns the sensor hash of this transformation cache */
          IdentifierHash hash() const;
          /** @brief Returns the matching transformation from the alignment store. 
            *        If a nullptr is given, then it's equivalent to the case that the transformation
            *        is pointing to a perfectly aligned surface. In this case, the internal nominal
            *        transformation cache is invoked. 
            * @param store: Pointer to the detector aligment store */
          const Amg::Isometry3D& getTransform(const DetectorAlignStore* store) const;
          /** @brief returns the cached transform from the Acts Geometry context
           *  @param gctx: The geometry context holding the aligned transforms */
          const Amg::Isometry3D& getTransform(const GeometryContext& gctx) const;
          /** @brief Store the final transform in the mutable alignment store. 
           *         Returns true whether a new transform was stored
           *  @param store: The reference to the store where the cache
           *                 stores its transform*/
          bool storeTransform(DetectorAlignStore& store) const;
#ifndef SIMULATIONBASE
          /** @brief returns the cached transform from the Acts Geometry context
           *  @param tgContext: The geometry context to be unpacked to the 
           *                    ATLAS geometry context */
          const Amg::Isometry3D& getTransform(const Acts::GeometryContext& tgContext) const;
#endif
          /** @brief resets the nominal cache associated with the detector element*/
          virtual void releaseNominalCache() const;
          /** @brief returns the detector type of the cache*/
          DetectorType detectorType() const;
      protected:
          /** @brief Assembles the transform from the readout geometry. This is the GeoModel
           *         facing side, so a general affine transform; the cache converts it to an
           *         Amg::Isometry3D once, checking the linear part, before handing it to Acts. */
          virtual Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const = 0;
      private:
          const IdentifierHash m_hash{};
          const DetectorType m_type{DetectorType::UnDefined};
          using TicketCounter = detail::TrfStoreTicketCounter;
          /** @brief Slot at which the aligned transforms can be put in the external 
           *         DetectorAlignStore */
          const unsigned int m_clientNo{TicketCounter::drawTicket(m_type)};
          /** @brief Cache to hold the nominal transform, if no DetectorAlignStore is provided
           *         The cache is automatically erased as soon as an instantiated store is 
           *         presented to the class */
          mutable CxxUtils::CachedUniquePtrT<Amg::Isometry3D> m_nomCache ATLAS_THREAD_SAFE{};
  };


  /** @brief Interface extension for caches used to hold the transforms
   *         of surfaces from the readout geometry. The interface expands 
   *         by the possibility to request the Surface's identifier and 
   *         to access the parent IDetectorElement.
   * 
   *         Further, the interface has two protected pointers to take
   *         the ownership of the associated SurfacePlacementBase and 
   *         of the Surface aligned by the positioning object */
  class IReadoutSurfacePositioning : public AlignableGeoPositioning {
    public:
      /** @brief Copy the constructors from the base class */
      using AlignableGeoPositioning::AlignableGeoPositioning;
      /** @brief Destructor */
      virtual ~IReadoutSurfacePositioning();
      /** @brief Returns the Identifier of the  */
      virtual Identifier identify() const = 0;
      /** @brief Returns the parent IDetectorElement owning the cache*/
      virtual const IDetectorElement* parent() const = 0;
      /** @brief Release the nominal transform stored in the cache */
      virtual void releaseNominalCache() const final;

  #ifndef SIMULATIONBASE
      friend class SurfacePlacement;
      /** @brief Returns the pointer to the associated surface placement */
      const SurfacePlacement* placement() const;
    private:
      /** @brief Holder of the surface owned by the surface placement */
      std::shared_ptr<Acts::Surface> m_surface{};
      /** @brief Holder of the surface placement pointer */
      std::shared_ptr<SurfacePlacement> m_placement{};
  #endif
 
  };

  /** @brief Extension to handle the caching process of the transform held by the cache.
   *         The class is templated over the actual ATLAS implementation and overrides
   *         the interface functions from the @ref TransformCache class. 
   *         The @ref identify and the @ref fetchTransform method are not implemented yet.
   *         Their implementation needs to happen at the same place where the template
   *         specification of the class is instantiated. E.g.
   *  
   *            class CakeDetectorElement : public IDetectorElement {
   *                  public:
   *                     using CakeSensorTrfCache_t = ReadoutSurfacePositioning<CakeDetectorElement>;
   *             };
   *         Instantiate the identify and fetch transform methods after the class... 
   *            Identifier ReadoutSurfacePositioning<CakeDetectorElement>::identify() const { return parent()->identify(); }
   * 
   *            Amg::Transform3D ReadoutSurfacePositioning<CakeDetectorElement>::fetchTransform(const DetectorAlignStore* store) const { 
   *                  return parent()->nominalTrf() *parent()->alignShift(store); 
   *            }
   */
  template<typename CachingDetectorEle> 
  class ReadoutSurfacePositioning: public IReadoutSurfacePositioning {
    public:
      /** @brief: Standard constructor taking the hash of the sensor element and 
       *          and the TransformMaker expressed usually as a lambda function
      **/
      explicit ReadoutSurfacePositioning(const IdentifierHash& hash, 
                                         const CachingDetectorEle* parentEle);
      /** @copydoc IReadoutSurfacePositioning::parent */
      const CachingDetectorEle* parent() const override final;
      /** @copydoc IReadoutSurfacePositioning::identify */
      Identifier identify() const override final;
    private:
      /** @copydoc IReadoutSurfacePositioning::fetchTransform */
       Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const override final;
       const CachingDetectorEle* m_parent{nullptr};
  };

}
#include <ActsGeoUtils/TransformCache.icc>
#endif