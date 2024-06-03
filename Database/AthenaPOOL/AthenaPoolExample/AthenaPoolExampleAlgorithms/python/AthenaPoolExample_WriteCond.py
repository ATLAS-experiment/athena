#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_WriteCond.py
## @brief Example job options file to illustrate how to write conditions data to Pool.
###############################################################
#
# This Job option:
# ----------------
# 1. Reads the data from the SimplePoolFile1.root file
#    had been written with the AthneaPoolExample_WriteJobOptions.py
# 2. Writes another SimplePoolFile4.root ROOT file using WriteCond algorithm
#    which contains conditions for the event sample (eg pedestals).
# ------------------------------------------------------------

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

outputStreamName = "ExampleCond"
outputFileName = "ROOTTREE:SimplePoolFile4.root"
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Input.Files = ["SimplePoolFile1.root"]
flags.addFlag(f"Output.{outputStreamName}FileName", outputFileName)
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = ["PoolSvc", "AthenaPoolCnvSvc","AthenaPoolAddressProviderSvc", "MetaDataSvc"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )    

from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg, AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog1.xml"]) )
acc.merge( AthenaPoolExampleWriteCfg(flags, outputStreamName,
                                     writeCatalog = "file:Catalog1.xml",
                                     disableEventTag = noTag) )

# Creata and attach the algorithms
acc.addEventAlgo( CompFactory.AthPoolEx.ReadData("ReadData", OutputLevel = DEBUG) )
acc.addEventAlgo( CompFactory.AthPoolEx.WriteCond("WriteCond",
                                                  DetStore = acc.getService("DetectorStore"),
                                                  ConditionName = "PedestalWriteData",
                                                  Weight = 0.0, Offset = 0.0,
                                                  OutputLevel = DEBUG) )

from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
acc.merge( OutputConditionsAlgCfg(flags, outputStreamName, outputFile = outputFileName,
                                  ObjectList = ["ExampleHitContainer#PedestalWriteData"],
                                  WriteIOV = False,
                                  OutputLevel = DEBUG
                                  ) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






