/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VolumeDebuggerTool.h"

namespace G4UA
{

  std::unique_ptr<VolumeDebugger>
  VolumeDebuggerTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a VolumeDebugger");
    VolumeDebugger::Config config;
    config.path = m_path;
    config.targetVolume = m_targetVolume;
    config.res = m_res;
    config.tol = m_tol;
    config.verbose = m_verbose;
    config.volumeCheck = m_volumeCheck;
    config.targetMaxCopiesToCheck = m_targetMaxCopiesToCheck;
    config.dumpGDML = m_dumpGDML;
    config.dumpPhysicsRegions = m_dumpPhysicsRegions;
    config.printGeo = m_printGeo;
    auto action = std::make_unique<VolumeDebugger>(config);
    actionList.runActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
