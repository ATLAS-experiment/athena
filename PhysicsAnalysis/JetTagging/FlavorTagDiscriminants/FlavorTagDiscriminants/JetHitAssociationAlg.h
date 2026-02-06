/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_HIT_ASSOCIATION_ALG_HH
#define JET_HIT_ASSOCIATION_ALG_HH


// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODBase/IParticleContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Read and write handles
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

// Element links
#include "AthLinks/ElementLink.h"


namespace FlavorTagDiscriminants {

  class JetHitAssociationAlg : public AthReentrantAlgorithm {
    
    public:
      JetHitAssociationAlg(const std::string& name,
                            ISvcLocator* pSvcLocator);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext&) const override;
    

    private:
      // Particle container
      SG::ReadHandleKey<xAOD::IParticleContainer> m_jetCollectionKey {
        this, "jetContainer", "tempEmtopoJets", "Key for particle collection"};

      SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_hitAssociationKey {
        this, "hitAssociation", m_jetCollectionKey, "hitsAssociatedWithJet", "Key for decorating hit links"};


      // Hits input
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_inputHitCollectionKey {
        this, "hitContainer", "PixelClusters", "Key for input hits"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_hitsXRelToVertexKey {
        this, "hitReaderX", m_inputHitCollectionKey, "HitsXRelToBeamspot", "Key for hits x coordinate relative to vertex"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_hitsYRelToVertexKey {
        this, "hitReaderY", m_inputHitCollectionKey, "HitsYRelToBeamspot", "Key for hits x coordinate relative to vertex"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_hitsZRelToVertexKey {
        this, "hitReaderZ", m_inputHitCollectionKey, "HitsZRelToBeamspot", "Key for hits x coordinate relative to vertex"};


      // Jet-Hit association configuration
      Gaudi::Property<float> m_dPhiHitToJet {
        this, "dphiHitToJet", 0.2, "Phi difference between the hit and jet/RoiDescriptor vertex; -1 to disable if using jets selection"};

      Gaudi::Property<float> m_dEtaHitToVertex {
        this, "detaHitToJet", 0.2, "Eta difference between the hit and the RoiDescriptor vertex, if using a wedge selection"};

      Gaudi::Property<float> m_dZHitToVertex {
        this, "dzHitToVertex", 180, "Z difference between the hit and the RoiDescriptor vertex, if using a wedge selection"};

      Gaudi::Property<bool> m_useWedgeSelection {
        this, "useWedgeSelection", false, "Use an RoIDescriptor wedge selection"};

      Gaudi::Property<bool> m_includeBarrel {
        this, "includeBarrelHits", true, "Include barrel hits"};

      Gaudi::Property<bool> m_includeEndcap {
        this, "includeEndcapHits", true, "Include endcap hits"};

      Gaudi::Property<int> m_maxHits {
        this, "maxHits", 200, "Maximum number of total hits"};

      Gaudi::Property<bool> m_removeBadIDPixelHits{
        this, "removeBadIDPixelHits", false, "Flag for removing bad ID Pixel hits (only valid if running over the ID PixelClusters collection)"};


      // Helper methods
      const std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>>
      getJetHits(const xAOD::IParticle* jet,
                    const std::vector<std::pair<TLorentzVector, const xAOD::TrackMeasurementValidation*>>& hits) const;

      bool isGoodIDPixelHit(const xAOD::TrackMeasurementValidation* hit) const;
  };

}

#endif
