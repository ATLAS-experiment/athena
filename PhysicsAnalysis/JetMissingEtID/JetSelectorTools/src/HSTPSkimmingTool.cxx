#include "HSTPSkimmingTool.h"

StatusCode HSTPSkimmingTool::initialize()
{
  return StatusCode::SUCCESS;
}

bool HSTPSkimmingTool::eventPassesFilter(const EventContext& ctx) const {
  
  if (!m_doHSTPFiltering) return true;
  
  const xAOD::JetContainer* truthJets = nullptr;
  const xAOD::JetContainer* truthPileupJets = nullptr;

  SG::ReadHandle<xAOD::JetContainer> truthJetsDressedWZ{ "AntiKt4TruthDressedWZJets", ctx};
  if (truthJetsDressedWZ.isValid()) { truthJets = truthJetsDressedWZ.cptr();} 
  else {
    SG::ReadHandle<xAOD::JetContainer> truthJetsWZ{ "AntiKt4TruthWZJets", ctx };
    if (truthJetsWZ.isValid()) { truthJets = truthJetsWZ.cptr();} 
    else { SG::ReadHandle<xAOD::JetContainer> truthJetsDefault{ "AntiKt4TruthJets", ctx};
      if (truthJetsDefault.isValid()) { truthJets = truthJetsDefault.cptr(); }
    }
  }

  // HSTP filter enabled but failed to retrieve truth jet collection. Event will be accepted.
  if (!truthJets) return true;
  

  // if HSTP filter enabled but failed to retrieve truth pile-up jet collection. Event will be accepted.
  SG::ReadHandle<xAOD::JetContainer> truthPileupJetsHandle{ "InTimeAntiKt4TruthJets", ctx };
  if (truthPileupJetsHandle.isValid()) { truthPileupJets = truthPileupJetsHandle.cptr();}
  else return true; 

  constexpr double jetThreshold = 5000.0;
  double maxHardScatterJetPt = jetThreshold;

  for (const xAOD::Jet* jet : *truthJets) {
    if (jet->pt() > maxHardScatterJetPt) {
      maxHardScatterJetPt = jet->pt();
    }
  }

  // Reject the event if any pile-up jet has a larger pT.
  for (const xAOD::Jet* pileupJet : *truthPileupJets) {
    if (pileupJet->pt() > maxHardScatterJetPt) {
      ATH_MSG_DEBUG("Event rejected by HSTP filter");
      return false;
    }
  }

  ATH_MSG_DEBUG("Event passed HSTP filter");
  return true;
}
