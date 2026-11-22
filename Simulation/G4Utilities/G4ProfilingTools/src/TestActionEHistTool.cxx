/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TestActionEHistTool.h"

namespace G4UA
{

  std::unique_ptr<TestActionEHist>
  TestActionEHistTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a TestActionEHist");
    TestActionEHist::Config config;
    config.name = m_name;
    config.dCALO = m_dCALO;
    config.dBeam = m_dBeam;
    config.dIDET = m_dIDET;
    config.dMUON = m_dMUON;
    config.maxhists = m_maxhists;
    config.dDetail = m_dDetail;
    auto action = std::make_unique<TestActionEHist>(config);
    actionList.runActions.push_back( action.get() );
    actionList.trackingActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
