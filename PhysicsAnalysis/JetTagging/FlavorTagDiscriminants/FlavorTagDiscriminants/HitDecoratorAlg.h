/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIT_DECORATOR_ALG_HH
#define HIT_DECORATOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Read and write handle keys
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"



namespace FlavorTagDiscriminants {

  class HitDecoratorAlg: public AthReentrantAlgorithm {

    public:

      HitDecoratorAlg(const std::string& name,
                            ISvcLocator* pSvcLocator );

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ) const override;
      virtual StatusCode finalize() override;


    private:

      // Read and write handle keys
      SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_HitContainerKey {
        this,"hitContainer","PixelClusters","Key for hits"};

      SG::ReadHandleKey< xAOD::EventInfo > m_eventInfoKey {
        this,"eventInfo","EventInfo","Key for EventInfo"};

      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitXKey {
        this, "hitsXRelToBeamspotDecorator", m_HitContainerKey, "HitsXRelToBeamspot", "Key for output hits x coordinate relative to beamspot"};

      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitYKey {
        this, "hitsYRelToBeamspotDecorator", m_HitContainerKey, "HitsYRelToBeamspot", "Key for output hits y coordinate relative to beamspot"};

      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitZKey {
        this, "hitsZRelToBeamspotDecorator", m_HitContainerKey, "HitsZRelToBeamspot", "Key for output hits z coordinate relative to beamspot"};

  };
}

#endif
