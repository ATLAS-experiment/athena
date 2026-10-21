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

  std::array<std::string,38 > caloNames = {"EMB1_Layer", "EMB2_Layer", "EMB3_Layer",
                                          "EME1NegZ_Layer", "EME2NegZ_Layer", "EME3NegZ_Layer",
                                          "EME1PosZ_Layer", "EME2PosZ_Layer", "EME3PosZ_Layer",
                                          "HEC0NegZ_Layer", "HEC1NegZ_Layer", "HEC2NegZ_Layer", "HEC3NegZ_Layer",
                                          "HEC0PosZ_Layer", "HEC1PosZ_Layer", "HEC2PosZ_Layer", "HEC3PosZ_Layer",
                                          "TileBar0_Layer", "TileBar1_Layer", "TileBar2_Layer",
                                          "TileGap1NegZ_Layer", "TileGap2NegZ_Layer", "TileGap3NegZ_Layer",
                                          "TileGap1PosZ_Layer", "TileGap2PosZ_Layer", "TileGap3PosZ_Layer",
                                          "TileExt0NegZ_Layer","TileExt1NegZ_Layer","TileExt2NegZ_Layer",
                                          "TileExt0PosZ_Layer","TileExt1PosZ_Layer","TileExt2PosZ_Layer",
                                          "FCAL0NegZ_Layer","FCAL1NegZ_Layer","FCAL2NegZ_Layer",
                                          "FCAL0PosZ_Layer","FCAL1PosZ_Layer","FCAL2PosZ_Layer"};


  m_trackingGeometrySvc->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {

    //get geo identifier
    Acts::GeometryIdentifier geoId = vol->geometryId();
    const auto & name = vol->volumeName();
    ATH_MSG_DEBUG(name << " - " << geoId << " - surfaces: " << vol->surfaces().size());
  
    if (std::ranges::contains(caloNames, name)){
        ATH_MSG_DEBUG("About to insert caloName " << name << " into map");
        m_caloVolumeLayerMap[std::make_pair(geoId.volume(), geoId.layer())] = vol->volumeName();
    }

  });

  return StatusCode::SUCCESS;
}

std::unique_ptr<eflowTrackCaloPoints> PFTrackCaloExtensionTool::execute(const EventContext& ctx, const xAOD::TrackParticle* track) const {

    ATH_MSG_DEBUG("About to extrapolate track with pt, eta and phi: " << track->pt() << ", " << track->eta() << " and " << track->phi());

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::CaloExtensionContainer>> extensionReadDecorHandle(m_extensionReadDecorKey, ctx);
    ElementLink<ActsTrk::CaloExtensionContainer> trackCaloExtensionLink = extensionReadDecorHandle(*track);

    std::map<eflowCalo::LAYER, const Acts::BoundTrackParameters&> parametersMap;
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

          if (m_caloVolumeLayerMap.contains(std::make_pair(thisGeoID.volume(), thisGeoID.layer()))) {
            ATH_MSG_DEBUG("This surface is in the calorimeter");
            Acts::Vector3 pos = pars.position(gctx);
            double eta = pos.eta();
            double phi = pos.phi();
            ATH_MSG_DEBUG("Eta and Phi in caloLayer " << m_caloVolumeLayerMap.at(std::make_pair(thisGeoID.volume(), thisGeoID.layer())) << " are " << eta << " and " << phi);
          }    
      }
    }

    return std::make_unique<eflowTrackCaloPoints>(parametersMap, gctx);
}

StatusCode PFTrackCaloExtensionTool::finalize() {
  return StatusCode::SUCCESS;
}
