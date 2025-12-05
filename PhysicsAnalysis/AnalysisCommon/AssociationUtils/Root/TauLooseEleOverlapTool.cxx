/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>

// Framework includes
#include "AthContainers/ConstDataVector.h"

// Local includes
#include "AssociationUtils/TauLooseEleOverlapTool.h"

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  TauLooseEleOverlapTool::TauLooseEleOverlapTool(const std::string& name)
    : BaseOverlapTool(name)
  {
    declareProperty("DR", m_maxDR = 0.2,
                    "Delta-R cone for flagging overlaps");
    declareProperty("UseRapidity", m_useRapidity = true,
                    "Calculate delta-R using rapidity");
    declareProperty("ElectronID", m_eleID = "DFCommonElectronsLHLoose",
                    "Loose electron ID selection string");
    declareProperty("AltElectronID", m_altEleID = "",
                    "Optional alternate ID in case primary one unavailable");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode TauLooseEleOverlapTool::initializeDerived()
  {
    // Initialize the dR matcher
    m_dRMatcher = std::make_unique<DeltaRMatcher>(m_maxDR, m_useRapidity);
    ATH_CHECK (m_dRMatcher->setObjectTypes (xAODType::ObjectType::Tau, xAODType::ObjectType::Electron));
    addSubtool(*m_dRMatcher);

    resetAccessor(m_accessors->m_eleIDAcc, *m_accessors, m_eleID, {.isOptional = !m_eleID.empty()});
    if (!m_altEleID.empty())
      resetAccessor(m_accessors->m_altEleIDAcc, *m_accessors, m_altEleID, {.isOptional = true});

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauLooseEleOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    // Check the container types
    ATH_CHECK( checkForXAODContainer<xAOD::TauJetContainer>(cont1, "First container arg is not of type TauJetContainer!") );
    ATH_CHECK( checkForXAODContainer<xAOD::ElectronContainer>(cont2, "Second container arg is not of type ElectronContainer!") );

    ATH_CHECK( internalFindOverlaps(cont1, cont2) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode TauLooseEleOverlapTool::
  internalFindOverlaps(columnar::Particle1Range taus,
                       columnar::Particle2Range electrons) const
  {
    ATH_MSG_DEBUG("Removing taus from loose electrons");

    // Initialize output decorations if necessary
    initializeDecorations(taus);
    initializeDecorations(electrons);

    // Loop over loose surviving electrons
    for(auto electron : electrons){
      if(isRejectedObject(electron)) continue;

      // Check the electron ID
      bool passID = false;
      ATH_CHECK( checkElectronID(electron, passID) );
      if(!passID) continue;

      // Loop over surviving taus
      for(auto tau : taus){
        if(!isSurvivingObject(tau)) continue;

        // Test for overlap
        if(m_dRMatcher->objectsMatch(electron, tau)){
          ATH_CHECK( handleOverlap(tau, electron) );
        }
      }
    }  

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Loose electron criteria
  //---------------------------------------------------------------------------
  StatusCode TauLooseEleOverlapTool::
  checkElectronID(columnar::Particle2Id electron, bool& pass) const
  {
    auto& acc = *m_accessors;
    // Try the configured ID string.
    if (acc.m_eleIDAcc.isAvailable(electron)) {
      pass = acc.m_eleIDAcc(electron);
    } else {
      // If the ID wasn't found, try the fallback ID if provided.
      if(!m_altEleID.empty()) {
        if (acc.m_altEleIDAcc.isAvailable(electron)) {
          pass = acc.m_altEleIDAcc(electron);
        } else {
          ATH_MSG_ERROR("Electron IDs unavailable: " <<
                        m_eleID << ", " << m_altEleID);
          return StatusCode::FAILURE;
        }
      } else {
        ATH_MSG_ERROR("Electron ID unavailable: " << m_eleID);
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace ORUtils
