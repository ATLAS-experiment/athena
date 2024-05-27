#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_ReadMeta.py
## @brief Example job options file to illustrate how to read metadata from Pool.
###############################################################
#
# This Job options:
# ----------------
# 1. Read from "SimplePoolFile5.root" created by the WriteMeta example
# 2. Use a custom metadata reading tool 
# ------------------------------------------------------------

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Input.Files = ["SimplePoolFile5.root"]
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = ["EventSelector",
                                     "PoolSvc", "AthenaPoolCnvSvc","AthenaPoolAddressProviderSvc", "MetaDataSvc"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )    

# Configure AthenaPool reading
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog2.xml"]) )

acc.getService("MetaDataSvc").MetaDataTools += ["AthPoolEx::ReadMeta"]

# Creata and attach the algorithm
acc.addEventAlgo( CompFactory.AthPoolEx.ReadData("ReadData", OutputLevel = DEBUG) )

# Run
import sys
sc = acc.run( flags.Exec.MaxEvents )
sys.exit( sc.isFailure() )






