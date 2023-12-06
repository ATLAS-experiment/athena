# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EventDisplaysOnline.EventDisplaysOnlineHelpers import GetRunType, GetBFields, WaitForPartition
from AthenaCommon.Constants import INFO, DEBUG, ERROR, WARNING

isHIMode = False #TODO
#TODO isBeamSplashMode = False
isOfflineTest = True
testWithoutPartition = True

##----------------------------------------------------------------------##
## When the ATLAS partition is not running you can use two test         ##
## partitions that serve events from a raw data file.                   ##
## To see which files will be ran over on the test partitions, at       ##
## point 1, see the uncommented lines in:                               ##
## /det/dqm/GlobalMonitoring/GMTestPartition_oks/tdaq-10-00-00/         ##
## without_gatherer/GMTestPartitionT9.data.xml                          ##
## and in:                                                              ##
## /det/dqm/GlobalMonitoring/GMTestPartition_oks/tdaq-10-00-00/         ##
## without_gatherer/GMTestPartition.data.xml                            ##
##----------------------------------------------------------------------##
partitionName = 'GMTestPartition' # ''ATLAS', 'GMTestPartitionT9' or 'GMTestPartition' 

# Pause this thread until the partition is up
if not testWithoutPartition or not isOfflineTest:
    WaitForPartition(partitionName)

# outputDirectory="/atlas/EventDisplayEvents/"
outputDirectory="/afs/cern.ch/user/m/myexley/WorkSpace/testCA/run/output/"

if isHIMode:
    maxEvents=200
    projectTags=['data23_hi']
    projectName='data23_hi'
    publicStreams=['physics_MinBias']
else:
    maxEvents=100
    projectTags=['data23_13p6TeV']
    projectName='data23_13p6TeV'
    publicStreams=['physics_Main']

# Setup unique output files (so that multiple Athena jobs on the same machine don't interfere)
import os
jobId = os.environ.get('TDAQ_APPLICATION_NAME', '').split(':')
if not len(jobId) == 5:
    from random import randint
    jobId = ['Athena-EventProcessor', 'Athena-EventDisplays-Segment', 'EventDisplays-Rack', 'tmp', '%d' % randint(0, 999)]

if not isOfflineTest:
    IPC_timeout = int(os.environ['TDAQ_IPC_TIMEOUT'])
    print(" IPC_timeout Envrionment Variable = %d" %IPC_timeout)

##----------------------------------------------------------------------##
# Define the flags
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
if not isOfflineTest:
    from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
    autoConfigOnlineRecoFlags(flags, partitionName)
flags.Concurrency.NumThreads = 1

# Conditions tag
if isOfflineTest:
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-01'
else:
    flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01' # Online conditions tag

flags.GeoModel.Layout="atlas"
flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00' # Geometry tag

if isHIMode:
    flags.Beam.BunchSpacing = 100 # ns    
else:
    flags.Beam.BunchSpacing = 25 # ns
flags.Trigger.triggerConfig='DB'

# Test wth a small amount of events and write out to tmp dir
if testWithoutPartition or partitionName != 'ATLAS' or isOfflineTest:
    flags.Exec.MaxEvents = 3
    flags.Output.ESDFileName = outputDirectory + "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])
else:
    flags.Exec.MaxEvents = -1
    flags.Output.ESDFileName = "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])

flags.Output.doWriteESD = True
#flags.Output.doJiveXML = True

if testWithoutPartition or isOfflineTest:
    #flags.Input.Files = ['/detwork/dqm/EventDisplays_test_data/data23_13p6TeV.00454188.physics_Main.daq.RAW._lb0633._SFO-12._0002.data']
    flags.Input.Files = ['/afs/cern.ch/work/m/myexley/ED-files/nominal/data23_13p6TeV.00454188.physics_Main.daq.RAW._lb0633._SFO-12._0002.data']
else:
    flags.Input.Files = [] # Files are read from the ATLAS (or GM test) partition

flags.Reco.EnableTrigger = False
flags.LAr.doHVCorr = False # ATLASRECTS-6823
flags.Exec.OutputLevel = INFO

if isOfflineTest:
    flags.Common.isOnline = False

if partitionName == 'ATLAS' and not testWithoutPartition and not isOfflineTest:
    # Read run number from the partition
    # For beam plashes when LAr running in 32 samples mode, the current run number to LAr config is needed
    # IS stands for Information Service
    from ispy import ISObject, IPCPartition
    RunParams = ISObject(IPCPartition(partitionName), 'RunParams.RunParams', 'RunParams')
    RunParams.checkout()
    flags.Input.RunNumber = RunParams.run_number
    
    # Get the B field
    (solenoidOn,toroidOn)=GetBFields()
    flags.BField.override = True
    flags.BField.solenoidOn = solenoidOn
    flags.BField.barrelToroidOn = toroidOn
    flags.BField.endcapToroidOn = toroidOn

