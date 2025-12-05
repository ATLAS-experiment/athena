/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "AssociationUtils/TauJetOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  TauJetOverlapTool::TauJetOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("BJetLabel", m_bJetLabel = "",
                    "Input b-jet flag. Disabled by default.");
    declareProperty("DR", m_dR = 0.2, "Maximum dR for overlap match");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode TauJetOverlapTool::initializeDerived()
  {
    // Initialize the b-jet helper
    if(!m_bJetLabel.empty()) {
      ATH_MSG_DEBUG("Configuring btag-aware OR with btag label: " << m_bJetLabel);
      resetAccessor (m_accessors->m_bJetAcc, *m_accessors, m_bJetLabel);
    }

    // Initialize the dR matcher
    m_dRMatcher = std::make_unique<DeltaRMatcher>(m_dR, m_useRapidity);
    ATH_CHECK (m_dRMatcher->setObjectTypes (xAODType::ObjectType::Jet, xAODType::ObjectType::Tau));
    addSubtool(*m_dRMatcher);

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauJetOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::JetContainer>(cont1, "First container arg is not of type JetContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::TauJetContainer>(cont2, "Second container arg is not of type TauJetContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauJetOverlapTool::
  internalFindOverlaps(columnar::Particle1Range jets,
                       columnar::Particle2Range taus) const
  {
    ATH_MSG_DEBUG("Removing overlapping taus and jets");
    auto& acc = *m_accessors;

    // Initialize output decorations if necessary
    initializeDecorations(taus);
    initializeDecorations(jets);

    // TODO: add anti-tau support.

    // Remove non-btagged jets that overlap with taus.
    for(const auto tau : taus){
      if(!isSurvivingObject(tau)) continue;
      for(const auto jet : jets){
        if(!isSurvivingObject(jet)) continue;

        // Don't reject user-defined b-tagged jets
        if(!m_bJetLabel.empty() && acc.m_bJetAcc(jet)) continue;

        if(m_dRMatcher->objectsMatch(tau, jet)){
          ATH_CHECK( handleOverlap(jet, tau) );
        }
      }
    }

    // Remove taus that overlap with remaining jets
    for(const auto jet : jets){
      if(!isSurvivingObject(jet)) continue;
      for(const auto tau : taus){
        if(!isSurvivingObject(tau)) continue;

        if(m_dRMatcher->objectsMatch(jet, tau)) {
          ATH_CHECK( handleOverlap(tau, jet) );
        }
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
