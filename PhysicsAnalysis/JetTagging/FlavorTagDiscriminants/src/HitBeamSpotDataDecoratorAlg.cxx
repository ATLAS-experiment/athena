/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header file
#include "FlavorTagDiscriminants/HitBeamSpotDataDecoratorAlg.h"

// Read and write handles
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "StoreGate/ReadCondHandle.h"


namespace FlavorTagDiscriminants {

HitBeamSpotDataDecoratorAlg::HitBeamSpotDataDecoratorAlg(const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}


StatusCode HitBeamSpotDataDecoratorAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name());

    // BS conditions key
    ATH_CHECK(m_beamSpotKey.initialize());

    // Initialize hit keys
    ATH_CHECK(m_HitContainerKey.initialize());
    ATH_CHECK(m_OutputHitXKey.initialize());
    ATH_CHECK(m_OutputHitYKey.initialize());
    ATH_CHECK(m_OutputHitZKey.initialize());

    return StatusCode::SUCCESS;
}


StatusCode HitBeamSpotDataDecoratorAlg::execute(const EventContext& ctx) const {
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

  
StatusCode HitBeamSpotDataDecoratorAlg::getEventVertex(const EventContext& ctx, ROOT::Math::XYZVector& vtx) const {
    // Read out beamspot from conditions
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle(m_beamSpotKey, ctx);
    if(!beamSpotHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve BeamSpot conditions data with key " << m_beamSpotKey.key());
        return StatusCode::FAILURE;
    }
    const InDet::BeamSpotData* beamSpot = *beamSpotHandle;

    vtx.SetXYZ(
        beamSpot->beamPos()[Amg::x],
        beamSpot->beamPos()[Amg::y],
        beamSpot->beamPos()[Amg::z]
    );

    return StatusCode::SUCCESS;
}

}
