/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/QwTool.h"
#include "JetSubStructureUtils/Qw.h"

#include "AsgDataHandles/WriteDecorHandle.h"


QwTool::QwTool(const std::string& myname)
: JetSubStructureMomentToolsBase(myname) {
}

StatusCode QwTool::initialize() {
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("NSubjettinessTool needs to have its input jet container name configured!");
    return StatusCode::FAILURE;
  }

  m_Qw_Key = m_jetContainerName + "." + m_prefix + m_Qw_Key.key();
  ATH_CHECK(m_Qw_Key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode QwTool::modify(xAOD::JetContainer& jets) const {
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Qw(m_Qw_Key);

  for(const xAOD::Jet* injet : jets){
    fastjet::PseudoJet jet;
    bool decorate = SetupDecoration(jet, *injet);
    float qw_value = -999;

    if (decorate) {
      static const JetSubStructureUtils::Qw qw;
      qw_value = qw.result(jet);
    }

    wdh_Qw(*injet) = qw_value;
  }

  return StatusCode::SUCCESS;
}