# GM test partition needs to be given the below info
if (partitionName == 'GMTestPartition' or partitionName == 'GMTestPartitionT9') or testWithoutPartition:# or isOfflineTest:
    flags.Input.RunNumber = [412343]
    flags.Input.LumiBlockNumber = [1]
    flags.Input.ProjectName = projectName

if not testWithoutPartition:
    RunType = GetRunType()
    flags.Beam.Type = RunType # "singlebeam", "collisions" or "cosmics"

flags.lock()
flags.dump()
##----------------------------------------------------------------------##
from RecJobTransforms.RecoSteering import RecoSteering
acc = RecoSteering(flags)

from IOVDbSvc.IOVDbSvcConfig import addOverride
acc.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2"))
acc.merge(addOverride(flags,"/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1"))

if isHIMode:
    maxEvents=200
    projectTags=['data23_hi']
    publicStreams=['physics_MinBias']
else:
    maxEvents=100
    projectTags=['data23_13p6TeV']
    publicStreams=['physics_Main']

if not testWithoutPartition:
    bytestreamConversion = CompFactory.ByteStreamCnvSvc()
    acc.addService(bytestreamConversion, primary=True)

    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(flags))

    bytestreamInput = acc.getService("ByteStreamInputSvc")
    bytestreamInput.Partition = partitionName
    bytestreamInput.GroupName = "EventDisplaysOnline"
    bytestreamInput.PublishName = "EventDisplays"
    bytestreamInput.Key = "dcm"
    bytestreamInput.KeyCount = 3 # equal or greater than the number of DCMs for beam splashes
    bytestreamInput.Timeout = 600000 # 144000000 (40 hrs) for beam splashes
    bytestreamInput.UpdatePeriod = 200
    bytestreamInput.BufferSize = 10 # three times of keycount for beam splashes
    bytestreamInput.ISServer = '' # Disable histogramming
    # Empty stream name list to read all streams
    # 'MinBias' for beam splashes
    # An explicit list for nominal data taking to exclude some high rate streams
    bytestreamInput.StreamNames = ['ZeroBias:CosmicCalo:IDCosmic:CosmicMuons:Background:Standby:L1Calo:Main']
    bytestreamInput.StreamType = "physics"
    bytestreamInput.StreamLogic = "Or"

    if partitionName != 'ATLAS':
        bytestreamInput.KeyValue = [ 'Test_emon_push' ]
        bytestreamInput.KeyCount = 1

def StreamToFileToolCfg(flags, name="StreamToFileTool",**kwargs):
    result = ComponentAccumulator()
    # if testing and you need to see the output change below to e.g. /tmp/username
    OutputDirectory = outputDirectory
    prefixFileName = "%s/.Unknown/JiveXML" % outputDirectory
    #set the StreamName as prefix
    kwargs.setdefault("FileNamePrefix", prefixFileName)
    the_tool = CompFactory.JiveXML.StreamToFileTool(name,**kwargs)
    result.addPublicTool(the_tool)
    return result
acc.merge(StreamToFileToolCfg(flags))

if not isOfflineTest:
    def StreamToServerToolCfg(flags, name="StreamToServerTool",**kwargs):
        result = ComponentAccumulator()
        serverService = CompFactory.JiveXML.ExternalONCRPCServerSvc(name="ExternalONCRPCServerSvc", Hostname = "pc-tdq-mon-29")
        result.addService(serverService)
        kwargs.setdefault("ServerService",serverService)
        kwargs.setdefault("StreamName","Unknown")
        the_tool = CompFactory.JiveXML.StreamToServerTool(name,**kwargs)
        result.addPublicTool(the_tool)
        return result
    acc.merge(StreamToServerToolCfg(flags))

from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
acc.merge(AlgoJiveXMLCfg(flags))

from EventDisplaysOnline.OnlineEventDisplaysSvc import OnlineEventDisplaysSvc
acc.addService(OnlineEventDisplaysSvc(
    name = "OnlineEventDisplaysSvc",
    OutputLevel = DEBUG,               # Verbosity
    MaxEvents = maxEvents,             # Number of events to keep per stream
    OutputDirectory = outputDirectory, # Base directory for streams
    ProjectTags = projectTags,         # Project tags that are allowed to be made public
    Public = publicStreams,            # These streams go into public stream when Ready4Physics

), create=True)

StreamESD = acc.getEventAlgo("OutputStreamESD")
vp1Alg = CompFactory.VP1EventProd(name="VP1EventProd", InputPoolFile = StreamESD.OutputFile)
acc.addEventAlgo(vp1Alg)

acc.getService("PoolSvc").WriteCatalog = "xmlcatalog_file:PoolFileCatalog_%s_%s.xml" % (jobId[3], jobId[4])

##----------------------------------------------------------------------##
## Neeed line below to fix error that occurs when trying to             ##
## get CaloRec::ToolConstants/H1WeightsCone4Topo DataObject             ##
##----------------------------------------------------------------------##
acc.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/det/dqm/GlobalMonitoring/PoolFileCatalog_M7/PoolFileCatalog.xml"]

sc = acc.run()
import sys
sys.exit(0 if sc.isSuccess() else 1)
