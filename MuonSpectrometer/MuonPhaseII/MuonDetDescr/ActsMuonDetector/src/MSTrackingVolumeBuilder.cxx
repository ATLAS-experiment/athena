/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSTrackingVolumeBuilder.h"

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "Acts/Geometry/CylinderLayer.hpp"
#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"
#include "Acts/Utilities/BinnedArrayXD.hpp"
#include "Acts/Surfaces/SurfaceArray.hpp"

#include <ranges>

using namespace Muon::MuonStationIndex;

namespace ActsTrk{

    StatusCode MSTrackingVolumeBuilder::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }

    std::shared_ptr<Acts::TrackingVolume> 
        MSTrackingVolumeBuilder::trackingVolume(const Acts::GeometryContext& /*gctx*/,
                                                std::shared_ptr<const Acts::TrackingVolume> insideVolume,
                                                std::shared_ptr<const Acts::VolumeBounds> /*outsideBounds*/) const {
    
    ATH_MSG_INFO("Setup MS Gen-1 tracking volume");
    std::shared_ptr<Acts::VolumeBounds> msBounds{};
    if (insideVolume) {
        msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 14500, 22500, 3200);
    } else {
        msBounds = std::make_unique<Acts::CylinderVolumeBounds>(0, 14500, 22500);
    }
    /// Assign surface IDs
    auto surfCounter = make_array<unsigned, toInt(ChIndex::ChIndexMax)>(0);
    std::vector<std::shared_ptr<const Acts::Surface>> msSurfaces{};
    Acts::SurfaceVector rawSurfs{};
    for (const MuonGMR4::MuonReadoutElement* re : m_detMgr->getAllReadoutElements()) {
        const unsigned chIdx = toInt(re->chamberIndex());
        auto reSurfaces = re->getSurfaces();
        msSurfaces.reserve(msSurfaces.size() + reSurfaces.size());
        for (std::shared_ptr<Acts::Surface>& surf : re->getSurfaces()) {
            surf->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(chIdx + m_firstLayId).withSensitive(++surfCounter[chIdx]));
            msSurfaces.push_back(std::move(surf));
        }
    }

    std::ranges::transform(msSurfaces, std::back_inserter(rawSurfs), [](const auto& s) { return s.get(); });
    auto surfaceArray = std::make_unique<Acts::SurfaceArray>(std::make_unique<Acts::SurfaceArray::SingleElementLookup>(rawSurfs),
                                                            msSurfaces);
  
    auto cylinder = Acts::CylinderLayer::create(Amg::Transform3D::Identity(), 
                                                std::make_shared<Acts::CylinderBounds>(14500, 22500), 
                                                std::move(surfaceArray), 10.*Gaudi::Units::m,
                                                nullptr, Acts::LayerType::active);
    
    auto material = std::make_unique<Acts::HomogeneousVolumeMaterial>(Acts::Material::Vacuum());

    auto layerArray = std::make_unique<Acts::BinnedArrayXD<Acts::LayerPtr>>(cylinder);

    return std::make_shared<Acts::TrackingVolume>(Acts::Transform3::Identity(), std::move(msBounds),
                                                  std::move(material), std::move(layerArray), nullptr,
                                                  Acts::MutableTrackingVolumeVector{},
                                                  "Muon Spectrometer Envelope");
}

}

