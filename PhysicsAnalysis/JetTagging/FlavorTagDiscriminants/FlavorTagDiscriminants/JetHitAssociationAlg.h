/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_HIT_ASSOCIATION_ALG_HH
#define JET_HIT_ASSOCIATION_ALG_HH




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
// STL includes
#include <string>
#include <utility> //std::pair
#include <vector>


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

      SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_wedgeZKey {
        this, "wedgeZDecor", "", "Jet decoration holding the wedge z-centre, in the same frame as the hit positions; if unset the wedge is centred on z = 0"};


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
        this, "dphiHitToJet", 0.2, "Phi half-width of the wedge around the jet axis"};

      Gaudi::Property<float> m_dEtaHitToVertex {
        this, "detaHitToJet", 0.2, "Eta half-width of the wedge around the jet axis"};

      Gaudi::Property<float> m_dZHitToVertex {
        this, "dzHitToVertex", 180, "Z half-width [mm] of the wedge around its z-centre"};

      Gaudi::Property<bool> m_useDRCone {
        this, "useDRCone", false, "Select hits in a dR cone around the jet axis instead of the wedge"};

      Gaudi::Property<float> m_dRHitToJet {
        this, "dRHitToJet", 0.4, "Cone size, if useDRCone is set"};

      Gaudi::Property<bool> m_includeBarrel {
        this, "includeBarrelHits", true, "Include barrel hits"};

      Gaudi::Property<bool> m_includeEndcap {
        this, "includeEndcapHits", true, "Include endcap hits"};

      Gaudi::Property<int> m_maxHits {
        this, "maxHits", 200, "Maximum number of total hits; 0 to keep all"};


      struct Hit {
        const xAOD::TrackMeasurementValidation* original_hit;
        float phi;
        float z;
        float r;
      };


      // Helper methods

      // hits must be sorted by phi.
      const std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>>
      getJetHits(const xAOD::IParticle* jet,
                    const std::vector<Hit>& hits,
                    double zed) const;
  };

}

#endif
