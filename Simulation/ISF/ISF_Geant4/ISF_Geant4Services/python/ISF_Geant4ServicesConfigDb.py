# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addService
addService("ISF_Geant4Services.ISF_Geant4ServicesConfigLegacy.getGeant4SimSvc",          "ISF_Geant4SimSvc")
addService("ISF_Geant4Services.ISF_Geant4ServicesConfigLegacy.getFullGeant4SimSvc",      "ISF_FullGeant4SimSvc")
addService("ISF_Geant4Services.ISF_Geant4ServicesConfigLegacy.getPassBackGeant4SimSvc",  "ISF_PassBackGeant4SimSvc")
addService("ISF_Geant4Services.ISF_Geant4ServicesConfigLegacy.getATLFAST_Geant4SimSvc",      "ISF_ATLFAST_Geant4SimSvc")
