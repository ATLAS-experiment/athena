/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LooperKillerTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Initialize - temporarily here for debugging
  //---------------------------------------------------------------------------
  StatusCode LooperKillerTool::initialize()
  {
    ATH_MSG_DEBUG("initialize");
    return StatusCode::SUCCESS;
  }

  StatusCode LooperKillerTool::finalize()
  {
    ATH_MSG_DEBUG("finalize");

    // Accumulate the results across threads
    LooperKiller::Report report;
    m_actions.accumulate(report, &LooperKiller::getReport,
                         &LooperKiller::Report::merge);

    ATH_MSG_INFO( "******* Report from "<< name()<< " *******");
    ATH_MSG_INFO(" Set to kill tracks over " << m_maxSteps.value() << " steps");
    ATH_MSG_INFO(" and give " << m_printSteps.value() << " steps of verbose output");
    ATH_MSG_INFO(" We killed " << report.killed_tracks << " tracks this run.");
    ATH_MSG_INFO(" Was set to " << (m_abortEvent?"":"not ") << "abort events and ");
    ATH_MSG_INFO( (m_setError?"":"not ") << "set an error state." );

    return StatusCode::SUCCESS;
  }


  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<LooperKiller>
  LooperKillerTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a LooperKiller action");
    LooperKiller::Config config;
    config.MaxSteps = m_maxSteps;
    config.PrintSteps = m_printSteps;
    config.VerboseLevel = m_verboseLevel;
    config.AbortEvent = m_abortEvent;
    config.SetError = m_setError;
    config.BSM_Only = m_bsmOnly;
    auto action =  std::make_unique<LooperKiller>(config);
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

}
