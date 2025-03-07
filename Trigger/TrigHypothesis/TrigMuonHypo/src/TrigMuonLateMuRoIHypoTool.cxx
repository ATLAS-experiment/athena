/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigMuonLateMuRoIHypoTool.h"

class ISvcLocator;
TrigMuonLateMuRoIHypoTool::TrigMuonLateMuRoIHypoTool(const std::string & type, const std::string & name, const IInterface* parent):
  AthAlgTool(type, name, parent),
  m_decisionId(HLT::Identifier::fromToolName(name)){
}

TrigMuonLateMuRoIHypoTool::~TrigMuonLateMuRoIHypoTool(){
}

StatusCode TrigMuonLateMuRoIHypoTool::initialize(){
  if(m_acceptAll) {
    ATH_MSG_DEBUG("Accepting all the events!");
  } 

  return StatusCode::SUCCESS;
}

bool TrigMuonLateMuRoIHypoTool::decideOnRoIs(TrigMuonLateMuRoIHypoTool::MuonEFInfo& input) const{

  //for pass through mode
  if(m_acceptAll) {
    ATH_MSG_DEBUG("Accept property is set: taking all the events");
    return true;
  }

  //  decision making
  //Get xAOD::MuonContainer from hypotool
  const  TrigRoiDescriptor* lateRoI = input.roi;
  if( !lateRoI ){
    ATH_MSG_DEBUG("Retrieval of RoI descriptor failed");
    return false; // refuse
  }
  else ATH_MSG_DEBUG("found late muon roi");
  return true;	// accept
}
  
StatusCode TrigMuonLateMuRoIHypoTool::decide(std::vector<MuonEFInfo>& toolInput) const {

  for (auto& tool : toolInput){
    if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool.previousDecisionIDs)){
      if(decideOnRoIs(tool)==true){
        ATH_MSG_DEBUG("Passes selection");
        TrigCompositeUtils::addDecisionID(m_decisionId, tool.decision);
      }
      else ATH_MSG_DEBUG("Does not pass selection");
    }
  }

  return StatusCode::SUCCESS;
}
