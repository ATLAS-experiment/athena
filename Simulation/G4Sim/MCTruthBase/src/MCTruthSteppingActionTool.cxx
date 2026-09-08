/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruthSteppingActionTool.h"


namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  MCTruthSteppingActionTool::
  MCTruthSteppingActionTool(const std::string& type, const std::string& name,
                            const IInterface* parent)
    : UserActionToolBase<MCTruthSteppingAction>(type, name, parent)
  {}

  //---------------------------------------------------------------------------
  // Initialize the tool
  //---------------------------------------------------------------------------
  StatusCode MCTruthSteppingActionTool::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() );
    ATH_CHECK(m_truthRecordSvc.retrieve());
    ATH_CHECK(m_geoIDSvc.retrieve());
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create an MCTruthSteppingAction
  //---------------------------------------------------------------------------
  std::unique_ptr<MCTruthSteppingAction>
  MCTruthSteppingActionTool::makeAndFillAction(G4AtlasUserActions& actionLists)
  {
    ATH_MSG_DEBUG("Constructing an MCTruthSteppingAction");
    auto action = std::make_unique<MCTruthSteppingAction> (
        m_volumeCollectionMap.value(), m_secondarySavingLevel.value(),
        m_subDetVolLevel.value(),
        *m_truthRecordSvc, *m_geoIDSvc,
        msgSvc(), msg().level() );
    actionLists.eventActions.push_back( action.get() );
    actionLists.steppingActions.push_back( action.get() );
    return action;
  }

  /// Calls BeginOfAthenaEvent
  StatusCode MCTruthSteppingActionTool::BeginOfAthenaEvent(HitCollectionMap& hitCollections)
  {
    for(const auto& volCollPair : m_volumeCollectionMap.value()) {
      hitCollections.Emplace<TrackRecordCollection>(volCollPair.second, volCollPair.second);
    }
    return StatusCode::SUCCESS;
  }
  /// Calls EndOfAthenaEvent
  StatusCode MCTruthSteppingActionTool::EndOfAthenaEvent(HitCollectionMap& hitCollections)
  {
    for(const auto& volCollPair : m_volumeCollectionMap.value()) {
      CHECK(hitCollections.Record<TrackRecordCollection>(volCollPair.second));
    }
    return StatusCode::SUCCESS;
  }

} // namespace G4UA
