/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/Tool_TauConstituentGetter.h"
#include "PanTauAlgs/TauConstituent.h"
#include "PanTauAlgs/HelperFunctions.h"
#include "PanTauAlgs/Tool_InputConverter.h"
#include "xAODTau/TauJet.h"
#include "xAODPFlow/PFO.h"

PanTau::Tool_TauConstituentGetter::Tool_TauConstituentGetter(const std::string& name) :
  asg::AsgTool(name)
{
}

PanTau::Tool_TauConstituentGetter::~Tool_TauConstituentGetter() = default;

StatusCode PanTau::Tool_TauConstituentGetter::initialize() {

  ATH_MSG_INFO(" initialize()");
  m_init=true;

  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InputConverter, m_Tool_InputConverterName ) );
    
  ATH_CHECK( m_Tool_InputConverter.retrieve() );
    
  return StatusCode::SUCCESS;
}


/**
 * Function to get the PFOs for a given TauJet object (Shots in each PFO etc are collected in "ConvertToTauConstituent")
 */
StatusCode PanTau::Tool_TauConstituentGetter::GetTauConstituents(const xAOD::TauJet* tauJet,
                                                                 std::vector<TauConstituent*>& outputConstituents) const {
    
  //loop over charged PFOs
  for(unsigned int iChrgPFO=0; iChrgPFO<tauJet->nProtoChargedPFOs(); iChrgPFO++) {

    //convert to tau constituent
    PanTau::TauConstituent* curConst = nullptr;
    ATH_CHECK(m_Tool_InputConverter->ConvertToTauConstituent(tauJet->protoChargedPFO( iChrgPFO ), curConst, tauJet) );
    if(curConst == nullptr) {
      ATH_MSG_DEBUG("Problems converting charged PFO into tau constituent -> skip PFO");
      continue;
    }
        
    //add to list of tau constituents
    outputConstituents.push_back(curConst);
        
  }//end loop over charged PFOs
  
  // Pi0 tagged PFOs are not collected!
    
  //loop over neutral PFOs
  for(unsigned int iNeutPFO=0; iNeutPFO<tauJet->nProtoNeutralPFOs(); iNeutPFO++) {
        
    //convert to tau constituent
    PanTau::TauConstituent* curConst = nullptr;
    ATH_CHECK( m_Tool_InputConverter->ConvertToTauConstituent(tauJet->protoNeutralPFO( iNeutPFO ), curConst, tauJet) );
    if(curConst == nullptr) {
      ATH_MSG_DEBUG("Problems converting neutral PFO into tau constituent -> skip PFO");
      continue;
    }
        
    //add to list of tau constituents
    outputConstituents.push_back(curConst);
        
  }//end loop over charged PFOs
       
  return StatusCode::SUCCESS;    
}
