/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaDebugStackingAction.h"
#include "AthenaStackingActionTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode AthenaStackingActionTool::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() );
    ATH_MSG_DEBUG( "KillAllNeutrinos: " << m_killAllNeutrinos.value() );
    ATH_MSG_DEBUG( "PhotonEnergyCut: " << m_photonEnergyCut.value() );
    ATH_MSG_DEBUG( "RussianRouletteNeutronThreshold: " << m_russianRouletteNeutronThreshold.value() );
    ATH_MSG_DEBUG( "RussianRouletteNeutronWeight: " << m_russianRouletteNeutronWeight.value() );
    ATH_MSG_DEBUG( "RussianRoulettePhotonThreshold: " << m_russianRoulettePhotonThreshold.value() );
    ATH_MSG_DEBUG( "RussianRoulettePhotonWeight: " << m_russianRoulettePhotonWeight.value() );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<AthenaStackingAction>
  AthenaStackingActionTool::makeAndFillAction(G4AtlasUserActions& actionLists)
  {
    ATH_MSG_DEBUG("Creating an AthenaStackingAction");
    // Create and configure the action plugin.
    AthenaStackingAction::Config config;
    config.killAllNeutrinos = m_killAllNeutrinos;
    config.photonEnergyCut = m_photonEnergyCut;
    config.applyNRR = m_applyNRR;
    config.russianRouletteNeutronThreshold = m_russianRouletteNeutronThreshold;
    config.russianRouletteNeutronWeight = m_russianRouletteNeutronWeight;
    config.applyPRR = m_applyPRR;
    config.russianRoulettePhotonThreshold = m_russianRoulettePhotonThreshold;
    config.russianRoulettePhotonWeight = m_russianRoulettePhotonWeight;
    config.isISFJob = m_isISFJob;
    std::unique_ptr<AthenaStackingAction> action{nullptr};
    if (m_useDebugAction) {
      action = std::make_unique<AthenaDebugStackingAction>(config);
    } else {
      action = std::make_unique<AthenaStackingAction>(config);
    }
    actionLists.stackingActions.push_back( action.get() );
    return action;
  }

}
