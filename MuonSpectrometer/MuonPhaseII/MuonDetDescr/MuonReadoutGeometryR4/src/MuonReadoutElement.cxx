/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "ActsGeoUtils/TransformCache.h"
#include "ActsGeoUtils/SurfaceCache.h"
#ifndef SIMULATIONBASE
#    include "Acts/Surfaces/LineBounds.hpp"
#    include "Acts/Surfaces/PlanarBounds.hpp"
#    include "Acts/Geometry/GeometryContext.hpp"
#    include "Acts/Surfaces/StrawSurface.hpp"
#    include "Acts/Surfaces/PlaneSurface.hpp"
#endif

using namespace ActsTrk;
namespace {  
    /// Dummy transformation
    static const Amg::Transform3D dummyTrans{Amg::Transform3D::Identity()};
}
namespace MuonGMR4 {
MuonReadoutElement::~MuonReadoutElement() = default;
MuonReadoutElement::MuonReadoutElement(const defineArgs& args)
    : GeoVDetectorElement(args.physVol),
      AthMessaging("MuonReadoutElement"),
      m_args{args} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        ATH_MSG_FATAL("Failed to retrieve the MuonIdHelperSvc");
    }
    m_stName = m_idHelperSvc->stationName(identify());
    m_stEta = m_idHelperSvc->stationEta(identify());
    m_stPhi = m_idHelperSvc->stationPhi(identify());
    m_detElHash = m_idHelperSvc->detElementHash(identify());
    m_chIdx = m_idHelperSvc->chamberIndex(identify());
}
StatusCode MuonReadoutElement::createGeoTransform() {
    /// Check that the alignable node has been assigned
    if(!alignableTransform()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" has no assigned alignable node");
       return StatusCode::FAILURE;
    }
    return insertTransform<MuonReadoutElement>(geoTransformHash());
}
IdentifierHash MuonReadoutElement::geoTransformHash() {     
    static const IdentifierHash hash{static_cast<unsigned>(~0)-1};
    return hash;
}


const Amg::Transform3D& MuonReadoutElement::localToGlobalTransform(const ActsTrk::GeometryContext& ctx, 
                                                               const IdentifierHash& hash) const {
    TransformCacheMap::const_iterator cache = m_localToGlobalCaches.find(hash);
    if (cache != m_localToGlobalCaches.end()) return cache->second->getTransform(ctx.getStore(detectorType()).get());
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- "
                <<idHelperSvc()->toStringDetEl(identify())<<" hash: "<<hash<<" is unknown.");
    return dummyTrans;
}

const Amg::Transform3D& MuonReadoutElement::toStation(const DetectorAlignStore* alignStore) const {
   return getMaterialGeom()->getAbsoluteTransform(alignStore ? alignStore->geoModelAlignment.get() : nullptr);
}
void MuonReadoutElement::releaseUnAlignedTrfs() const {
    for (const auto& cache : m_localToGlobalCaches) {
        cache.second->releaseNominalCache();
    }
}

unsigned int MuonReadoutElement::storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const {
    if (store.detType != detectorType()) return 0;
    unsigned int aligned{0};
    for (const auto& cache : m_localToGlobalCaches) {
        cache.second->getTransform(&store);
        ++aligned;
    }
    return aligned;
}

Amg::Transform3D MuonReadoutElement::globalToLocalTransform(const ActsTrk::GeometryContext& ctx) const {
    return globalToLocalTransform(ctx, geoTransformHash());
}
const Amg::Transform3D& MuonReadoutElement::localToGlobalTransform(const ActsTrk::GeometryContext& ctx) const {
    return localToGlobalTransform(ctx, geoTransformHash());
}
#ifndef SIMULATIONBASE
const Acts::Transform3& MuonReadoutElement::transform(const Acts::GeometryContext& anygctx) const {
    const ActsTrk::GeometryContext *gctx = anygctx.get<const ActsTrk::GeometryContext *>();
    return localToGlobalTransform(*gctx, geoTransformHash());
}
std::shared_ptr<Acts::Surface> MuonReadoutElement::surfacePtr(const IdentifierHash& hash) const {
    ActsTrk::SurfaceCacheSet::const_iterator cache = m_surfaces.find(hash);
    if(cache != m_surfaces.end()) return (*cache)->getSurface();
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
               <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));
    return nullptr;
}

const Acts::Surface& MuonReadoutElement::surface() const { return surface(geoTransformHash()); }
Acts::Surface& MuonReadoutElement::surface() { return surface(geoTransformHash()); }
const Acts::Surface& MuonReadoutElement::surface(const IdentifierHash& hash) const { return *surfacePtr(hash); }
Acts::Surface& MuonReadoutElement::surface(const IdentifierHash& hash) { return *surfacePtr(hash); }

StatusCode MuonReadoutElement::strawSurfaceFactory(const IdentifierHash& hash, 
                                                   std::shared_ptr<const Acts::LineBounds> lBounds) {

    //get the local to global transform cache
    TransformCacheMap::const_iterator transformCache = m_localToGlobalCaches.find(hash);
    if (transformCache == m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }

    auto insert_itr = m_surfaces.insert(std::make_unique<ActsTrk::SurfaceCache>(transformCache->second.get()));
    if(!insert_itr.second){
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
    //Create straw surface for the surface cache
    (*insert_itr.first)->setSurface(Acts::Surface::makeShared<Acts::StrawSurface>(lBounds, **insert_itr.first));
    return StatusCode::SUCCESS;

}

StatusCode MuonReadoutElement::planeSurfaceFactory(const IdentifierHash& hash, 
                                                   std::shared_ptr<const Acts::PlanarBounds> pBounds){

    //get the local to global transform cache
    TransformCacheMap::const_iterator transformCache = m_localToGlobalCaches.find(hash);
    if (transformCache == m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }    
    auto insert_itr = m_surfaces.insert(std::make_unique<ActsTrk::SurfaceCache>(transformCache->second.get()));
    if(!insert_itr.second){
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
    //Create a plane surface for the surface cache
    (*insert_itr.first)->setSurface(Acts::Surface::makeShared<Acts::PlaneSurface>(pBounds, **insert_itr.first));
    return StatusCode::SUCCESS;
}

void MuonReadoutElement::setChamberLink(const Chamber* chamber) {
    m_chambLink = chamber;
}
void MuonReadoutElement::setSectorLink(const SpectrometerSector* envelope) {
    m_msSectorLink = envelope;
}

std::vector<std::shared_ptr<Acts::Surface>> MuonReadoutElement::getSurfaces() const {
    std::vector<std::shared_ptr<Acts::Surface>> surfaces{};
    surfaces.reserve(m_surfaces.size());
    for (const std::unique_ptr<SurfaceCache>& cache : m_surfaces) {
        if (cache->hash() != geoTransformHash()) {
            surfaces.push_back(cache->getSurface());
            ATH_MSG_VERBOSE("Add surface "<<idHelperSvc()->toString(cache->identify())
                           <<std::endl<<(surfaces.back()->bounds()));
        }
    }
    return surfaces;
}
#endif

}  // namespace MuonGMR4
