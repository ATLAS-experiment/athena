# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EventDisplaysOnline.EventDisplaysOnlineHelpers import GetRunType, GetBFields, WaitForPartition
from AthenaConfiguration.Enums import BeamType

isCosmicData = True
isHIMode = False #TODO
isBeamSplashMode = False
isOfflineTest = False
testWithoutPartition = False

# An explicit list for nominal data taking to exclude some high rate streams
# Empty list to read all
streamsWanted = ['express','ZeroBias','CosmicCalo','IDCosmic','CosmicMuons','Background','Standby','L1Calo','Main']
if isBeamSplashMode:
    streamsWanted = ['MinBias']#HltError
# If testing at p1, write out to /tmp/ to see output
outputDirectory="/atlas/EventDisplayEvents/"
#outputDirectory="/tmp/myexley"

if isOfflineTest:
    outputDirectory="/afs/cern.ch/user/m/myexley/WorkSpace/hackTest/run/output/"

sendToPublicStream = False # Gets set later, overwrite here to True to test it

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
partitionName = 'ATLAS' # 'ATLAS', 'GMTestPartition' or 'GMTestPartitionT9'

if isHIMode:
    maxEvents=200 # Number of events to keep per stream in /atlas/EventDisplays/stream
    projectTags=['data24_hi']
    projectName='data24_hi'
    publicStreams=['MinBias']
if isCosmicData:
    maxEvents=200
    projectTags=['data24_cos']
    projectName='data24_cos'
    publicStreams=['Main']#TODO
if isBeamSplashMode:
    maxEvents=-1
    projectTags=['data24_13p6TeV']
    projectName='data24_13p6TeV'
    publicStreams=['']
else:
    maxEvents=100
    projectTags=['data24_13p6TeV']
    projectName='data24_13p6TeV'
    publicStreams=['Main']


# Pause this thread until the partition is up
if not testWithoutPartition or not isOfflineTest:
    WaitForPartition(partitionName)

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

# Conditions tag
flags.IOVDb.DatabaseInstance = "CONDBR2"
if isOfflineTest:
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-02'
else:
    flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01' # Online conditions tag
flags.GeoModel.Layout="atlas"
flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00' # Geometry tag

if isHIMode:
    flags.Beam.BunchSpacing = 100 # ns
else:
    flags.Beam.BunchSpacing = 25 # ns
flags.Trigger.triggerConfig='DB'

# Test wth a small amount of events and write out to e.g. a tmp dir
if testWithoutPartition or partitionName != 'ATLAS' or isOfflineTest:
    flags.Exec.MaxEvents = 5
    flags.Output.ESDFileName = outputDirectory + "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])
else:
    flags.Exec.MaxEvents = -1
    flags.Output.ESDFileName = "ESD-%s-%s.pool.root" % (jobId[3], jobId[4])

flags.Output.doWriteESD = True
#flags.Output.doJiveXML = True

if testWithoutPartition:
    flags.Input.Files = ['/detwork/dqm/EventDisplays_test_data/data23_13p6TeV.00454188.physics_Main.daq.RAW._lb0633._SFO-12._0002.data']
if isOfflineTest:
    flags.Input.Files = ['/eos/home-m/myexley/sharedWithATLASauthors/data23_13p6TeV.00454188.physics_Main.daq.RAW._lb0633._SFO-12._0002.data']
else:
    flags.Input.Files = [] # Files are read from the ATLAS (or GM test) partition

flags.Reco.EnableTrigger = False # TODO test True
flags.LAr.doHVCorr = False # ATLASRECTS-6823
flags.Detector.GeometryForward = False
flags.Detector.EnableFwdRegion = False

from AthenaCommon.Constants import INFO
flags.Exec.OutputLevel = INFO
flags.Concurrency.NumThreads = 0

if isOfflineTest:
    flags.Common.isOnline = False
else:
    flags.Common.isOnline = True

if partitionName == 'ATLAS' and not testWithoutPartition and not isOfflineTest:
    # Read run number from the partition
    # For beam plashes when LAr running in 32 samples mode, the current run number to LAr config is needed
    # IS stands for Information Service
    from ispy import ISObject, IPCPartition, ISInfoAny, ISInfoDictionary
    part = IPCPartition(partitionName)
    RunParams = ISObject(part, 'RunParams.RunParams', 'RunParams')
    RunParams.checkout()
    run_number = RunParams.getAttributeValue('run_number')
    flags.Input.OverrideRunNumber = True
    flags.Input.RunNumbers = [run_number]

    # Is the data allowed to be seen by the general public on atlas live
    ready4physics = ISInfoAny()
    ISInfoDictionary(part).getValue('RunParams.Ready4Physics', ready4physics)
    print("Ready for physics: %s " % ready4physics.get())
    physicsReady = ISObject(part, 'RunParams.Ready4Physics','Ready4PhysicsInfo')
    physicsReady.checkout()
    print("Ready for physics: %r" % (physicsReady.ready4physics))
    if physicsReady.ready4physics and RunParams.T0_project_tag in projectTags:
        sendToPublicStream = True

    # Get the B field
    (solenoidOn,toroidOn)=GetBFields()
    flags.BField.override = True
    flags.BField.solenoidOn = solenoidOn
    flags.BField.barrelToroidOn = toroidOn
    flags.BField.endcapToroidOn = toroidOn

