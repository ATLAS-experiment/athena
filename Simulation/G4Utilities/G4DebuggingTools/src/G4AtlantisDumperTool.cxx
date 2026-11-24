/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlantisDumperTool.h"

namespace G4UA
{

  std::unique_ptr<G4AtlantisDumper>
  G4AtlantisDumperTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a G4AtlasDumper action");
    G4AtlantisDumper::Config config;
    config.tedep_cut = m_tedep_cut;
    config.length_cut = m_length_cut;
    config.p_cut = m_p_cut;
    config.ke_cut = m_ke_cut;
    auto action = std::make_unique<G4AtlantisDumper>(config);
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
