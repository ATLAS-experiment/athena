/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "PFTrackCaloExtensionTool.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/MaterialInteraction.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "ActsEvent/CaloExtension.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsInterop/UnitConverters.h"

#include "StoreGate/ReadDecorHandle.h"

#include "eflowTrackCaloPoints.h"

StatusCode PFTrackCaloExtensionTool::initialize() {

  ATH_CHECK( AthAlgTool::initialize() );
  ATH_CHECK(m_ctxProvider.initialize());

  ATH_CHECK(m_trackingGeometrySvc.retrieve());

  ATH_CHECK(m_extensionReadDecorKey.initialize());

  std::array<std::string, 7 > caloNames = {"EMB1_Layer", "EMB2_Layer", "EMB3_Layer",
                                          "TileBar0_Layer", "TileBar1_Layer", 
                                          "CaloLayerDiscNegativeZ", "CaloLayerDiscPositiveZ"};


  m_trackingGeometrySvc->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {

    //get geo identifier
    Acts::GeometryIdentifier geoId = vol->geometryId();
    const auto & name = vol->volumeName();
    ATH_MSG_DEBUG(name << " - " << geoId << " - surfaces: " << vol->surfaces().size());
  
    if (std::ranges::contains(caloNames, name)){
      vol->visitSurfaces([&](const Acts::Surface *surface){
      ATH_MSG_DEBUG("This surface has ID" << surface->geometryId());
      m_geoIdCaloSampleMap[surface->geometryId()] = static_cast<CaloSampling::CaloSample>(surface->geometryId().layer());
      });
    };
  });

  return StatusCode::SUCCESS;
}

std::unique_ptr<eflowTrackCaloPoints> PFTrackCaloExtensionTool::execute(const EventContext& ctx, const xAOD::TrackParticle* track) const {

    ATH_MSG_DEBUG("About to extrapolate track with pt, eta and phi: " << track->pt() << ", " << track->eta() << " and " << track->phi());

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::CaloExtensionContainer>> extensionReadDecorHandle(m_extensionReadDecorKey, ctx);
    ElementLink<ActsTrk::CaloExtensionContainer> trackCaloExtensionLink = extensionReadDecorHandle(*track);

    std::map<eflowCalo::LAYER, const Acts::BoundTrackParameters*> parametersMap;
    const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);

    if (!trackCaloExtensionLink.isValid()) {
       ATH_MSG_DEBUG("Decoration exists, but this track has no valid CaloExtension link (null link)");
    }
    else{
      const ActsTrk::CaloExtension* trackCaloExtension = *trackCaloExtensionLink;
      ATH_MSG_DEBUG("Have CaloExtensions with size " << trackCaloExtension->size() << " for this track");
      ATH_MSG_DEBUG("Are the parameters empty?: " << trackCaloExtension->parameters().empty());

      for (const Acts::BoundTrackParameters& pars : trackCaloExtension->parameters()) {
          const Acts::Surface& surface = pars.referenceSurface();
          ATH_MSG_DEBUG("Geometry ID of surface is " << surface.geometryId());

          const auto thisGeoID = surface.geometryId();

          if (m_geoIdCaloSampleMap.contains(thisGeoID)) {
            CaloSampling::CaloSample caloSample = m_geoIdCaloSampleMap.at(thisGeoID);
            ATH_MSG_DEBUG("This surface is in the calorimeter with CaloSample " << caloSample << " and eta, phi: " << pars.position(gctx).eta() << ", " << pars.position(gctx).phi());
            /* parametersMap may have several entries for Tile1,2,3 
            due to the simplfification of eflowCalo::LAYER which only considers
            Tile in general and not barrel, exctended barrel and gap.
            The impact is negligible as the eta/phi of these entries are very similar
            https://its.cern.ch/jira/browse/ATLJETMET-242
            */
            parametersMap[eflowCalo::translateSampl(caloSample)] = &pars;
          }  
      }
    }

    return std::make_unique<eflowTrackCaloPoints>(parametersMap, gctx);
}

StatusCode PFTrackCaloExtensionTool::finalize() {
  return StatusCode::SUCCESS;
}
