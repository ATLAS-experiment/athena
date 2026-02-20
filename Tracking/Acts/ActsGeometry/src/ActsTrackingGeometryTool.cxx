/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"

#include "Acts/Geometry/TrackingGeometry.hpp"


using namespace ActsTrk;

StatusCode ActsTrackingGeometryTool::initialize() {
    ATH_MSG_DEBUG(name() << " initializing");
    if (parent() != toolSvc()) {
        ATH_MSG_ERROR("The tool is initialized as a private tool but should be public");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    ATH_CHECK(m_rchk.initialize());
    m_detIdMap = createDetectorElementToGeoIdMap();
    if (!m_detIdMap) {
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

std::shared_ptr<const Acts::TrackingGeometry> ActsTrackingGeometryTool::trackingGeometry() const {
    return m_trackingGeometrySvc->trackingGeometry();
}
const ActsTrk::DetectorElementToActsGeometryIdMap* ActsTrackingGeometryTool::surfaceIdMap() const {
    return m_detIdMap.get();
}

const GeometryContext& ActsTrackingGeometryTool::getGeometryContext(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Creating alignment context for event");
    const GeometryContext* geoCtx{nullptr};
    if (!SG::get(geoCtx, m_rchk, ctx).isSuccess()) {
        ATH_MSG_ERROR("Creating alignment context failed: read cond handle invalid!");
    }
    return *geoCtx;
}

const GeometryContext& ActsTrackingGeometryTool::getNominalGeometryContext() const {
     return m_trackingGeometrySvc->getNominalContext();
}


std::unique_ptr<ActsTrk::DetectorElementToActsGeometryIdMap> 
    ActsTrackingGeometryTool::createDetectorElementToGeoIdMap() const {
    // create map from
    auto detector_element_to_geoid = std::make_unique<DetectorElementToActsGeometryIdMap>();

    struct Counter{ 
        unsigned n_sensitive_elements{0};
        unsigned n_detector_elements{0};
        unsigned n_wrong_type{0};
    };
    Counter counter {};
    trackingGeometry()->visitSurfaces([this, &counter, &detector_element_to_geoid](const Acts::Surface *surface) {
        if (!surface || !surface->isSensitive()) {
            ++counter.n_wrong_type;
            return;
        }
        ++counter.n_sensitive_elements;
        const auto* detEl = dynamic_cast<const IDetectorElementBase*>(surface->surfacePlacement());
        if (!detEl) {           
            return;
        }

        auto insert_id = [&detector_element_to_geoid, &surface, &counter](const xAOD::UncalibMeasType type,
                                                                          const IdentifierHash& hash) {
            detector_element_to_geoid->insert(std::make_pair(makeDetectorElementKey(type, hash),
                                                             DetectorElementToActsGeometryIdMap::makeValue(surface->geometryId())));
            ++counter.n_detector_elements;
        };
        switch(detEl->detectorType()) {
            using enum DetectorType;
            case Pixel:
                insert_id(xAOD::UncalibMeasType::PixelClusterType,
                          dynamic_cast<const ActsDetectorElement*>(detEl)->identifyHash());
                break;
            case Sct:
                insert_id(xAOD::UncalibMeasType::StripClusterType,
                          dynamic_cast<const ActsDetectorElement*>(detEl)->identifyHash());
                break;
            case Hgtd:
                insert_id(xAOD::UncalibMeasType::HGTDClusterType,
                         dynamic_cast<const ActsDetectorElement*>(detEl)->identifyHash());
                break;
            case Trt: {
                break;
            }
            /// Muon system
            case Mdt:
            case Rpc:
            case Tgc:
            case Csc:
            case Mm:
            case sTgc:{
                // surface map not needed for the muon detectors
               ++counter.n_detector_elements; 
               break;
            }
            case UnDefined:
                ATH_MSG_ERROR("Undefined element encountered");
                counter.n_detector_elements = 0;
                return;
        }
    }, true /*sensitive surfaces*/);
    ATH_MSG_INFO( "Surfaces without associated detector elements " << (counter.n_sensitive_elements -counter.n_detector_elements)
                << " (with " << counter.n_detector_elements << ")" );
    if (counter.n_sensitive_elements > 0 && 
        counter.n_detector_elements==0) {
        ATH_MSG_ERROR( "No surface with associated detector element" );
        return nullptr;
    }
    if (counter.n_wrong_type>0) {
        ATH_MSG_WARNING( "Surfaces associated to detector elements not of type Trk::TrkDetElementBase :" << counter.n_wrong_type);
    }
    return detector_element_to_geoid;
}