/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIT_BEAMSPOT_DATA_DECORATOR_ALG_HH
#define HIT_BEAMSPOT_DATA_DECORATOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Online beamspot access in the HLT/EF
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "StoreGate/ReadCondHandleKey.h"

// Read and write handle keys
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include <Math/Vector3D.h>


namespace FlavorTagDiscriminants {

class HitBeamSpotDataDecoratorAlg : public AthReentrantAlgorithm {
/** @name HitBeamSpotDataDecoratorAlg
 *  @brief Decorate hits with their position w.r.t. beamspot using BeamSpotData 
 *         conditions.
 */

public:
    HitBeamSpotDataDecoratorAlg(const std::string& name,
                                ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;


private:
    // Read and write handle keys for hits
    SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitContainerKey {
        this, "hitContainer", "PixelClusters", "Key for hits"};

    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_OutputHitXKey {
        this, "hitsXRelToBeamspotDecorator", m_HitContainerKey, "HitsXRelToBeamspot", "Key for output hits x coordinate relative to the vertex"};

    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_OutputHitYKey {
        this, "hitsYRelToBeamspotDecorator", m_HitContainerKey, "HitsYRelToBeamspot", "Key for output hits y coordinate relative to the vertex"};

    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_OutputHitZKey {
        this, "hitsZRelToBeamspotDecorator", m_HitContainerKey, "HitsZRelToBeamspot", "Key for output hits z coordinate relative to the vertex"};


    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
        this, "beamSpotKey", "", "Key for BeamSpot confition data"};

    StatusCode getEventVertex(const EventContext& ctx, ROOT::Math::XYZVector& vtx) const;
};

}

#endif
