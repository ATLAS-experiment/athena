/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackRecorderTool.h"

namespace ActsTrk
{
  MaterialTrackRecorderTool::MaterialTrackRecorderTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface* parent)
    : G4UA::UserActionToolBase<MaterialTrackRecorder>(type, name, parent)
  {
  }

  std::unique_ptr<MaterialTrackRecorder>
  MaterialTrackRecorderTool::makeAndFillAction(G4UA::G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a MaterialTrackRecorder action");

    MaterialTrackRecorder::Config config;
    config.materialTrackCollectionName = m_materialTrackCollectionName.value();

    auto action = std::make_unique<MaterialTrackRecorder>(config);
    actionList.runActions.push_back( action.get() );
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

}
