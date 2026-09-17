# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
  
#********************************************************************
# MuonsSelectionToolConfig.py 
# Configures muon selection tool which is used to select muons 
# for use in physics analysis
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
#from AthenaConfiguration.Enums import LHCPeriod

### Standard configuration of the MuonSelectionTool used in reconstruction & validation jobs
### The snippet is not meant for analysis jobs as it inherently switches off important cuts ensuring 
### best muon selection quality
def MuonSelectionToolCfg(flags, name="MuonSelectionTool", **kwargs):
   """Configure the muon selection tool"""
   acc = ComponentAccumulator()
   #Extract the run period from the flags and set it for the tool. This is needed to ensure the tool applies the correct geometry cuts for Run 2, Run 3
   kwargs.setdefault("RunPeriod", 3 if flags.GeoModel.Run >= LHCPeriod.Run3 else 2)
   kwargs.setdefault("DisablePtCuts", True)
   kwargs.setdefault("TurnOffMomCorr", True)
   the_tool = CompFactory.CP.MuonSelectionTool(name, **kwargs)   
   acc.setPrivateTools(the_tool)
   return acc   

