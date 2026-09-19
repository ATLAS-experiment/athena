/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#include "ActsGeoUtils/VolumePlacement.h"
#include "GeoModelHelpers/GeoDeDuplicator.h"

#include <cassert>
namespace ActsTrk{
    //#################################################################################
    //                      VolumePlacement::VolumeGeoPositioning
    //#################################################################################
    VolumePlacement::VolumeGeoPositioning::VolumeGeoPositioning(const CacheFlags flags,
                                                const DetectorType type,
                                                const VolumePlacement* parent):
        ActsTrk::AlignableGeoPositioning{IdentifierHash{}, type},
        m_parent{parent}, m_flags{flags} {}
    VolumePlacement::VolumeGeoPositioning::VolumeGeoPositioning(const VolumePlacement* parent,
                                                const std::size_t portalIdx):
        ActsTrk::AlignableGeoPositioning(portalIdx, parent->detectorType()),
        m_parent{parent} {}                                       
                    
    Amg::Transform3D VolumePlacement::VolumeGeoPositioning::fetchTransform(const DetectorAlignStore* store) const {
        switch (m_flags) {
            case CacheFlags::volumeLocToGlob: {
                return m_parent->localToGlobalTransform(store);
            } case CacheFlags::portalLocToGlob: {
                return m_parent->localToGlobalTransform(store) *
                       m_parent->portalPlacement(hash())->portalToVolumeCenter();
            } case CacheFlags::volumeGlobToLoc: {
                return m_parent->localToGlobalTransform(store).inverse();
            }
        }
        return Amg::Transform3D::Identity();
    }
    //#################################################################################
    //                          VolumePlacement
    //#################################################################################
    VolumePlacement::VolumePlacement(const DetectorType detType,
                                     const AlignableNode_t parentNode,
                                     const std::optional<Amg::Transform3D> & addShift):
        m_parent{parentNode},
        m_locToGlobCache{std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeLocToGlob, detType, this)},
        m_globToLocCache{std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeGlobToLoc, detType, this)} {
        if (!addShift) {
            return;
        }       
        m_refShift = GeoDeDuplicator{}.makeTransform(*addShift);
    }
    void VolumePlacement::addChild(std::unique_ptr<VolumePlacement>&& child){
        assert(child.get() != this);
        assert(child != nullptr);
        m_children.push_back(std::move(child));
    }
    VolumePlacement::VolumePlacement(const IDetectorElement& parentElement,
                                     const std::optional<Amg::Transform3D> & addShift):
        m_parent{&parentElement},
        m_globToLocCache{std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeGlobToLoc, 
                                                                parentElement.detectorType(), this)} {
        if (!addShift) {
            return;
        }
        m_locToGlobCache = std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeLocToGlob,
                                                                   parentElement.detectorType(), this);
        m_refShift = GeoDeDuplicator{}.makeTransform(*addShift);

    }

    VolumePlacement::VolumePlacement(const VolumePlacement& parentPlacement,
                                     const std::optional<Amg::Transform3D> & addShift):
        m_parent{&parentPlacement}{
        if (!addShift) {
            return;
        }
        m_locToGlobCache = std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeLocToGlob, 
                                                         parentPlacement.detectorType(), this);
        m_globToLocCache = std::make_unique<VolumeGeoPositioning>(VolumeGeoPositioning::CacheFlags::volumeGlobToLoc, 
                                                          parentPlacement.detectorType(), this);
        m_refShift = GeoDeDuplicator{}.makeTransform(*addShift);

    }

    void VolumePlacement::makePortalsAlignable(const Acts::GeometryContext& gctx,
                                               const std::vector<std::shared_ptr<Acts::RegularSurface>>& portalsToAlign) {
        Acts::VolumePlacementBase::makePortalsAlignable(gctx, portalsToAlign);
        for (std::size_t p = 0ul; p < portalsToAlign.size(); ++p) {
            m_portalCaches.emplace_back(std::make_unique<VolumeGeoPositioning>(this, p));
        }
    }

    const Acts::Transform3& VolumePlacement::localToGlobalTransform(const Acts::GeometryContext& tgContext) const {
        if (!m_locToGlobCache) {
            if (std::holds_alternative<const IDetectorElement*>(m_parent)){
                const auto* gctx = tgContext.get<const GeometryContext*>();
                return std::get<const IDetectorElement*>(m_parent)->localToGlobalTransform(*gctx);
            } else if (std::holds_alternative<const VolumePlacement*>(m_parent)) {
                return std::get<const VolumePlacement*>(m_parent)->localToGlobalTransform(tgContext);
            } else {
                THROW_EXCEPTION("A VolumePlacement without cache && alignable trf should never happen");
            }
        }
        return m_locToGlobCache->getTransform(tgContext);
    }
    const Amg::Isometry3D& VolumePlacement::localToGlobalTransform(const GeometryContext& gctx) const {
        return localToGlobalTransform(gctx.context());
    }
    const Acts::Transform3& VolumePlacement::globalToLocalTransform(const Acts::GeometryContext& tgContext) const{
        // The only configuration without glob -> loc cache is a parent volume placement
        if (!m_globToLocCache) {
            return std::get<const VolumePlacement*>(m_parent)->globalToLocalTransform(tgContext);
        }
        return m_globToLocCache->getTransform(tgContext);
    }
    const Amg::Isometry3D& VolumePlacement::globalToLocalTransform(const GeometryContext& gctx) const {
        return globalToLocalTransform(gctx.context());
    }

    const Acts::Transform3& VolumePlacement::portalLocalToGlobal(const Acts::GeometryContext& tgContext, 
                                                                 const std::size_t portalIdx) const {
        return portalIdx < m_portalCaches.size() ? 
               m_portalCaches.at(portalIdx)->getTransform(tgContext) : localToGlobalTransform(tgContext);
    }
    void VolumePlacement::connectCenterSurface(std::shared_ptr<Acts::RegularSurface> surface) {
        if (m_surfacePlacement) {
            THROW_EXCEPTION("Center surface already defined");
        }
        m_surfacePlacement = std::make_shared<Acts::detail::PortalPlacement>(std::numeric_limits<std::size_t>::max(),
                                                                             Amg::Isometry3D::Identity(), this,
                                                                             surface);
   }
   DetectorType VolumePlacement::detectorType() const { 
       if (m_locToGlobCache){
            return m_locToGlobCache->detectorType();
       }
       return std::visit([](const auto& parent) {
            using visit_t = std::decay_t<decltype(parent)>;
            if constexpr(!std::is_same_v<visit_t, AlignableNode_t>) {
                return parent->detectorType();
            }
            return DetectorType::UnDefined;
       }, m_parent);
    }

    Amg::Transform3D VolumePlacement::localToGlobalTransform(const DetectorAlignStore* store) const {
        return std::visit([&](const auto& parent) -> Amg::Transform3D {
            using visit_t = std::decay_t<decltype(parent)>;
            if constexpr(std::is_same_v<visit_t, AlignableNode_t>) {
                return parent->getTransform(store ? store->geoModelAlignment.get() : nullptr);
            } else {
                return parent->localToGlobalTransform(store);
            }
            return Amg::Transform3D::Identity(); 
        }, m_parent) * (m_refShift ? m_refShift->getDefTransform() 
                                   : Amg::Transform3D::Identity());
    }
  
    unsigned VolumePlacement::storeAlignedTransforms(DetectorAlignStore& store) const {
        unsigned n{0};
        for (const VolumeGeoPositioning* volCache : {m_locToGlobCache.get(), m_globToLocCache.get()}) {
            if (volCache) {
                n+=volCache->storeTransform(store);
            }
        }
        for (const std::unique_ptr<VolumeGeoPositioning>& cache: m_portalCaches) {
            n+=cache->storeTransform(store);
        }
        for (const std::shared_ptr<VolumePlacement>& child : m_children) {
            n+=child->storeAlignedTransforms(store);
        }
        return n;
    }
}
#endif