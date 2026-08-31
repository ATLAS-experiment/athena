/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ReadoutGeoDumpAlg.h"


#include "Acts/Geometry/TrackingGeometry.hpp"
#include  "ActsGeometryInterfaces/ISurfacePlacement.h"
namespace ActsTrk {

    StatusCode ReadoutGeoDumpAlg::initialize() {
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        for (const auto type : m_detTypes){
            try{
                m_selTypes.insert(static_cast<DetectorType>(type));
            } catch (...) {
                ATH_MSG_ERROR("Cannot translate "<<m_detTypes);
                return StatusCode::FAILURE;
            }
        }

        return StatusCode::SUCCESS;
    }
    StatusCode ReadoutGeoDumpAlg::finalize() {
        ATH_CHECK(m_tree.write());
         return StatusCode::SUCCESS;
    }
    StatusCode ReadoutGeoDumpAlg::execute(const EventContext& ctx) {
        if (m_executed) {
            return StatusCode::SUCCESS;
        }

        const Acts::GeometryContext tgContext{m_ctxProvider.getGeometryContext(ctx)};

        const auto trackingGeo = m_trackingGeometrySvc->trackingGeometry();
        
        trackingGeo->visitSurfaces([&](const Acts::Surface* surface){
            // We only want alignable surfaces
            if (!surface->isAlignable() || !surface->isSensitive()) {
                return;
            }
            const auto* detEl = dynamic_cast<const ISurfacePlacement*>(surface->surfacePlacement());
            // Somehow it's not a known detector element
            if (!detEl) {
                return;
            }
            // The detector element is from another system
            if(!m_selTypes.empty() && !m_selTypes.count(detEl->detectorType())){
                return;
            }
            //
            m_readoutTransform = surface->localToGlobalTransform(tgContext);
            m_identifier = detEl->identify().get_compact();
            m_detType = Acts::toUnderlying(detEl->detectorType());
            const auto& bounds = surface->bounds();
            m_boundsType = Acts::toUnderlying(bounds.type());
            m_thickness = surface->thickness();
            for (const double val : bounds.values()){
                m_boundValues.push_back(val);
            }
            if (!m_tree.fill(ctx)) {
                THROW_EXCEPTION("Failed to fill the tree");
            }
        }, false);
        m_executed = false;
        return StatusCode::SUCCESS;
    }

}
