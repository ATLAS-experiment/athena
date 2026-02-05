/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header file
#include "FlavorTagDiscriminants/HitDecoratorAlg.h"

// Read and write handles
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"


namespace FlavorTagDiscriminants {

  HitDecoratorAlg::HitDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}


  StatusCode HitDecoratorAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name());

    // Initialize EventInfo/vertex/beamspot keys
    if(!m_eventInfoKey.empty() && !m_verticesKey.empty()) {
      ATH_MSG_ERROR("Cannot use multiple Vertex position sources for the local coordinates calculation");
      return StatusCode::FAILURE;
    } else if(!m_eventInfoKey.empty()) {
      ATH_MSG_DEBUG("Calculating local coordinates w.r.t. EventInfo Beamspot");
    } else if(!m_verticesKey.empty()) {
      ATH_MSG_DEBUG("Calculating local coordinates w.r.t. Primary Vertex");
    } else {
      ATH_MSG_DEBUG("Calculating local coordinates w.r.t. detector origin (same coordinates)");
    }

    ATH_CHECK(m_eventInfoKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_verticesKey.initialize(SG::AllowEmpty));


    // Initialize hit keys
    ATH_CHECK(m_HitContainerKey.initialize());
    ATH_CHECK(m_OutputHitXKey.initialize());
    ATH_CHECK(m_OutputHitYKey.initialize());
    ATH_CHECK(m_OutputHitZKey.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode HitDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name());

    // Get event vertex
    ROOT::Math::XYZVector vtx;
    ATH_CHECK(getEventVertex(ctx, vtx));

    // Read out hits
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> hits (m_HitContainerKey, ctx);
    if(!hits.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve hit container with key " << m_HitContainerKey.key());
      return StatusCode::FAILURE;
    }

    // Set up hit decorators
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitX (m_OutputHitXKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitY (m_OutputHitYKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitZ (m_OutputHitZKey, ctx);

    // Calculate relative hit position to the vertex and decorate it to hits container
    for(const xAOD::TrackMeasurementValidation* hit : *hits) {
      correctedHitX(*hit) = hit->globalX() - vtx.X();
      correctedHitY(*hit) = hit->globalY() - vtx.Y();
      correctedHitZ(*hit) = hit->globalZ() - vtx.Z();
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode HitDecoratorAlg::getEventVertex(const EventContext& ctx, ROOT::Math::XYZVector& vtx) const {
    vtx.SetXYZ(0, 0, 0);

    if(!m_eventInfoKey.empty()) {
      // Read out event info
      SG::ReadHandle<xAOD::EventInfo> eventInfoHandle(m_eventInfoKey, ctx);
      if(!eventInfoHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve event info container with key " << m_eventInfoKey.key());
        return StatusCode::FAILURE;
      }

      vtx.SetXYZ(
        eventInfoHandle->beamPosX(),
        eventInfoHandle->beamPosY(),
        eventInfoHandle->beamPosZ()
      );

    } else if(!m_verticesKey.empty()) {
      // Read out vertices collection
      SG::ReadHandle<xAOD::VertexContainer> verticesHandle(m_verticesKey, ctx);
      if(!verticesHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve vertices container with key " << m_verticesKey.key());
        return StatusCode::FAILURE;
      }

      if(verticesHandle->empty()) {
        ATH_MSG_ERROR("Empty primary vertices container");
        return StatusCode::FAILURE;
      }
      
      const xAOD::Vertex* pv = nullptr;
      for(const xAOD::Vertex *vertex : *verticesHandle) {
        if(vertex->vertexType() == xAOD::VxType::PriVtx) {
          pv = vertex;
          break;
        }
      }
      // If no PV was found, fallback to the beamspot (should be the first vertex in the collection)
      if(!pv) pv = verticesHandle->front();

      vtx.SetXYZ(
        pv->x(),
        pv->y(),
        pv->z()
      );
    }
    
    return StatusCode::SUCCESS;
  }
}
