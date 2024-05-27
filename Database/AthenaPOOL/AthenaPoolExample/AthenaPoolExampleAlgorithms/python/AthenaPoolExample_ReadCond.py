#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_ReadCond.py
## @brief Example job options file to illustrate how to read conditions data from APR
###############################################################
#
# This Job option:
# ----------------
# 1. Reads the data from the SimplePoolFile1.root file
#    had been written with the AthneaPoolExample_WriteJobOptions.py
# 2. Reads another SimplePoolFile4.root file using ReadCond algorithm
#    which contains conditions for the event sample written by
#    AthneaPoolExample_WriteCond.py
# ------------------------------------------------------------

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Input.Files = ["SimplePoolFile1.root"]
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = ["EventSelector", "CondProxyProvider",
                                     "PoolSvc", "AthenaPoolAddressProviderSvc", "MetaDataSvc"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )    

from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg
acc.merge( AthenaPoolExampleReadCfg( flags, readCatalogs = [ "file:Catalog1.xml" ] ) )

acc.addService( CompFactory.CondProxyProvider( "CondProxyProvider",
                                               InputCollections = ["SimplePoolFile4.root"]
                                               ) )
acc.getService("ProxyProviderSvc").ProviderNames += ["CondProxyProvider"]

# Creata and attach the algorithms
acc.addEventAlgo( CompFactory.AthPoolEx.ReadData("ReadData", OutputLevel = DEBUG) )
acc.addEventAlgo( CompFactory.AthPoolEx.ReadCond("ReadCond", OutputLevel = DEBUG) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






