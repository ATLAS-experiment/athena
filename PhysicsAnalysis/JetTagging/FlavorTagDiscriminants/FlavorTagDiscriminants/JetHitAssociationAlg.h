/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_HIT_ASSOCIATION_ALG_HH
#define JET_HIT_ASSOCIATION_ALG_HH


// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODEventInfo/EventInfo.h"

// Read and write handles
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

// Element links
#include "AthLinks/ElementLink.h"


namespace FlavorTagDiscriminants {

  class JetHitAssociationAlg : public AthReentrantAlgorithm{
    
    public:
      
      JetHitAssociationAlg (const std::string& name,
                        ISvcLocator* pSvcLocator);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ) const override;
      virtual StatusCode finalize() override;
    
    
    private:

      SG::ReadHandleKey<xAOD::JetContainer> m_jetCollectionKey {
        this,"jetContainer","tempEmtopoJets","Key for jets"};

      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer>  m_inputPixHitCollectionKey{
        this,"hitContainer","PixelClusters","Key for hits"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitsXRelToBeamspotKey{
        this, "hitReaderX","HitsXRelToBeamspot","Key for output hits x coordinate relative to beamspot"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitsYRelToBeamspotKey{
        this, "hitReaderY","HitsYRelToBeamspot","Key for output hits y coordinate relative to beamspot"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitsXRelToPVKey{
        this, "hitReaderXRelPV","HitsXRelToPV","Key for output hits x coordinate relative to primary vertex"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitsYRelToPVKey{
        this, "hitReaderYRelPV","HitsYRelToPV","Key for output hits y coordinate relative to primary vertex"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_HitsZRelToPVKey{
        this, "hitReaderZRelPV","HitsZRelToPV","Key for output hits z coordinate relative to primary vertex"};

      SG::WriteDecorHandleKey< xAOD::JetContainer > m_hitAssociationKey{
        this,"hitAssociation","hitsAssociatedWithJet","Key for decorating links"};
      
      Gaudi::Property<std::string> m_hitMetric{
        this, "HitMetric", "dphi", "Hit Association Metric: 'dphi' or 'dr' to jet"};

      Gaudi::Property <float>  m_maxDistToJet{
        this, "MaxDistToJet", 0.2, "Max distance between hit and jet"};

      Gaudi::Property <int>  m_maxHits{
        this, "maxHits", 200, "Maximum number of hits"};
      
  };
}

#endif