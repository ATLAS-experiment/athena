/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "PFTrackCaloExtensionTool.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "Acts/Material/MaterialInteraction.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "eflowTrackCaloPoints.h"

StatusCode PFTrackCaloExtensionTool::initialize() {

  ATH_CHECK( AthAlgTool::initialize() );

  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_trackingGeometrySvc.retrieve());

  std::array<std::string,3 > caloNames = {"EMB1_layer", "EMB2_layer", "EMB3_layer"};

  m_trackingGeometrySvc->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {
    const auto & name = vol->volumeName();
    ATH_MSG_DEBUG(name << " - " << vol->geometryId() << " - surfaces: " << vol->surfaces().size());
    if (std::ranges::contains(caloNames, name)){
        ATH_MSG_DEBUG("About to insert caloName " << name << " into map");
        m_caloNameGeoIDMap[vol->geometryId()] = name;
    } 
  });

  return StatusCode::SUCCESS;
}

std::unique_ptr<eflowTrackCaloPoints> PFTrackCaloExtensionTool::execute(const EventContext& ctx, const xAOD::TrackParticle* track) const {

    const Acts::TrackingVolume* caloExit = m_trackingGeometrySvc->getEnvelope(ActsTrk::SystemEnvelope::CaloExit);

    unsigned int lastMeasIdx = 0;
    if (!track->indexOfParameterAtPosition(lastMeasIdx, xAOD::LastMeasurement)) {
        ATH_MSG_ERROR("TrackParticle has no last measurement parameters");
        return nullptr;
    }

    Acts::Vector3 lastPos{track->parameterX(lastMeasIdx),
                          track->parameterY(lastMeasIdx),
                          track->parameterZ(lastMeasIdx)};
    Acts::Vector3 lastMom{track->parameterPX(lastMeasIdx),
                          track->parameterPY(lastMeasIdx),
                          track->parameterPZ(lastMeasIdx)};
    lastMom *= Acts::UnitConstants::MeV;

    Acts::BoundVector lastBoundParams = Acts::BoundVector::Zero();
    lastBoundParams[Acts::eBoundPhi]    = lastMom.phi();
    lastBoundParams[Acts::eBoundTheta]  = lastMom.theta();
    lastBoundParams[Acts::eBoundQOverP] = track->charge() / lastMom.norm();

    std::shared_ptr<const Acts::Surface> lastSurface = Acts::CurvilinearSurface(lastPos, lastMom.normalized()).planeSurface()->getSharedPtr();

    Acts::BoundTrackParameters boundPars{
        std::move(lastSurface),
        lastBoundParams,
        std::nullopt,
        Acts::ParticleHypothesis::pion()
    };

    Acts::Result<std::pair<std::vector<Acts::detail::Step>, Acts::RecordedMaterial>> result = m_extrapolationTool.get()->propagationSteps(ctx, boundPars);

    if( !result.ok() ) {
        ATH_MSG_WARNING("Error during extrapolation: " << result.error().message());
        return nullptr;
    }

    const auto &[steps, _] = result.value();

    ATH_MSG_DEBUG("Have extrapolated track with pt, eta and phi: " << track->pt() << ", " << track->eta() << " and " << track->phi());

    for(const auto &step : steps) {


        if( step.surface == nullptr || step.surface->geometryId().sensitive() == 0 ) {
          continue;
        }

        Acts::GeometryIdentifier thisGeoID = step.geoID;
        ATH_MSG_DEBUG("Got step with geoID " << thisGeoID);

        if (m_caloNameGeoIDMap.contains(thisGeoID)){
            const auto &p = step.position;
            auto eta = Acts::VectorHelpers::eta(p);
            auto phi = Acts::VectorHelpers::phi(p);
            ATH_MSG_DEBUG("Eta and Phi in caloLayer " << m_caloNameGeoIDMap.at(thisGeoID) << " are " << eta << " and " << phi);
        }
        else ATH_MSG_WARNING("Could not find this GeometryIdentifier " << thisGeoID << " in the map");        
    }

    ATH_MSG_DEBUG("Finished steps loop");
    ATH_MSG_DEBUG("");

    return std::make_unique<eflowTrackCaloPoints>();
}

StatusCode PFTrackCaloExtensionTool::finalize() {
  return StatusCode::SUCCESS;
}