# GM test partition needs to be given the below info
if (partitionName == 'GMTestPartition' or partitionName == 'GMTestPartitionT9'):
    flags.Input.OverrideRunNumber = True
    flags.Input.RunNumbers = [447705]#keep this number the same as (or close to) the run number of the file you are testing on
    flags.Input.LumiBlockNumbers = [1]
    flags.Input.ProjectName = projectName

if not testWithoutPartition:
    if isCosmicData:
        flags.Beam.Type = BeamType.Cosmics
    else:
        flags.Beam.Type = BeamType.Collisions
flags.lock()
flags.dump()
##----------------------------------------------------------------------##

from RecJobTransforms.RecoSteering import RecoSteering
acc = RecoSteering(flags)

from IOVDbSvc.IOVDbSvcConfig import addOverride
if not isOfflineTest:
    acc.merge(addOverride(flags, "/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2", db=""))

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
    bytestreamInput.KeyCount = 3
    bytestreamInput.Timeout = 600000
    bytestreamInput.UpdatePeriod = 200
    bytestreamInput.BufferSize = 10 # three times of keycount for beam splashes
    bytestreamInput.ISServer = '' # Disable histogramming
    bytestreamInput.StreamNames = streamsWanted
    #bytestreamInput.StreamType = "physics" #comment out for all streams, e.g. if you also want claibration streams
    bytestreamInput.StreamLogic = "Or"
    if isBeamSplashMode:
        bytestreamInput.KeyCount = 62 # equal or greater than the number of DCMs for beam splashes
        bytestreamInput.BufferSize = 186 # three times of keycount for beam splashes
        bytestreamInput.Timeout = 144000000 #(40 hrs) for beam splashes
        bytestreamInput.StreamType = "physics"
    if partitionName != 'ATLAS':
        bytestreamInput.KeyValue = [ 'Test_emon_push' ]
        bytestreamInput.KeyCount = 1

onlineEventDisplaysSvc = CompFactory.OnlineEventDisplaysSvc(
    name = "OnlineEventDisplaysSvc",
    MaxEvents = maxEvents,                   # Number of events to keep per stream
    OutputDirectory = outputDirectory,       # Base directory for streams
    SendToPublicStream = sendToPublicStream, # Allowed to be made public
    PublicStreams = publicStreams,           # These streams go into public stream when Ready4Physics
    StreamsWanted = streamsWanted,
    BeamSplash = isBeamSplashMode,
)
acc.addService(onlineEventDisplaysSvc, create=True)

def StreamToFileToolCfg(flags, name='StreamToFileTool',**kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))
    kwargs.setdefault("IsOnline", True)
    the_tool = CompFactory.JiveXML.StreamToFileTool(**kwargs)
    result.setPrivateTools(the_tool)
    return result
streamToFileTool = acc.popToolsAndMerge(StreamToFileToolCfg(flags))

streamToServerTool = None
if not isOfflineTest:
    def StreamToServerToolCfg(flags, name="StreamToServerTool",**kwargs):
        result = ComponentAccumulator()
        kwargs.setdefault("OnlineEventDisplaysSvc", acc.getService("OnlineEventDisplaysSvc"))
        serverService = CompFactory.JiveXML.ExternalONCRPCServerSvc(name="ExternalONCRPCServerSvc", Hostname = "pc-tdq-mon-29")
        result.addService(serverService)
        kwargs.setdefault("ServerService",serverService)
        kwargs.setdefault("StreamName",".Unknown")
        the_tool = CompFactory.JiveXML.StreamToServerTool(name,**kwargs)
        result.setPrivateTools(the_tool)
        return result
    streamToServerTool = acc.popToolsAndMerge(StreamToServerToolCfg(flags))

from JiveXML.JiveXMLConfig import AlgoJiveXMLCfg
acc.merge(AlgoJiveXMLCfg(flags,StreamToFileTool=streamToFileTool,StreamToServerTool=streamToServerTool,OnlineMode=True))

# This creates an ESD file per event which is renamed and moved to the desired output
# dir in the VP1 Event Prod alg
from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
acc.merge(OutputStreamSequencerSvcCfg(flags,incidentName="EndEvent"))

StreamESD = acc.getEventAlgo("OutputStreamESD")
#vp1Alg = CompFactory.VP1EventProd(name="VP1EventProd",
#                                  InputPoolFile = StreamESD.OutputFile,
#                                  IsOnline = True,
#                                  OnlineEventDisplaysSvc = onlineEventDisplaysSvc)
#acc.addEventAlgo(vp1Alg, primary=True)

acc.getService("PoolSvc").WriteCatalog = "xmlcatalog_file:PoolFileCatalog_%s_%s.xml" % (jobId[3], jobId[4])

##----------------------------------------------------------------------##
## Need line below to fix error that occurs when trying to              ##
## get CaloRec::ToolConstants/H1WeightsCone4Topo DataObject             ##
##----------------------------------------------------------------------##
acc.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/det/dqm/GlobalMonitoring/PoolFileCatalog_M7/PoolFileCatalog.xml"]

sc = acc.run()
import sys
sys.exit(0 if sc.isSuccess() else 1)
