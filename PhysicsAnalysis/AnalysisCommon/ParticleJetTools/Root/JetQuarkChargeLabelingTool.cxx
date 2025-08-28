/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/JetQuarkChargeLabelingTool.h"
#include "ParticleJetTools/ParticleJetLabelCommon.h"
#include "xAODJet/JetContainer.h"
#include "AsgMessaging/Check.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

using namespace std;
using namespace xAOD;

JetQuarkChargeLabelingTool::JetQuarkChargeLabelingTool(const std::string& name)
        : AsgTool(name) {
}

StatusCode JetQuarkChargeLabelingTool::initialize()
{
  
  // Initialize output handles
  ATH_MSG_DEBUG("Initializing JetQuarkChargeLabelingTool");
  m_map = m_mapOption.value();
  
  return StatusCode::SUCCESS;
}

StatusCode JetQuarkChargeLabelingTool::decorate(const JetContainer& jets) const
{

  ATH_MSG_VERBOSE("In " << name() << "::decorate()");
  
  SG::AuxElement::Accessor<int> hadronAccessorHandle(m_hadronAccessor.value());
  SG::AuxElement::Accessor<int> partonAccessorHandle(m_partonAccessor.value());
  SG::AuxElement::Decorator<int> chargeDecoratorHandle(m_chargeDecorator.value());
  
  for (const xAOD::Jet* jet: jets) {
    int final_pdgId = -999 ;

    //get hadron pdgId and parton label :
    
    int hadron_pdgId = hadronAccessorHandle(*jet);
    int parton_pdgId = partonAccessorHandle(*jet);
    int negative_pdgId = (-1)*hadron_pdgId;

    if( hadron_pdgId == 0 ){
      final_pdgId = parton_pdgId;
      ATH_MSG_DEBUG("Jet is a light-jet. Associating to the jet the value of the " << m_partonAccessor);
    }else{
      if(m_map.count(hadron_pdgId) >0){
	final_pdgId = m_map.at(hadron_pdgId);
        ATH_MSG_DEBUG("Found associated hadron. Assigning " << final_pdgId << " to jet");
      }else if(m_map.count(negative_pdgId) >0 ){
	final_pdgId = (-1)*m_map.at(negative_pdgId);
	ATH_MSG_DEBUG("Found associated hadron. Assigning " << final_pdgId << " to jet");
      }else{                                                                                                                                                                                                    
	ATH_MSG_DEBUG("Couldn't find the hadron with pdgId " << hadron_pdgId << " in current map. Assigning default " << final_pdgId << " to jet" );
      };
    };

    // Decorate the jet with the charge too
    chargeDecoratorHandle(*jet) = final_pdgId;
  };

  return StatusCode::SUCCESS;
}
