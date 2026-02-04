#!/env/python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_WriteMeta.py
## @brief Example job options file to illustrate how to write metadata to Pool.
###############################################################
#
# This Job option:
# ----------------
# 1. Writes a SimplePoolFile5.root file with ExampleHit, using WriteData algorithm
# ------------------------------------------------------------

import os
os.system ('rm -f Catalog2.xml')

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

outputFileName = "ROOTTREE:SimplePoolFile5.root"
outputStreamName = "ExampleMeta"
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Input.Files = []
flags.addFlag(f"Output.{outputStreamName}FileName", outputFileName)
flags.Exec.MaxEvents = 10
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = ["PoolSvc", "AthenaPoolCnvSvc","AthenaPoolAddressProviderSvc", "MetaDataSvc"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )    

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg( flags, disableBeamSpot=True ) )

from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleWriteCfg( flags, outputStreamName,
                                      writeCatalog = "file:Catalog2.xml",
                                      disableEventTag = noTag ) )

# Creata and attach the algorithms
acc.addEventAlgo( CompFactory.AthPoolEx.WriteData("WriteData", OutputLevel = DEBUG) )
acc.addEventAlgo( CompFactory.AthPoolEx.WriteCond("WriteCond",
                                                  DetStore = acc.getService("MetaDataStore"),
                                                  ConditionName = "PedestalWriteData",
                                                  OutputLevel = DEBUG) )

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge( OutputStreamCfg( flags, outputStreamName, disableEventTag = noTag,
                            ItemList = ["EventInfo#*", "ExampleHitContainer#MyHits"],
                            MetadataItemList = ["ExampleHitContainer#PedestalWriteData"],
                            ) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






