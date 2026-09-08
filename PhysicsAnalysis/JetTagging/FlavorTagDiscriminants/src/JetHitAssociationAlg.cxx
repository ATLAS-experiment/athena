/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Include header file
#include "FlavorTagDiscriminants/JetHitAssociationAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"

// RoIDescriptor for wedge selections
#include "RoiDescriptor/RoiDescriptor.h"

//Include some helpful ROOT objects here.

#include "CxxUtils/phihelper.h"
#include <cmath>
#include <optional>
#include <algorithm> //sort, min, max


namespace FlavorTagDiscriminants {

  JetHitAssociationAlg::JetHitAssociationAlg(const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}


  StatusCode JetHitAssociationAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name());

    // Initialize jet keys
    ATH_CHECK(m_jetCollectionKey.initialize());
    CHECK(m_hitAssociationKey.initialize());

    ATH_CHECK(m_wedgeZKey.initialize(!m_wedgeZKey.empty()));

    // Initialize hits position decoration keys
    ATH_CHECK(m_inputHitCollectionKey.initialize());
    ATH_CHECK(m_hitsXRelToVertexKey.initialize());
    ATH_CHECK(m_hitsYRelToVertexKey.initialize());
    ATH_CHECK(m_hitsZRelToVertexKey.initialize());

    if(m_useDRCone) {
      if(m_dRHitToJet <= 0) {
        ATH_MSG_FATAL("Invalid dR cone size!");
        return StatusCode::FAILURE;
      }
    }
    else if(m_dPhiHitToJet <= 0 || m_dEtaHitToVertex <= 0 || m_dZHitToVertex <= 0) {
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

    std::optional<SG::ReadDecorHandle<xAOD::IParticleContainer, float>> wedgeZ;
    if(!m_wedgeZKey.empty()) wedgeZ.emplace(m_wedgeZKey, ctx);


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
    std::vector<Hit> hits;
    hits.reserve(hitsHandle->size());
    for(const xAOD::TrackMeasurementValidation* hit : *hitsHandle) {
      const int hit_bec = bec(*hit);
      if(hit_bec == 0 && !m_includeBarrel) continue;
      if(hit_bec != 0 && !m_includeEndcap) continue;

      const float hx = x(*hit);
      const float hy = y(*hit);
      hits.push_back({hit, std::atan2(hy, hx), z(*hit), std::sqrt(hx*hx + hy*hy)});
    }

    // Loop over jets
    for(const xAOD::IParticle* jet : *jetReadHandle) {

      double zed = 0.0;
      if(wedgeZ) {
        zed = (*wedgeZ)(*jet);
        if(!std::isfinite(zed)) {
          ATH_MSG_WARNING("Wedge z-centre is not finite, dropping hits for this jet.");
          hitAssociation(*jet) = {};
          continue;
        }
      }

      // Create links and decorate the jet
      std::vector<ElementLink<xAOD::TrackMeasurementValidationContainer>> links;
      for(const auto& [dist, hit] : getJetHits(jet, hits, zed)) {
        links.push_back(ElementLink<xAOD::TrackMeasurementValidationContainer>(
          m_inputHitCollectionKey.key(),
          hit->index(),
          ctx
        ));
      }

      hitAssociation(*jet) = std::move(links);
    }

    return StatusCode::SUCCESS;
  }


  const std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>>
  JetHitAssociationAlg::getJetHits(const xAOD::IParticle* jet,
                                    const std::vector<Hit>& hits,
                                    double zed) const {
    const double eta = jet->eta();
    const double phi = jet->phi();

    std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>> ret;

    if(m_useDRCone) {
      for(const Hit& hit : hits) {
        const float dEta = eta - std::asinh((hit.z - zed) / hit.r);
        const float dPhi = CxxUtils::wrapToPi(phi - hit.phi);

        const float dR = std::sqrt(dEta * dEta + dPhi * dPhi);
        if(dR > m_dRHitToJet) continue;

        ret.emplace_back(dR, hit.original_hit);
      }
    }
    else {
      const RoiDescriptor roi(
        eta, eta - m_dEtaHitToVertex, eta + m_dEtaHitToVertex,
        phi, CxxUtils::wrapToPi(phi - m_dPhiHitToJet), CxxUtils::wrapToPi(phi + m_dPhiHitToJet),
        zed, zed - m_dZHitToVertex, zed + m_dZHitToVertex
      );

      for(const Hit& hit : hits) {
        if(!RoiUtil::contains(roi, hit.z, hit.r, hit.phi)) continue;

        ret.emplace_back(std::abs(CxxUtils::wrapToPi(phi - hit.phi)), hit.original_hit);
      }
    }

    // If a maximum number of hits has been provided, sort them by distance and truncate the vector
    if(m_maxHits > 0) {
      std::sort(ret.begin(), ret.end(), [](const auto& h1, const auto& h2) -> bool { return h1.first < h2.first; });
      ret.resize(std::min(static_cast<unsigned int>(m_maxHits), static_cast<unsigned int>(ret.size())));
    }

    return ret;
  }

}
