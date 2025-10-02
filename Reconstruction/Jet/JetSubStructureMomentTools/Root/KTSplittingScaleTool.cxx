/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/KTSplittingScaleTool.h"
#include "JetSubStructureUtils/KtSplittingScale.h"
#include "JetSubStructureUtils/ZCut.h"

#include "AsgDataHandles/WriteDecorHandle.h"

KTSplittingScaleTool::KTSplittingScaleTool(const std::string& name) : 
  JetSubStructureMomentToolsBase(name)
{
}

StatusCode KTSplittingScaleTool::initialize() {
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("KTSplittingScaleTool needs to have its input jet container name configured!");
    return StatusCode::FAILURE;
  }

  m_Split12_Key = m_jetContainerName + "." + m_prefix + m_Split12_Key.key();
  m_Split23_Key = m_jetContainerName + "." + m_prefix + m_Split23_Key.key();
  m_Split34_Key = m_jetContainerName + "." + m_prefix + m_Split34_Key.key();

  m_ZCut12_Key = m_jetContainerName + "." + m_prefix + m_ZCut12_Key.key();
  m_ZCut23_Key = m_jetContainerName + "." + m_prefix + m_ZCut23_Key.key();
  m_ZCut34_Key = m_jetContainerName + "." + m_prefix + m_ZCut34_Key.key();

  ATH_CHECK(m_Split12_Key.initialize());
  ATH_CHECK(m_Split23_Key.initialize());
  ATH_CHECK(m_Split34_Key.initialize());

  ATH_CHECK(m_ZCut12_Key.initialize());
  ATH_CHECK(m_ZCut23_Key.initialize());
  ATH_CHECK(m_ZCut34_Key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode KTSplittingScaleTool::modify(xAOD::JetContainer& jets) const
{

  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Split12(m_Split12_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Split23(m_Split23_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Split34(m_Split34_Key);

  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_ZCut12(m_ZCut12_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_ZCut23(m_ZCut23_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_ZCut34(m_ZCut34_Key);
  
  for(const xAOD::Jet* injet : jets){
    fastjet::PseudoJet jet;
    bool decorate = SetupDecoration(jet, *injet);

    float Split12_value = -999, Split23_value = -999, Split34_value = -999,
      ZCut12_value = -999, ZCut23_value = -999, ZCut34_value = -999;

    if (decorate) {
      JetSubStructureUtils::KtSplittingScale split12(1);
      JetSubStructureUtils::KtSplittingScale split23(2);
      JetSubStructureUtils::KtSplittingScale split34(3);
      Split12_value = split12.result(jet);
      Split23_value = split23.result(jet);
      Split34_value = split34.result(jet);
    
      JetSubStructureUtils::ZCut zcut12(1);
      JetSubStructureUtils::ZCut zcut23(2);
      JetSubStructureUtils::ZCut zcut34(3);

      ZCut12_value = zcut12.result(jet);
      ZCut23_value = zcut23.result(jet);
      ZCut34_value = zcut34.result(jet);
    }

    wdh_Split12(*injet) = Split12_value;
    wdh_Split23(*injet) = Split23_value;
    wdh_Split34(*injet) = Split34_value;

    wdh_ZCut12(*injet) = ZCut12_value;
    wdh_ZCut23(*injet) = ZCut23_value;
    wdh_ZCut34(*injet) = ZCut34_value;
  }

  return StatusCode::SUCCESS;
}
