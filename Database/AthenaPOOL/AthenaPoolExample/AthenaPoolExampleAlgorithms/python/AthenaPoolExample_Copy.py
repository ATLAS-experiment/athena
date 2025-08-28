#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_Copy.py
## @brief An example Athena job to illustrate how to copy content of a file.
###############################################################
#
# This Job option:
# ----------------
# 1. Creates a copy of objects from SimplePoolFile1.root file
# ------------------------------------------------------------
# Expected output file (20 events):
# -rw-r--r--  1 gemmeren zp 28071 Aug  5 18:59 SimplePoolReplica1.root
# ------------------------------------------------------------
# File:SimplePoolReplica1.root
# Size:       27.413 kb
# Nbr Events: 20
# 
# ================================================================================
#      Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
# ================================================================================
#       11.985 kb        1.420 kb        0.071 kb        0.000       20  (T) DataHeader
# --------------------------------------------------------------------------------
#        3.846 kb        0.313 kb        0.016 kb        0.288       20  (B) EventInfo_p3_McEventInfo
#        3.001 kb        0.427 kb        0.021 kb        0.535        1  (T) MetaDataHdrDataHeaderForm
#       10.312 kb        0.745 kb        0.037 kb        0.156       20  (T) POOLContainer_DataHeaderForm
#       10.578 kb        0.903 kb        0.045 kb        0.379        1  (T) MetaDataHdrDataHeader
#       18.451 kb        1.198 kb        0.060 kb        0.383        1  (B) EventStreamInfo_p2_Stream1
#       18.785 kb        2.686 kb        0.134 kb        0.131       20  (B) ExampleHitContainer_p1_MyHits
# ================================================================================
#       76.958 kb        7.692 kb        0.385 kb        0.000       20  TOTAL (POOL containers)
# 
# ================================================================================

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName

streamName = "Copy"
outputFileName =  "SimplePoolReplica1.root"
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Input.Files = [ "SimplePoolFile1.root" ]
flags.addFlag(f"Output.{streamName}FileName", outputFileName)
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = [ outputStreamName(streamName), "EventSelector",
                                      "PoolSvc", "AthenaPoolCnvSvc","AthenaPoolAddressProviderSvc", "MetaDataSvc" ]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

# Pool reading and writing
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg, AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog1.xml"]) )
acc.merge( AthenaPoolExampleWriteCfg(flags, streamName,
                                     writeCatalog = "file:Catalog1.xml",
                                     disableEventTag = noTag) )

# Produce xAOD::EventInfo from EventInfo
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg(flags, disableBeamSpot = True) )

streamCA = OutputStreamCfg(flags, streamName, disableEventTag = noTag,
                           ItemList = ["EventInfo#*", "ExampleHitContainer#MyHits"])
stream = streamCA.getEventAlgo( outputStreamName( streamName ) )
stream.ExtendProvenanceRecord = False
acc.merge( streamCA )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






