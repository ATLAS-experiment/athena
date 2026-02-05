/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialValidation.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/IInterface.h"
#include "ActsInterop/Logger.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/IntersectionMaterialAssigner.hpp"

ActsTrk::MaterialValidation::MaterialValidation(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
{}

ActsTrk::MaterialValidation::~MaterialValidation()
{}

StatusCode ActsTrk::MaterialValidation::initialize()
{
    ATH_CHECK(m_materialTrackCollectionKey.initialize());
    ATH_CHECK(m_rndmGenSvc.retrieve());
    ATH_CHECK(m_trackingGeometrySvc.retrieve());

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

    /// The material validater
    Acts::MaterialValidater::Config validaterConfig;
    validaterConfig.materialAssigner = materialAssigner;
    m_materialValidater = std::make_shared<Acts::MaterialValidater> (validaterConfig,
                                                                     makeActsAthenaLogger(this, "MaterialValidater"));

    return StatusCode::SUCCESS;
}

StatusCode
ActsTrk::MaterialValidation::execute (const EventContext& ctx) const
{
    // Write to the collection to the EventStore
    SG::WriteHandle<ActsTrk::RecordedMaterialTrackCollection> materialTracks(m_materialTrackCollectionKey, ctx);

    // Record the collection once per event if not already there
    if (!materialTracks.isPresent()) {
        auto coll = std::make_unique<ActsTrk::RecordedMaterialTrackCollection>();
        ATH_CHECK(materialTracks.record(std::move(coll)));
    }

    // Add the track to the recorded collection
    auto* coll = materialTracks.ptr();
    if (!coll) {
        ATH_MSG_ERROR("RecordedMaterialTrackCollection ptr() is null for key "
                      << m_materialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }

    // Some useful parameters to be used later
    Acts::Vector3 startPosition(0., 0., 0.);
    const ActsTrk::GeometryContext& geoContext{m_trackingGeometrySvc->getNominalContext()};
    Acts::MagneticFieldContext magFieldContext;

    ATHRNG::RNGWrapper *wrapper = m_rndmGenSvc->getEngine(this);
    wrapper->setSeed(name(), ctx);
    CLHEP::HepRandomEngine *rndmEngine = wrapper->getEngine(ctx);

    // Loop over the number of tracks
    for (std::size_t iTrack = 0; iTrack < m_nTracks; ++iTrack) {
        // Generate a random phi and eta
        double phi = rndmEngine->flat() * 2 * M_PI - M_PI;
        double eta = rndmEngine->flat() * std::abs(m_etaRange.value().second - m_etaRange.value().first) + m_etaRange.value().first;
        double theta = 2 * std::atan(std::exp(-eta));
        Acts::Vector3 direction(std::cos(phi) * std::sin(theta),
                                std::sin(phi) * std::sin(theta), std::cos(theta));

        // Record the material
        auto rmTrack = m_materialValidater->recordMaterial(geoContext.context(),
                                                           magFieldContext,
                                                           startPosition,
                                                           direction);

        // filling the collection
        coll->push_back(std::move(rmTrack));

    }

    return StatusCode::SUCCESS;

}


