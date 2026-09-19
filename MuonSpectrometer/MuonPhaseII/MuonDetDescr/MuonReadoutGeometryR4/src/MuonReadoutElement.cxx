/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "ActsGeoUtils/TransformCache.h"
#include "ActsGeoUtils/SurfacePlacement.h"
#ifndef SIMULATIONBASE
#    include "Acts/Surfaces/LineBounds.hpp"
#    include "Acts/Surfaces/PlanarBounds.hpp"
#    include "Acts/Geometry/GeometryContext.hpp"
#    include "Acts/Surfaces/StrawSurface.hpp"
#    include "Acts/Surfaces/PlaneSurface.hpp"
#endif

using namespace ActsTrk;
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
    m_centralTrfCache = std::make_unique<ReadoutSurfacePositioning<MuonReadoutElement>>(geoTransformHash(), this);
    return StatusCode::SUCCESS;
}
IdentifierHash MuonReadoutElement::geoTransformHash() {     
    static const IdentifierHash hash{static_cast<unsigned>(~0)-1};
    return hash;
}

const Amg::Transform3D& MuonReadoutElement::toStation(const DetectorAlignStore* alignStore) const {
   return getMaterialGeom()->getAbsoluteTransform(alignStore ? alignStore->geoModelAlignment.get() : nullptr);
}
void MuonReadoutElement::releaseUnAlignedTrfs() const {
    for (const auto& cache : m_localToGlobalCaches) {
        if (cache){
            cache->releaseNominalCache();
        }
    }
    m_centralTrfCache->releaseNominalCache();
}

unsigned MuonReadoutElement::storeAlignedTransforms(DetectorAlignStore& store) const {
    if (store.detType != detectorType()) {
        return 0;
    }
    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Start "<<idHelperSvc()->toStringDetEl(identify())<<".");
    unsigned int aligned{0};
    aligned+=m_centralTrfCache->storeTransform(store);
    for (const auto& cache : m_localToGlobalCaches) {
        if (!cache) {
            continue;
        }
        aligned+=cache->storeTransform(store);
    }
    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Internal caching done.");
    return aligned;
}

Amg::Transform3D MuonReadoutElement::globalToLocalTransform(const GeometryContext& ctx) const {
    return globalToLocalTransform(ctx, geoTransformHash());
}
const Amg::Isometry3D& MuonReadoutElement::localToGlobalTransform(const GeometryContext& ctx) const {
    return localToGlobalTransform(ctx, geoTransformHash());
}
#ifndef SIMULATIONBASE
const Amg::Isometry3D& MuonReadoutElement::localToGlobalTransform(const Acts::GeometryContext& anygctx) const {
    const GeometryContext *gctx = anygctx.get<const GeometryContext *>();
    return localToGlobalTransform(*gctx, geoTransformHash());
}
std::shared_ptr<Acts::Surface> MuonReadoutElement::surfacePtr(const IdentifierHash& hash) const {
    const  IReadoutSurfacePositioning* cache = transformCache(hash);
    if(cache  && cache->placement()) return cache->placement()->getSurface();
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
    IReadoutSurfacePositioning* cache = transformCache(hash);
    if (!cache) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }
    auto placement =  SurfacePlacement::makeShared<Acts::StrawSurface>(*cache, std::move(lBounds));
    if(!placement){
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonReadoutElement::planeSurfaceFactory(const IdentifierHash& hash, 
                                                   std::shared_ptr<const Acts::PlanarBounds> pBounds){

    //get the local to global transform cache
    IReadoutSurfacePositioning* cache = transformCache(hash);
    if (!cache) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }
    auto placement =  SurfacePlacement::makeShared<Acts::PlaneSurface>(*cache, std::move(pBounds));
    if(!placement) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
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
    surfaces.reserve(m_localToGlobalCaches.size());
    for (const auto& cache : m_localToGlobalCaches) {
        if (cache && cache->placement()) {
            surfaces.push_back(cache->placement()->getSurface());
            ATH_MSG_VERBOSE("Add surface "<<idHelperSvc()->toString(cache->identify())
                           <<std::endl<<(surfaces.back()->bounds()));
        }
    }
    return surfaces;
}
#endif

}  // namespace MuonGMR4
