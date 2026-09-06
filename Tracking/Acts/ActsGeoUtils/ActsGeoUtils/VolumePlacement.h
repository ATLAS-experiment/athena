/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsGeoUtils_VolumePlacement_H
#define ActsGeoUtils_VolumePlacement_H
#ifndef SIMULATIONBASE

#include "ActsGeoUtils/TransformCache.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometryInterfaces/IVolumePlacement.h"
///

#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

#include <optional>
#include <variant>

namespace ActsTrk{
    /** @brief Implementation to make a (tracking) volume alignable. The placement
     *         is coupled to the alignment system either via a reference IDetectorElement,
     *         an alignable node in the geometry tree or another VolumePlacement. */
    class VolumePlacement: public IVolumePlacement {
        public:
            /** @brief Abrivation of an alignable GeoTransform  */
            using AlignableNode_t = GeoIntrusivePtr<GeoAlignableTransform>;
            /** @brief Abrivation of the parent input types that are 
             *         connected to the alignment system
             *   --> A direct alignable transform
             *   --> A detector element base following the alignment
             *   --> Another alignable volume element */
            using Parent_t = std::variant<AlignableNode_t, 
                                          const IDetectorElement*,
                                          const VolumePlacement*>;
            /** @brief Constructor taking an Alignable transform from the geometry tree
             *  @param detType: Detectortype of the alignment store into which the transforms are written
             *  @param parentNode: Pointer to the alignable transform
             *  @param addShift: Additional optional shift to be applied on top of the alignable reference */
            explicit VolumePlacement(const DetectorType detType,
                                     const AlignableNode_t parentNode,
                                     const std::optional<Amg::Transform3D> & addShift = std::nullopt);
            /** @brief Constructor taking the reference to a Detector element. The life time of the 
             *         detector element must be ensured to exceed the Placement's lifetime
             *  @param parentElement: The detector element which alignment the volume is following
             *  @param addShift: Additional optional shift to be applied on top of the alignable reference */
            explicit VolumePlacement(const IDetectorElement& parentElement,
                                     const std::optional<Amg::Transform3D> & addShift = std::nullopt);
            /** @brief Constructor taking the reference to another placement The life time of the 
             *       object must be ensured to exceed the Placement's lifetime
             *  @param parentPlacement: The parent placement moving according to the alignment
             *  @param addShift: Additional optional shift to be applied on top of the alignable reference */
            explicit VolumePlacement(const VolumePlacement& parentPlacement,
                                     const std::optional<Amg::Transform3D> & addShift = std::nullopt);
            /** @brief Add a child volume placement to this placement. The object takes
             *         ownership and fills the alignment cache
             *  @param child: The child to be appended  */
            void addChild(std::unique_ptr<VolumePlacement>&& child);
            /** @brief Connect an external surface and place it into the volume center
             *  @param surface pointer to the surface that is connected to the placement */
            void connectCenterSurface(std::shared_ptr<Acts::RegularSurface> surface);
            /** @copydoc ActTrk::IVolumePlacement::detectorType */
            DetectorType detectorType() const override final;
            /** @copydoc ActTrk::IVolumePlacement::storeAlignedTransforms */
            unsigned storeAlignedTransforms(DetectorAlignStore& store) const override final;
            /** @copydoc Acts::VolumePlacmentBase::makePortalsAlignable */
            void makePortalsAlignable(const Acts::GeometryContext& gctx,
                                      const std::vector<std::shared_ptr<Acts::RegularSurface>>& portalsToAlign) override final;
           /** @copydoc Acts::VolumePlacmentBase::localToGlobalTransform */
            const Acts::Transform3& localToGlobalTransform(const Acts::GeometryContext& gctx) const override final;
           /** @copydoc ActsTrk::IVolumePlacement::localToGlobalTransform */ 
            const Amg::Transform3D& localToGlobalTransform(const GeometryContext& gctx) const;
            /** @copydoc Acts::VolumePlacmentBase::localToGlobalTransform */
             const Acts::Transform3& globalToLocalTransform(const Acts::GeometryContext& gctx) const override final;
            /** @copydoc Acts::VolumePlacmentBase::globalToLocalTransform */
            const Amg::Transform3D& globalToLocalTransform(const GeometryContext& gctx) const;
            /** @copydoc Acts::VolumePlacmentBase::portalLocalToGlobal */
            const Acts::Transform3& portalLocalToGlobal(const Acts::GeometryContext& gctx, 
                                                        const std::size_t portalIdx) const override final;
        private:
            /** @brief Constructs the local -> global transform of the volume. The request
             *         is forwarded to the parent and an optional shift is applied on top
             *  @param store: From which the alignment information is grepped and to which
             *                the final transform is written */
            Amg::Transform3D localToGlobalTransform(const DetectorAlignStore* store) const;
            /** @brief Auxiliary class to store the aligned transforms of the volume and 
             *         of the associated portals */
            class VolumeGeoPositioning: public AlignableGeoPositioning {
                public:
                    /** @brief Flag to indicate which kind of transform is handled by the AlignedCache */
                    enum class CacheFlags: std::uint8_t {
                        volumeLocToGlob, // Local -> global transform of the volume
                        volumeGlobToLoc, // Global -> local transform of the volume
                        portalLocToGlob  // Local -> global transform of the associated portal
                    };
                    /** @brief Constructor for the cache storing the of the volume
                     *         itself
                     * @param flags: Flag indicating local -> global or global -> local
                     * @param type: In which detector transform store is the cache appended
                     * @param parent: Pointer to the parent creating the cache */
                    explicit VolumeGeoPositioning(const CacheFlags flags,
                                                  const DetectorType type,
                                                  const VolumePlacement* parent);
                    /** @brief Constructor for the cache storing the transform of 
                     *         the alignable portals.
                     * @param parent: Pointer to the parent creating the cache
                     * @param portalidx: Index of the portal represented by the cache */
                    explicit VolumeGeoPositioning(const VolumePlacement* parent,
                                                  const std::size_t portalIdx);
                    /** @brief Fetch the transform to store it in the detector alignment cache */
                    virtual Amg::Transform3D fetchTransform(const DetectorAlignStore* store) const override;
                private:
                    /** @brief Back reference to the parent VolumePlacement */
                    const VolumePlacement* m_parent{nullptr};
                    /** @brief Flags to indicate which transform type is handled */
                    CacheFlags m_flags{CacheFlags::portalLocToGlob};
            };
            /** @brief Parent element which is following the alignment */
            Parent_t m_parent{};
            /** @brief Additional shift on top of the parent position */
            GeoIntrusivePtr<GeoTransform> m_refShift{}; 
            /** @brief Cache to handle the local -> global transform of the volume */
            std::unique_ptr<VolumeGeoPositioning> m_locToGlobCache{};
            /** @brief Cache to handle the global -> local transform of the volume */
            std::unique_ptr<VolumeGeoPositioning> m_globToLocCache{};
            /** @brief Cache to handle the local -> global transforms of the associated portals */
            std::vector<std::unique_ptr<VolumeGeoPositioning>> m_portalCaches{};
            /** @brief Children spawning from this VolumePlacement */
            std::vector<std::shared_ptr<VolumePlacement>> m_children{};
            /** @brief Pipe the local -> global transform to a surface*/
            std::shared_ptr<Acts::detail::PortalPlacement> m_surfacePlacement{};
    };
}

#endif
#endif