# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool

addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getMCTruthUserActionTool"              , "ISFMCTruthUserActionTool")

addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getPhysicsValidationUserActionTool"              , "ISFG4PhysicsValidationUserActionTool")

addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getTrackProcessorUserActionTool"              , "ISFG4TrackProcessorUserActionTool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getFullG4TrackProcessorUserActionTool"        , "FullG4TrackProcessorUserActionTool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getPassBackG4TrackProcessorUserActionTool"    , "PassBackG4TrackProcessorUserActionTool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getATLFAST_G4TrackProcessorUserActionTool"       , "ATLFAST_G4TrackProcessorUserActionTool")

addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getGeant4Tool",               "ISF_Geant4Tool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getFullGeant4Tool",           "ISF_FullGeant4Tool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getPassBackGeant4Tool",       "ISF_PassBackGeant4Tool")
addTool("ISF_Geant4Tools.ISF_Geant4ToolsConfigLegacy.getATLFAST_Geant4Tool",           "ISF_ATLFAST_Geant4Tool")
