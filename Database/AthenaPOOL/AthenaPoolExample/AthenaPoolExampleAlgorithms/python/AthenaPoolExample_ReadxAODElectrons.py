#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_ReadxAODElectrons.py
## @brief Example job options file to illustrate how to read event data from Pool.
#
# This Job option:
# ----------------
# 1. Reads the data from files have been written with AthenaPoolExample_Write.py
#    and AthenaPoolExample_ReadWrite.py
#
#==============================================================

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Input.Files = [ "SimplePoolFile_xAOD.root" ]
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = [ "ReadExampleElectron", "PoolSvc", "AthenaPoolCnvSvc",
                                      "AthenaPoolAddressProviderSvc", "MetaDataSvc", "EventSelector" ]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

# Configure AthenaPool reading
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog1.xml"]) )

# Create and attach the reading algorithm for the ExampleElectron and its decorations
acc.addEventAlgo( CompFactory.AthPoolEx.ReadExampleElectron("ReadExampleElectron", OutputLevel = DEBUG) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
