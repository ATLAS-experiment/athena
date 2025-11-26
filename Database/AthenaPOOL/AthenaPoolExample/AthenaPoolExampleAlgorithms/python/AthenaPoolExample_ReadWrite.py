#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_ReadWrite.py
###############################################################
#
# This Job option:
# ----------------
# 1. Reads the data (EventInfo, ExampleHits) from the SimplePoolFile1.root file
#    that has been written with the AthenaPoolExample_Write.py
# 2. Writes SimplePoolFile3.root file with ExampleTracks using ReWriteData algorithm
# ------------------------------------------------------------

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName

streamName = "ReWrite"
outputFileName = "SimplePoolFile3.root"
noTag = True

# Setup flags
flags = initConfigFlags()
flags.Input.Files = ["SimplePoolFile1.root"]
flags.addFlag(f"Output.{streamName}FileName", outputFileName)
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = [outputStreamName(streamName),
                                     "PoolSvc", "AthenaPoolCnvSvc","AthenaPoolAddressProviderSvc", "MetaDataSvc"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

# Pool reading and writing
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg, AthenaPoolExampleWriteCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog1.xml"]) )
acc.merge( AthenaPoolExampleWriteCfg( flags, streamName,
                                      writeCatalog = "file:Catalog1.xml",
                                      disableEventTag = noTag ) )

# Create and attach the algorithms
acc.addEventAlgo( CompFactory.AthPoolEx.ReadData("ReadData", OutputLevel = DEBUG) )
acc.addEventAlgo( CompFactory.AthPoolEx.ReWriteData("ReWriteData", OutputLevel = DEBUG) )

# Produce xAOD::EventInfo from EventInfo
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge( EventInfoCnvAlgCfg(flags, disableBeamSpot = True) )

# ----------------  Output Stream configuration
streamCA = OutputStreamCfg( flags, streamName, disableEventTag = True,
                            ItemList = [ "ExampleTrackContainer#MyTracks" ] )
stream = streamCA.getEventAlgo( outputStreamName( streamName ) )
acc.merge( streamCA )

#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
#--------------------------------------------------------------
stream.WritingTool.OutputLevel = 3
stream.HelperTools[0].OutputLevel = 3

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
