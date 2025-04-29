/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//Include header file
#include "FlavorTagDiscriminants/JetHitAssociationAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

//Include some helpful ROOT objects here.
#include "math.h"


namespace FlavorTagDiscriminants {

  JetHitAssociationAlg::JetHitAssociationAlg(const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc){}


  StatusCode JetHitAssociationAlg::initialize() {
    
    ATH_MSG_INFO( "Initializing " << name() );

    // Initialize reader
    ATH_CHECK(m_inputPixHitCollectionKey.initialize());
    ATH_CHECK(m_jetCollectionKey.initialize());

    // Initialize decoration reader
    m_HitsXRelToBeamspotKey = m_inputPixHitCollectionKey.key() + "." + m_HitsXRelToBeamspotKey.key();
    ATH_CHECK(m_HitsXRelToBeamspotKey.initialize());
    m_HitsYRelToBeamspotKey = m_inputPixHitCollectionKey.key() + "." + m_HitsYRelToBeamspotKey.key();
    ATH_CHECK(m_HitsYRelToBeamspotKey.initialize());

    // Initialize decorator
    m_hitAssociationKey = m_jetCollectionKey.key() + "." + m_hitAssociationKey.key();
    CHECK(m_hitAssociationKey.initialize());
    
    return StatusCode::SUCCESS;
  }


  StatusCode JetHitAssociationAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name());
    
    // Read out jets
    SG::ReadHandle<xAOD::JetContainer> jetReadHandle(m_jetCollectionKey, ctx);
    if ( !jetReadHandle.isValid() ) {
      ATH_MSG_ERROR("Failed to retrieve jet container with key " << m_jetCollectionKey.key() );
      return StatusCode::FAILURE;
    }
    
    std::vector<const xAOD::Jet*> jets;
    jets.reserve( jetReadHandle->size() );
    for (const xAOD::Jet *jet : *jetReadHandle) {
      jets.push_back(jet);
    }

    // Read out hits relative to beamspot
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, float> HitsXRelToBeamspot(m_HitsXRelToBeamspotKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, float> HitsYRelToBeamspot(m_HitsYRelToBeamspotKey, ctx);
    
    // Calculate hit phi values
    std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>> hitCoord;

    for (const auto* hit : *HitsXRelToBeamspot) {
        auto localX = HitsXRelToBeamspot(*hit);
        auto localY = HitsYRelToBeamspot(*hit);
        float phi = std::atan2(localY, localX);
        hitCoord.emplace_back(phi,hit);
    }

    // Set up element link
    SG::WriteDecorHandle<xAOD::JetContainer,std::vector< ElementLink<xAOD::TrackMeasurementValidationContainer> > > hitAssociation (m_hitAssociationKey, ctx);
    
    // Loop over jets
    for (const xAOD::Jet *jet : jets) {
      std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>> closeHits;
      std::vector<ElementLink<xAOD::TrackMeasurementValidationContainer> > vectorEL; 
      double jetPhi = jet->phi();

      // Compare phi values of hits and jets
      for (const auto& [hitPhi, hit]: hitCoord) {
        float dPhi = std::abs(jetPhi-hitPhi);
        while (dPhi > M_PI) {dPhi -= 2. * M_PI; dPhi = std::abs(dPhi);}

        if (dPhi < m_dPhiHitToJet) {
          closeHits.emplace_back(dPhi, hit);
        }
      }

      // Sort hits by dPhi and associate maximal m_maxHits hits to each jet
      std::sort(closeHits.begin(), closeHits.end(),[](const auto& p1, const auto& p2) { return p1 < p2; });
      closeHits.resize(std::min(int(m_maxHits), int(closeHits.size())));

      for (const auto& [phi, hit]: closeHits) {
        vectorEL.push_back(ElementLink<xAOD::TrackMeasurementValidationContainer>(m_HitsXRelToBeamspotKey.key(), hit->index()));
      }

      // Decorate the ElementLinks of hits to jet
      hitAssociation(*jet) = std::move(vectorEL);
    }

    return StatusCode::SUCCESS;
  }


  StatusCode JetHitAssociationAlg::finalize() {  
    return StatusCode::SUCCESS;
  }

}
