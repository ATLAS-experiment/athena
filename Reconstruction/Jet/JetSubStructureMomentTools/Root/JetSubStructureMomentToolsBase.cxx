/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"
#include "JetEDM/JetConstituentFiller.h"

JetSubStructureMomentToolsBase::JetSubStructureMomentToolsBase(const std::string& name) : 
  JetModifierBase(name)
{
  declareProperty("InputContainer",m_inputContainer = "");
  declareProperty("Prefix",        m_prefix         = "");
}

StatusCode JetSubStructureMomentToolsBase::initialize() {

  ATH_MSG_DEBUG("Initializing " << name());
  
  if(!m_inputContainer.empty() && m_prefix.empty()) {
    ATH_MSG_WARNING("No prefix defined, defaulting to " << m_inputContainer);
    m_prefix = m_inputContainer;
  }
  
  if(!m_prefix.empty() && m_prefix.find_last_of('_')+1 != m_prefix.size()) m_prefix += "_";
 
  return StatusCode::SUCCESS;
}


bool JetSubStructureMomentToolsBase::SetupDecoration(fastjet::PseudoJet& pseudojet,const  xAOD::Jet& jet, bool requireJetStructure) const {

  bool decorate = true;
  
  if (m_inputContainer.empty()) {
    if (!checkForConstituents(jet))
      decorate = false;
    else pseudojet = buildPseudoJet(jet, requireJetStructure);
  }
  else {
    std::vector<const xAOD::IParticle*> AssociatedParticles;
    if (!(jet.getAssociatedObjects(m_inputContainer,AssociatedParticles))){
      ATH_MSG_WARNING("InputContainer " << m_inputContainer << " unknown. Exiting..");
      decorate = false;
    }
    ATH_MSG_VERBOSE("Found " << AssociatedParticles.size() << " associated particles");
    if(AssociatedParticles.empty()) decorate = false; 
    else pseudojet = buildPseudoJet(AssociatedParticles);
  }

  return decorate;
}


fastjet::PseudoJet JetSubStructureMomentToolsBase::buildPseudoJet (const xAOD::Jet & jet, bool requireJetStructure) const {
  std::vector<fastjet::PseudoJet> constit_pseudojets = jet::JetConstituentFiller::constituentPseudoJets(jet,true,requireJetStructure);
  return fastjet::join(constit_pseudojets);
}

fastjet::PseudoJet JetSubStructureMomentToolsBase::buildPseudoJet(const std::vector<const xAOD::IParticle*>& iparticles) const {
  std::vector<fastjet::PseudoJet> pjs;
  pjs.reserve(iparticles.size());
  for (const auto *iparticle : iparticles)  
    pjs.emplace_back(iparticle->p4());
  return fastjet::join(pjs);
}

void JetSubStructureMomentToolsBase::print() const {
  ATH_MSG_INFO("Properties for JetSubStructureMomentTool " << name());
  ATH_MSG_INFO("InputContainer: " << m_inputContainer);
  ATH_MSG_INFO("Prefix: " << m_prefix);
}

