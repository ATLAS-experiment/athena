#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_Concat.py
## @brief Example job options file to illustrate how to concatenate two write jobs

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName

stream1name = "Stream1"
file1Name = "ROOTTREE:SimplePoolFile1.root"
stream2name = "Stream2"
file2Name = "ROOTTREE:SimplePoolFile3.root"
outSequence = 'AthOutSeq'
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Input.Files = []
flags.addFlag(f"Output.{stream1name}FileName", file1Name)
flags.addFlag(f"Output.{stream2name}FileName", file2Name)
flags.Common.MsgSuppression = False
flags.Exec.MaxEvents = 20
flags.Exec.DebugMessageComponents = [outputStreamName(stream1name), outputStreamName(stream2name),
                                     "PoolSvc", "AthenaPoolCnvSvc", "WriteData", "ReWriteData"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg( flags, disableBeamSpot=True ) )

acc.addEventAlgo( CompFactory.AthPoolEx.WriteData("WriteData", OutputLevel = DEBUG) )
acc.addEventAlgo( CompFactory.AthPoolEx.ReWriteData("ReWriteData", OutputLevel = DEBUG) )

# Pool writing
# ----------------  Output Stream 1 configuration
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleWriteCfg(flags, stream1name, writeCatalog = "file:Catalog1.xml",
                                     disableEventTag = noTag) )

acc.merge( OutputStreamCfg(flags, stream1name, disableEventTag = noTag,
                           ItemList = ['EventInfo#*', 'ExampleHitContainer#My*']
                           ) )

acc.addEventAlgo(
    CompFactory.MakeInputDataHeader(
        "MakeInputDH", StreamName = outputStreamName(stream1name), OutputLevel = DEBUG ),
    sequenceName = outSequence )

# ----------------  Output Stream 2 configuration
acc.merge( AthenaPoolExampleWriteCfg(flags, stream2name, disableEventTag = noTag) )
acc.merge( OutputStreamCfg(flags, stream2name, disableEventTag = noTag,
                           ItemList = ['EventInfo#*', 'ExampleTrackContainer#*Trackss']
                           ) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






