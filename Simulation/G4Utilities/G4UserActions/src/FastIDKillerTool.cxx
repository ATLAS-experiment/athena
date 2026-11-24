/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastIDKillerTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  std::unique_ptr<FastIDKiller>
  FastIDKillerTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a FastIDKiller action");
    FastIDKiller::Config config;
    config.R = m_R;
    config.Z = m_Z;
    config.energyCut = m_energyCut;
    config.isDalek = m_isDalek;
    auto action = std::make_unique<FastIDKiller>(config);
    actionList.runActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

  //---------------------------------------------------------------------------
  StatusCode FastIDKillerTool::finalize()
  {
    // Accumulate results across threads
    FastIDKiller::Report report;
    m_actions.accumulate(report, &FastIDKiller::getReport,
                         &FastIDKiller::Report::merge);
    ATH_MSG_INFO( "Fast Inner Detector Killer killed " << report.killCount <<
                  " particles during this run." );
    return StatusCode::SUCCESS;
  }

} // namespace G4UA
