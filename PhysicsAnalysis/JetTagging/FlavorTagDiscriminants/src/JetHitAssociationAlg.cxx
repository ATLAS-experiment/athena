/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Include header file
#include "FlavorTagDiscriminants/JetHitAssociationAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

// RoIDescriptor for wedge selections
#include "RoiDescriptor/RoiDescriptor.h"

//Include some helpful ROOT objects here.
#include "math.h"
#include "CxxUtils/phihelper.h"


namespace FlavorTagDiscriminants {

  JetHitAssociationAlg::JetHitAssociationAlg(const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}


  StatusCode JetHitAssociationAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name());

    // Initialize jet keys
    ATH_CHECK(m_jetCollectionKey.initialize());
    CHECK(m_hitAssociationKey.initialize());

    // Initialize hits position decoration keys
    ATH_CHECK(m_inputHitCollectionKey.initialize());
    ATH_CHECK(m_hitsXRelToVertexKey.initialize());
    ATH_CHECK(m_hitsYRelToVertexKey.initialize());
    ATH_CHECK(m_hitsZRelToVertexKey.initialize());

    if(m_useWedgeSelection && (m_dPhiHitToJet <= 0 || m_dEtaHitToVertex <= 0 || m_dZHitToVertex <= 0)) {
      ATH_MSG_FATAL("Invalid RoiDescriptor size!");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }


  StatusCode JetHitAssociationAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name());

    // Read out jets
    SG::ReadHandle<xAOD::IParticleContainer> jetReadHandle(m_jetCollectionKey, ctx);
    if(!jetReadHandle.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container with key " << m_jetCollectionKey.key());
      return StatusCode::FAILURE;
    }
    // Avoid reading out hits if there are no jets to attach them to
    if(jetReadHandle->empty()) return StatusCode::SUCCESS;

    // Set up jet decorators
    SG::WriteDecorHandle<xAOD::IParticleContainer, std::vector<ElementLink<xAOD::TrackMeasurementValidationContainer>>> hitAssociation(m_hitAssociationKey, ctx);


    // Read out hits (we need to keep the handles open to later build the ElementLinks)
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> hitsHandle(m_inputHitCollectionKey, ctx);
    if(!hitsHandle.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve hits container with key " << hitsHandle.key());
      return StatusCode::FAILURE;
    }

    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, float> x(m_hitsXRelToVertexKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, float> y(m_hitsYRelToVertexKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, float> z(m_hitsZRelToVertexKey, ctx);

    // The bec aux-variable is not declaared through keys to the SG, so it creates dependency errors in the
    // schedules if we try to use a ReadDecorHandle(Key) for it.
    static const SG::AuxElement::ConstAccessor<int> bec("bec");
    
    // Filter input hits
    std::vector<std::pair<TLorentzVector, const xAOD::TrackMeasurementValidation*>> hits;
    for(const xAOD::TrackMeasurementValidation* hit : *hitsHandle) {
      const int hit_bec = bec(*hit);
      if(m_removeBadIDPixelHits && !isGoodIDPixelHit(hit)) continue;
      if(hit_bec == 0 && !m_includeBarrel) continue;
      if(hit_bec != 0 && !m_includeEndcap) continue;

      hits.emplace_back(TLorentzVector(x(*hit), y(*hit), z(*hit), 0), hit);
    }

    // Loop over jets
    for(const xAOD::IParticle* jet : *jetReadHandle) {
      // Create links and decorate the jet
      std::vector<ElementLink<xAOD::TrackMeasurementValidationContainer>> links;
      for(const auto& [dist, hit] : getJetHits(jet, hits)) {
        links.push_back(ElementLink<xAOD::TrackMeasurementValidationContainer>(
          m_inputHitCollectionKey.key(),
          hit->index(),
          ctx
        ));
      }

      hitAssociation(*jet) = links;
    }

    return StatusCode::SUCCESS;
  }


  const std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>>
  JetHitAssociationAlg::getJetHits(const xAOD::IParticle* jet, 
                                    const std::vector<std::pair<TLorentzVector, const xAOD::TrackMeasurementValidation*>>& hits) const {
    const TLorentzVector p4 = jet->p4();

    std::unique_ptr<RoiDescriptor> roi;
    if(m_useWedgeSelection) {
      roi = std::make_unique<RoiDescriptor>(
        jet->eta(), jet->eta() - m_dEtaHitToVertex, jet->eta() + m_dEtaHitToVertex, 
        jet->phi(), jet->phi() - m_dPhiHitToJet, jet->phi() + m_dPhiHitToJet, 
        0, m_dZHitToVertex, m_dZHitToVertex // We're working with coordinates w.r.t. vertex
      );
    }

    // Loop over all hits, and calculate/check their dPhi distances w.r.t. the jet
    std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>> ret;
    for(const auto& [hitP4, hit] : hits) {

      if(m_useWedgeSelection && !RoiUtil::contains(*roi, p4.Z(), p4.Perp(), p4.Phi())) continue;

      const float dPhi = std::abs(CxxUtils::wrapToPi(p4.Phi() - hitP4.Phi()));
      if(!m_useWedgeSelection && m_dPhiHitToJet >= 0 && dPhi > m_dPhiHitToJet) continue;

      ret.emplace_back(dPhi, hit);
    }

    // If a maximum number of hits has been provided, sort them by distance and truncate the vector
    if(m_maxHits > 0) {
      std::sort(ret.begin(), ret.end(), [](const auto& h1, const auto& h2) -> bool { return h1.first < h2.first; });
      ret.resize(std::min(static_cast<unsigned int>(m_maxHits), static_cast<unsigned int>(ret.size())));
    }

    return ret;
  }


  bool JetHitAssociationAlg::isGoodIDPixelHit(const xAOD::TrackMeasurementValidation* hit) const {
    if(!hit) return false;

    static const SG::AuxElement::ConstAccessor<char> isFake("isFake");
    if(isFake(*hit)) return false;

    static const SG::AuxElement::ConstAccessor<int> hasBSError("hasBSError");
    if(hasBSError(*hit)) return false;

    static const SG::AuxElement::ConstAccessor<char> DCSState("DCSState");
    if(DCSState(*hit)) return false;
    
    return true;
  }

}
