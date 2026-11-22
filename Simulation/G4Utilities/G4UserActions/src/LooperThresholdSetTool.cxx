/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "LooperThresholdSetTool.h"

namespace G4UA
{

//---------------------------------------------------------------------------
// Initialize - temporarily here for debugging
//---------------------------------------------------------------------------
StatusCode LooperThresholdSetTool::initialize()
{
        ATH_MSG_DEBUG("initialize");
        return StatusCode::SUCCESS;
}

StatusCode LooperThresholdSetTool::finalize()
{
        ATH_MSG_DEBUG("finalize");
        return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
// Create the action on request
//---------------------------------------------------------------------------
std::unique_ptr<LooperThresholdSet>
LooperThresholdSetTool::makeAndFillAction(G4AtlasUserActions& actionList)
{
        ATH_MSG_DEBUG("Making a LooperThresholdSet action");
        LooperThresholdSet::Config config;
        config.WarningEnergy = m_warningEnergy;
        config.ImportantEnergy = m_importantEnergy;
        config.NumberOfTrials = m_numberOfTrials;
        auto action =  std::make_unique<LooperThresholdSet>(config);
        actionList.runActions.push_back( action.get() );
        return action;
}

}
