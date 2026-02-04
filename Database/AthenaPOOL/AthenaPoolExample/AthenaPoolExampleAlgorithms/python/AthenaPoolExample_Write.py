#!/env/python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_Write.py
## @brief Example job options file to illustrate how to write event data to Pool.

###############################################################
#
# This Job option:
# ----------------
# 1. Writes a SimplePoolFile1.root file with ExampleHit DataObjects and in-file MetaData
# 2. Writes another SimplePoolFile2.root file using the AthenaPool support for multiple OutputStreams.
# 3. Writes an additional event-less file EmptyPoolFile.root.
# ------------------------------------------------------------
# Expected output file (20 events):
# -rw-r--r--  1 gemmeren zp 16345 Aug  5 16:56 EmptyPoolFile.root
# -rw-r--r--  1 gemmeren zp 25161 Aug  5 16:56 SimplePoolFile1.root
# -rw-r--r--  1 gemmeren zp 20654 Aug  5 16:56 SimplePoolFile2.root
# ------------------------------------------------------------
# File:SimplePoolFile1.root
# Size:       24.564 kb
# Nbr Events: 20
# 
# ================================================================================
#      Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
# ================================================================================
#       11.985 kb        1.420 kb        0.071 kb        0.000       20  (T) DataHeader
# --------------------------------------------------------------------------------
#        3.846 kb        0.313 kb        0.016 kb        0.288       20  (B) EventInfo_p3_McEventInfo
#        3.001 kb        0.427 kb        0.021 kb        0.535        1  (T) MetaDataHdrDataHeaderForm
#       10.312 kb        0.743 kb        0.037 kb        0.156       20  (T) POOLContainer_DataHeaderForm
#       10.578 kb        0.902 kb        0.045 kb        0.379        1  (T) MetaDataHdrDataHeader
#       18.451 kb        1.198 kb        0.060 kb        0.383        1  (B) EventStreamInfo_p2_Stream1
#       18.785 kb        2.686 kb        0.134 kb        0.131       20  (B) ExampleHitContainer_p1_MyHits
# ================================================================================
#       76.958 kb        7.689 kb        0.384 kb        0.000       20  TOTAL (POOL containers)
# ================================================================================
#
# File:SimplePoolFile2.root
# Size:       20.269 kb
# Nbr Events: 20
# 
# ================================================================================
#      Mem Size       Disk Size        Size/Evt      MissZip/Mem  items  (X) Container Name (X=Tree|Branch)
# ================================================================================
#       11.634 kb        1.396 kb        0.070 kb        0.000       20  (T) DataHeader
# --------------------------------------------------------------------------------
#        3.846 kb        0.313 kb        0.016 kb        0.288       20  (B) EventInfo_p3_McEventInfo
#        3.001 kb        0.428 kb        0.021 kb        0.535        1  (T) MetaDataHdrDataHeaderForm
#        8.358 kb        0.646 kb        0.032 kb        0.192       20  (T) POOLContainer_DataHeaderForm
#       10.578 kb        0.903 kb        0.045 kb        0.379        1  (T) MetaDataHdrDataHeader
#       18.440 kb        1.188 kb        0.059 kb        0.383        1  (B) EventStreamInfo_p2_Stream2
# ================================================================================
#       55.857 kb        4.873 kb        0.244 kb        0.000       20  TOTAL (POOL containers)
# ================================================================================
#
#==============================================================


import os
os.system ('rm -f *.root Catalog1.xml')

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName

stream1name = "Stream1"
file1Name = "ROOTTREE:SimplePoolFile1.root"
stream2name = "Stream2"
file2Name = "ROOTTREE:SimplePoolFile2.root"
stream3name = "Stream3"
file3Name =  "ROOTTREE:EmptyPoolFile.root"
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Exec.MaxEvents = 20
flags.Input.Files = []
flags.addFlag(f"Output.{stream1name}FileName", file1Name)
flags.addFlag(f"Output.{stream2name}FileName", file2Name)
flags.addFlag(f"Output.{stream3name}FileName", file3Name)
flags.Exec.DebugMessageComponents = [ outputStreamName(stream1name) , "PoolSvc", "AthenaPoolCnvSvc", "WriteData" ]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg( flags, disableBeamSpot=True ) )

# Load CutFlowSvc
from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
acc.merge( CutFlowSvcCfg( flags ) )    #addMetaDataToAllOutputFiles=True ) )
# Ensure proper metadata propagation
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
acc.merge( IOVDbSvcCfg( flags ) )

# Pool writing
acc.addEventAlgo( CompFactory.AthPoolEx.WriteData("WriteData", OutputLevel = DEBUG) )

# ----------------  Output Stream 1 configuration
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleWriteCfg( flags, stream1name, writeCatalog = "file:Catalog1.xml",
                                      disableEventTag = noTag ) )

stream1ca = OutputStreamCfg( flags, stream1name, disableEventTag = noTag,
                             ItemList = [
                                 'EventInfo#*', 'EventStreamInfo#*',
                                 'ExampleHitContainer#MyHits', 'ExampleHitContainer#PetersHits' ] )
acc.merge( stream1ca )

# ----------------  Output Stream 2 configuration
acc.merge( AthenaPoolExampleWriteCfg( flags, stream2name, disableEventTag = noTag ) )
stream2ca = OutputStreamCfg(flags, stream2name, disableEventTag = noTag,
                            ItemList = ['EventInfo#*', 'ExampleHitContainer#MyHits'] )
acc.merge( stream2ca )

# ----------------  Output Stream 3 configuration
filterAlg = CompFactory.AthPoolEx.PassNoneFilter("PassNoneFilter", OutputLevel = DEBUG)
acc.addEventAlgo( filterAlg )
acc.merge( AthenaPoolExampleWriteCfg( flags, stream3name, disableEventTag = noTag ) )
stream3ca = OutputStreamCfg(flags, stream3name, disableEventTag = noTag )
stream3 = stream3ca.getEventAlgo( outputStreamName( stream3name ) )
stream3.RequireAlgs = [ "PassNoneFilter" ]
acc.merge( stream3ca )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






