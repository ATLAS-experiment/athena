# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def RootWriteCfg(flags):
   """Creates a ComponentAccumulator instance containing the
   athena services required for ROOT file writing.
   """

   cfg = ComponentAccumulator()

   cfg.addService( CompFactory.Athena.RootSvc("AthenaRootSvc") )
   cfg.addService( CompFactory.Athena.RootCnvSvc("AthenaRootCnvSvc") )

   return cfg


def NtupleOutputStreamCfg(flags, streamName, fileName, ItemList=None, tupleName="physics", forceRead=False):
   """Configure writing an Ntuple output stream."""

   cfg = RootWriteCfg(flags)

   writingTool = CompFactory.Athena.RootOutputStreamTool(
      f"{streamName}Tool",
      TreeName = tupleName,
      OutputFile = fileName )

   outputStream = CompFactory.Athena.RootNtupleOutputStream(
      streamName,
      WritingTool = writingTool,
      OutputFile = fileName,
      ItemList = ["RunNumber", "EventNumber"] + ([] if ItemList is None else ItemList),
      ForceRead = forceRead )

   cfg.addEventAlgo(outputStream, domain='IO')
   return cfg
