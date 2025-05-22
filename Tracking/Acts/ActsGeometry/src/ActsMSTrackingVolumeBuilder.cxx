/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsMSTrackingVolumeBuilder.h"

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"

#include <ranges>

using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;


StatusCode ActsMSTrackingVolumeBuilder::initialize(){
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}

std::shared_ptr<Acts::TrackingVolume> ActsMSTrackingVolumeBuilder::trackingVolume(const Acts::GeometryContext& gctx,
                                                                                  std::shared_ptr<const Acts::TrackingVolume> /*insideVolume*/,
                                                                                  std::shared_ptr<const Acts::VolumeBounds> /*outsideBounds*/) const {
    
    const ActsGeometryContext* geoContext = gctx.get<const ActsGeometryContext* >();
    // std::unique_ptr<Acts::CutoutCylinderVolumeBounds> msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 14500, 22500, 3200);
    std::unique_ptr<Acts::CylinderVolumeBounds> msBounds = std::make_unique<Acts::CylinderVolumeBounds>(0, 14500, 22500);
    std::shared_ptr<Acts::TrackingVolume> msTrackingVolume = std::make_shared<Acts::TrackingVolume>(Acts::Transform3::Identity(),
                                                                                                    std::move(msBounds),
                                                                                                    "Muon Spectrometer Envelope");
    const MuonChamberSet chambers = m_detMgr->getAllChambers();
    std::vector<volumePtr> trackingVolumes{};
    unsigned int layerId = 1;
    for(const MuonGMR4::Chamber* chamber: chambers){
        unsigned int sensitiveId = 1;
        std::pair<unsigned int, unsigned int> chId{layerId,sensitiveId};
        auto [internalVolumes, surfaces] = constructInternals(*geoContext, *chamber, chId);
        std::ranges::for_each(surfaces, [&msTrackingVolume](auto& surf){msTrackingVolume->addSurface(std::move(surf));});
        layerId++;
    }
    return msTrackingVolume;
}

std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> ActsMSTrackingVolumeBuilder::constructInternals(const ActsGeometryContext& /*gctx*/,
                                                                                                    const MuonGMR4::Chamber& mChamber,
                                                                                                    std::pair<unsigned int, unsigned int>& chId) const {

    std::vector<volumePtr> internalVolumes{};
    std::vector<surfacePtr> surfaces{};

    if(!m_constructInternals) return {std::move(internalVolumes), std::move(surfaces)};                                                                                   

    const MuonGMR4::Chamber::ReadoutSet& readoutElements = mChamber.readoutEles();

    for(const MuonGMR4::MuonReadoutElement* ele : readoutElements){
        std::ranges::for_each(ele->getSurfaces(), [&surfaces, &chId](auto& surf){
            surf->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(chId.first).withSensitive(++chId.second));
            surfaces.push_back(surf);});
    }
    return {std::move(internalVolumes), std::move(surfaces)};
}


