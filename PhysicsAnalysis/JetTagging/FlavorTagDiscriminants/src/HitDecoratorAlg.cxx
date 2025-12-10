/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    ATH_MSG_INFO( "Initializing " << name());

    // Initialize reader
    ATH_CHECK( m_eventInfoKey.initialize() );
    ATH_CHECK( m_HitContainerKey.initialize() );

    // Initialize decorator
    ATH_CHECK( m_OutputHitXKey.initialize() );
    ATH_CHECK( m_OutputHitYKey.initialize() );
    ATH_CHECK( m_OutputHitZKey.initialize() );

    return StatusCode::SUCCESS;
  }


  StatusCode HitDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name());
    // Read out event info
    SG::ReadHandle<xAOD::EventInfo> event_info(m_eventInfoKey, ctx);
    if ( !event_info.isValid() ) {
      ATH_MSG_ERROR("Failed to retrieve event info container with key " << m_eventInfoKey.key() );
      return StatusCode::FAILURE;
    }

    // Read out hits
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> hits (m_HitContainerKey, ctx);
    if ( !hits.isValid() ) {
      ATH_MSG_ERROR("Failed to retrieve hit container with key " << m_HitContainerKey.key() );
      return StatusCode::FAILURE;
    }

    // Set up decorator
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitX (m_OutputHitXKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitY (m_OutputHitYKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> correctedHitZ (m_OutputHitZKey, ctx);

    // Construct beamspot vector
    const xAOD::EventInfo& ei = *event_info;

    // Calculate relative hit position to beamsport and decorate it to hits container
    for (const auto *hit: *hits) {
      float localX = hit->globalX() - ei.beamPosX();
      float localY = hit->globalY() - ei.beamPosY();
      float localZ = hit->globalZ() - ei.beamPosZ();
      
      correctedHitX(*hit) = localX;
      correctedHitY(*hit) = localY;
      correctedHitZ(*hit) = localZ;

    }
    
    return StatusCode::SUCCESS;
  }


  StatusCode HitDecoratorAlg::finalize() {
    return StatusCode::SUCCESS;
  }
}
