/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIT_DECORATOR_ALG_HH
#define HIT_DECORATOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Read and write handle keys
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include <Math/Vector3D.h>


namespace FlavorTagDiscriminants {

  class HitDecoratorAlg: public AthReentrantAlgorithm {
    /** @name HitDecoratorAlg
     *  @brief Decorate hits with their position w.r.t. beamspot (if the 
     *         EventInfo is provided; not for the HLT/EF!), the 
     *         event primary vertex (if the vertices collection is provided),
     *         or the origin (original hit coordiantes).
     */
    
    public:
      HitDecoratorAlg(const std::string& name,
                            ISvcLocator* pSvcLocator );

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;


    private:
      // Read and write handle keys for hits
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitContainerKey {
        this, "hitContainer", "PixelClusters", "Key for hits"};


      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitXKey {
        this, "hitsXRelToBeamspotDecorator", m_HitContainerKey, "HitsXRelToBeamspot", "Key for output hits x coordinate relative to beamspot"};

      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitYKey {
        this, "hitsYRelToBeamspotDecorator", m_HitContainerKey, "HitsYRelToBeamspot", "Key for output hits y coordinate relative to beamspot"};

      SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_OutputHitZKey {
        this, "hitsZRelToBeamspotDecorator", m_HitContainerKey, "HitsZRelToBeamspot", "Key for output hits z coordinate relative to beamspot"};

      SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey {
        this, "vertices", "", "Key for Vertices collection, if using a wedge selection w.r.t. the primary vertex"};

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
        this, "eventInfo", "EventInfo", "Key for EventInfo"};

      StatusCode getEventVertex(const EventContext& ctx, ROOT::Math::XYZVector& vtx) const;
  };
}

#endif
