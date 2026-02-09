/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialMapping.h"
#include "GaudiKernel/IInterface.h"
#include "ActsInterop/Logger.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/IntersectionMaterialAssigner.hpp"
#include "Acts/Material/BinnedSurfaceMaterialAccumulater.hpp"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

ActsTrk::MaterialMapping::MaterialMapping(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
{}

ActsTrk::MaterialMapping::~MaterialMapping()
{}

StatusCode ActsTrk::MaterialMapping::initialize()
{
    ATH_CHECK(m_materialTrackCollectionKey.initialize());
    ATH_CHECK(m_mappedMaterialTrackCollectionKey.initialize());
    ATH_CHECK(m_unmappedMaterialTrackCollectionKey.initialize());

    ATH_CHECK(m_trackingGeometrySvc.retrieve());

    ATH_CHECK(m_materialMapWriters.retrieve());

    // Retrieve the material surfaces
    std::vector<const Acts::Surface*> materialSurfaces = {};

    auto surfaceSelector = [&](const Acts::Surface* surface) {
        if (surface->surfaceMaterial() != nullptr &&
            std::ranges::find(materialSurfaces, surface) == materialSurfaces.end()) {
                materialSurfaces.push_back(surface);
        }
    };

    // Visit all surfaces (second argument = if true only sensitive surfaces are visited)
    m_trackingGeometrySvc->trackingGeometry()->visitSurfaces(surfaceSelector, false);

    // The material intersection assigner
    Acts::IntersectionMaterialAssigner::Config assingerConfig;
    assingerConfig.surfaces = materialSurfaces;
    auto materialAssigner = std::make_shared<Acts::IntersectionMaterialAssigner>(assingerConfig,
                                                                                 makeActsAthenaLogger(this, "MaterialAssigner"));

    // The binned surface material accumulator
    Acts::BinnedSurfaceMaterialAccumulater::Config accumulaterConfig;
    accumulaterConfig.materialSurfaces = materialSurfaces;
    auto materialAccumulater = std::make_shared<Acts::BinnedSurfaceMaterialAccumulater>(accumulaterConfig,
                                                                                        makeActsAthenaLogger(this, "MaterialAccumulater"));

    /// The material mapper
    Acts::MaterialMapper::Config mapperConfig;
    mapperConfig.assignmentFinder = materialAssigner;
    mapperConfig.surfaceMaterialAccumulater = materialAccumulater;
    m_materialMapper = std::make_shared<Acts::MaterialMapper> (mapperConfig,
                                                               makeActsAthenaLogger(this, "MaterialMapper"));

    // Create the state object
    m_mappingState = m_materialMapper->createState();

    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialMapping::finalize()
{
    Acts::TrackingGeometryMaterial detectorMaterial = m_materialMapper->finalizeMaps(*m_mappingState);

    const ActsTrk::GeometryContext& geoContext{m_trackingGeometrySvc->getNominalContext()};

    // Loop over the available writers and write the maps
    for (auto& materialWriter : m_materialMapWriters) {
        materialWriter->writeMaterial(geoContext, detectorMaterial);
    }

    return StatusCode::SUCCESS;
}

StatusCode
ActsTrk::MaterialMapping::execute (const EventContext& ctx) const
{
    // Read the collection to the EventStore
    SG::ReadHandle<ActsTrk::RecordedMaterialTrackCollection> materialTracks(m_materialTrackCollectionKey, ctx);
    // Check if all is fine
    if (!materialTracks.isValid()) {
        ATH_MSG_ERROR("Failed to read " << m_materialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }

    // Write to the collection to the EventStore
    SG::WriteHandle<ActsTrk::RecordedMaterialTrackCollection> unmappedMaterialTracks(m_unmappedMaterialTrackCollectionKey, ctx);
    SG::WriteHandle<ActsTrk::RecordedMaterialTrackCollection> mappedMaterialTracks(m_mappedMaterialTrackCollectionKey, ctx);

    // Record the collection once per event if not already there
    // You do it once for the unmapped material tracks ...
    if (!unmappedMaterialTracks.isPresent()) {
        auto coll = std::make_unique<ActsTrk::RecordedMaterialTrackCollection>();
        ATH_CHECK(unmappedMaterialTracks.record(std::move(coll)));
    }
    auto* unmappedColl = unmappedMaterialTracks.ptr();
    if (!unmappedColl) {
        ATH_MSG_ERROR("RecordedMaterialTrackCollection ptr() is null for key "
                      << m_unmappedMaterialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }
    // ... and then for the mapped material tracks
    if (!mappedMaterialTracks.isPresent()) {
        auto coll = std::make_unique<ActsTrk::RecordedMaterialTrackCollection>();
        ATH_CHECK(mappedMaterialTracks.record(std::move(coll)));
    }
    auto* mappedColl = mappedMaterialTracks.ptr();
    if (!mappedColl) {
        ATH_MSG_ERROR("RecordedMaterialTrackCollection ptr() is null for key "
                      << m_mappedMaterialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }

    auto mappingState = const_cast<Acts::MaterialMapper::State*>(m_mappingState.get());

    const ActsTrk::GeometryContext& geoContext{m_trackingGeometrySvc->getNominalContext()};
    Acts::MagneticFieldContext magFieldContext;

    // Loop over the material tracks and map them on the surfaces
    for (const auto& materialTrack : *materialTracks) {
        auto [mapped, unmapped] = m_materialMapper->mapMaterial(
            *mappingState, geoContext.context(), magFieldContext, materialTrack);
        // collect mapped and unmapped states
        mappedColl->push_back(mapped);
        unmappedColl->push_back(unmapped);
  }

  return StatusCode::SUCCESS;

}


