/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/JetPileupLabelingTool.h"

#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"


StatusCode JetPileupLabelingTool::initialize(){

  ATH_MSG_INFO("Initializing " << name());
  print();

  m_decIsHSKey = m_jetContainerName + "." + m_decIsHSKey.key();
  m_decIsPUKey = m_jetContainerName + "." + m_decIsPUKey.key();

  ATH_CHECK(m_truthJetsKey.initialize());
#ifndef XAOD_STANDALONE
  if(m_suppressOutputDeps) {
    // For analysis applications, may not enforce scheduling via data deps
      renounce(m_decIsHSKey);
      renounce(m_decIsPUKey);
  }
#endif
  ATH_CHECK(m_decIsHSKey.initialize());
  ATH_CHECK(m_decIsPUKey.initialize());

  return StatusCode::SUCCESS;
}

void JetPileupLabelingTool::print() const {

  ATH_MSG_INFO("Parameters for " << name() << " :");
  ATH_MSG_INFO("HS Truth Jet Container:           " << m_truthJetsKey.key());
  ATH_MSG_INFO("Name of isHS Label Decoration:    " << m_decIsHSKey.key());
  ATH_MSG_INFO("Name of isPU Label Decoration:    " << m_decIsPUKey.key());
}


StatusCode JetPileupLabelingTool::decorate(const xAOD::JetContainer& jets) const {

  // Get collection of HS Truth Jets
  SG::ReadHandle<xAOD::JetContainer> truthHSJetsHandle = SG::makeHandle (m_truthJetsKey);
  if (!truthHSJetsHandle.isValid()){
    ATH_MSG_ERROR("Invalid JetContainer datahandle: " << m_truthJetsKey.key());
    return StatusCode::FAILURE;
  }
  const xAOD::JetContainer* truthHSJets = truthHSJetsHandle.cptr();

  SG::WriteDecorHandle<xAOD::JetContainer, char> isHSJetHandle (m_decIsHSKey);
  SG::WriteDecorHandle<xAOD::JetContainer, char> isPUJetHandle (m_decIsPUKey);

  for (const xAOD::Jet* jet : jets){
      bool isHS = false;
      bool isPU = true;
      for (const xAOD::Jet *truthJet : *truthHSJets) {
          float dr = jet->p4().DeltaR(truthJet->p4());
          if (dr < m_hsMaxDR && truthJet->pt() > m_hsMinPt)
              isHS = true;
          if (dr < m_puMinDR && truthJet->pt() > m_puMinPt)
              isPU = false;
          if (isHS && !isPU)
              break;
      }
      isHSJetHandle(*jet) = isHS;
      isPUJetHandle(*jet) = isPU;
  }

  return StatusCode::SUCCESS;
}

