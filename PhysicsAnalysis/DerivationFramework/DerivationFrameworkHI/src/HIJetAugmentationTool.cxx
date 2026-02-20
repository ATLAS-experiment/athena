/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HIJetAugmentationTool.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include <format>

namespace DerivationFramework
{
  HIJetAugmentationTool::HIJetAugmentationTool(   const std::string& t,
                                                                const std::string& n,
                                                                const IInterface* p ) :   base_class(t,n,p)
  {
  }

  // Destructor
  HIJetAugmentationTool::~HIJetAugmentationTool()
  {
  }

  // Athena initialize and finalize
  StatusCode HIJetAugmentationTool::initialize()
  {
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_hiJet_key.initialize());
    ATH_CHECK(m_caloJet_key.initialize());
    ATH_CHECK(m_jvtUpdateTool.retrieve());
    
    m_jvtMatchedKey = m_hiJet_key.key() + "." + m_jvtMatchedKey.key();
    ATH_CHECK(m_jvtMatchedKey.initialize());
    
    ATH_MSG_INFO("DeltaRJetMatching = "<< m_deltaR.value());
    
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode HIJetAugmentationTool::finalize()
  {

    ATH_CHECK(m_jvtUpdateTool->finalize());

    return StatusCode::SUCCESS;
  }

  double deltaR(double eta1, double eta2, double phi1, double phi2) {
    double deltaPhi = TVector2::Phi_mpi_pi(phi1 - phi2);
    double deltaEta = eta1 - eta2;
    return std::sqrt(deltaEta * deltaEta + deltaPhi * deltaPhi);
  }

  StatusCode HIJetAugmentationTool::addBranches(const EventContext &ctx) const {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

    // Load jet containers
    SG::ReadHandle<xAOD::JetContainer> hiJets(m_hiJet_key, ctx);
    if (!hiJets.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve JetContainer with key " << m_hiJet_key);
      return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::JetContainer> caloJets(m_caloJet_key, ctx);
    if (!caloJets.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve JetContainer with key "
                    << m_caloJet_key);
      return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::JetContainer, float> jvtMatchedHandle(m_jvtMatchedKey,ctx);

    // first loop over calibrated HI jets
    for (const auto *hjet : *hiJets) {
      float mindR = 999.;
      float matchedJvt = -1;
      // second loop over topo jets
      for (const auto *tjet : *caloJets) {
        float newjvt = m_jvtUpdateTool->updateJvt(*tjet);

        // perform the matching
        float dR =
            deltaR(tjet->eta(), hjet->eta(), hjet->phi(), tjet->phi());
        
        if (dR < m_deltaR.value() && dR < mindR) {
          mindR = dR;
          matchedJvt = newjvt;
        }
      }
    
      if (mindR < m_deltaR.value()) {
        (jvtMatchedHandle)(*hjet) = matchedJvt;
      } else {
        (jvtMatchedHandle)(*hjet) = -1; 
      }
      
    }

    return StatusCode::SUCCESS;
  }
}


