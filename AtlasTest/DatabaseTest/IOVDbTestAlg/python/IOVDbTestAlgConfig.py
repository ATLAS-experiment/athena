#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG


def IOVDbTestAlgFlags():
   """Create default set of flags for tests"""

   flags = initConfigFlags()
   flags.Common.MsgSuppression = False
   flags.Exec.OutputLevel = DEBUG
   flags.Input.Files = []
   flags.Input.isMC = True
   flags.IOVDb.DBConnection = "sqlite://;schema=mytest.db;dbname=TESTCOOL"
   flags.IOVDb.DatabaseInstance = ""
   flags.IOVDb.GlobalTag = ""

   return flags


def IOVDbTestAlgWriteCfg(flags, registerIOV = False):
   # Basic services
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   acc = MainServicesCfg(flags)

   from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
   acc.merge( IOVDbSvcCfg(flags) )

   if registerIOV:
      acc.addService( CompFactory.IOVRegistrationSvc(OutputLevel = DEBUG) )

   # Setup MC EventSelector and POOL
   from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
   acc.merge( McEventSelectorCfg(flags,
                                 RunNumber         = 1,
                                 EventsPerRun      = 5,
                                 FirstEvent        = 1,
                                 EventsPerLB       = 1,
                                 FirstLB           = 1,
                                 InitialTimeStamp  = 0,
                                 TimeStampInterval = 5) )

   from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
   acc.merge( AthenaPoolCnvSvcCfg(flags,
                                  PoolContainerPrefix = "CollectionTree",
                                  StorageTechnology = "ROOTTREE",
                                  TopLevelContainerName = "<type>",
                                  SubLevelBranchName = "") )

   # Testing algorithm
   acc.addEventAlgo( CompFactory.IOVDbTestAlg(
      "IOVDbTestAlg",
      OutputLevel   = DEBUG,
      StreamName    = "CondStream2",
      RegTime       = 0,     # Set time to register - used for IOVDbTestAmdbCorrection
      WriteCondObjs = True,
      RegisterIOV   = registerIOV,
      ReadWriteCool = True,
      TagID         = "COOL-TEST-001",
      PrintLB       = True) )

   acc.addPublicTool( CompFactory.AthenaOutputStreamTool("CondStream2",
                                                         OutputFile = "SimplePoolFile.root") )
   return acc


def IOVDbTestAlgReadCfg(flags, overrideTag=True):
   # Basic services
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   acc = MainServicesCfg(flags)

   from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
   acc.merge( IOVDbSvcCfg(flags) )

   # Setup input services
   if len(flags.Input.Files) > 0:
      from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
      acc.merge( PoolReadCfg(flags) )
   else:
      from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
      acc.merge( McEventSelectorCfg(flags,
                                    RunNumber         = 1,
                                    EventsPerRun      = 5,
                                    FirstEvent        = 1,
                                    EventsPerLB       = 1,
                                    FirstLB           = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 5) )
   # Testing algorithm
   acc.addEventAlgo( CompFactory.IOVDbTestAlg(
      "IOVDbTestAlg",
      OutputLevel   = DEBUG,
      WriteCondObjs = False,
      RegisterIOV   = False,
      ReadWriteCool = True,
      PrintLB       = True) )

   t = "COOL-TEST-001"
   acc.merge( addFolders(flags, "/IOVDbTest/IOVDbTestMDTEleMap",
                         tag = f"MDTEleMap_{t}" if overrideTag else None) )
   acc.merge( addFolders(flags, "/IOVDbTest/IOVDbTestAMDBCorrection",
                         tag = f"AmdbCorrection_{t}" if overrideTag else None) )
   acc.merge( addFolders(flags, "/IOVDbTest/IOVDbTestAttrList",
                         tag = f"AttrList_{t}" if overrideTag else None) )
   acc.merge( addFolders(flags, "/IOVDbTest/IOVDbTestAttrListColl",
                         tag = f"AttrListColl_{t}" if overrideTag else None) )
   acc.merge( addFolders(flags, "/IOVDbTest/IOVDbTestMDTEleMapColl",
                         tag = f"MDTEleMapColl_{t}" if overrideTag else None) )

   return acc
