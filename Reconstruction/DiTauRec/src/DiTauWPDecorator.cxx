/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/DiTauWPDecorator.h"

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauWPDecorator::DiTauWPDecorator( const std::string& type, const std::string& name, const IInterface * parent) :
  DiTauToolBase(type, name, parent)
{
  declareInterface<DiTauToolBase > (this);
}

//______________________________________________________________________________
DiTauWPDecorator::~DiTauWPDecorator() = default;

//______________________________________________________________________________
StatusCode DiTauWPDecorator::initialize()
{
  if (!m_ditauContainerName.empty() && m_decorWPs.empty()) {
    ATH_MSG_ERROR("DiTauContainerName is provided but DecorWPNames is empty");
    return StatusCode::FAILURE;
  }
 
  for (size_t wpIndex=0; wpIndex < m_decorWPs.size(); ++wpIndex) {
    m_charDecors.emplace_back(SG::AuxElement::Accessor<char>( m_decorWPs[wpIndex] ));
  }

  return StatusCode::SUCCESS;
}

StatusCode DiTauWPDecorator::executeObj( xAOD::DiTauJet &xDiTau, const EventContext& /*ctx*/) const
{
    const SG::ConstAccessor<float> acc_score(m_scoreName);
    float score = acc_score(xDiTau); 

    // Decorate other WPs
    for (size_t wpIndex=0; wpIndex < m_decorWPs.size(); ++wpIndex) {
      const SG::Accessor<char>& decorator = m_charDecors[wpIndex];
      decorator(xDiTau) = passOmniWP(score,m_decorWPCuts[wpIndex]);
    }

    return StatusCode::SUCCESS;
}

bool DiTauWPDecorator::passOmniWP(float score, float wp_thrshold ) const{ 
    return score > wp_thrshold;
}    

     


