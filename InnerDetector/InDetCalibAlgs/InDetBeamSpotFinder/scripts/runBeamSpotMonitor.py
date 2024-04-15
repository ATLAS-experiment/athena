#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

import configMy

#Job options
if 'outputlevel' not in configMy.jobConfig:                   configMy.jobConfig['outputlevel'] = 3
if 'maxEvents' not in configMy.jobConfig:                     configMy.jobConfig['maxEvents'] = -1
if 'skipEvents' not in configMy.jobConfig:                    configMy.jobConfig['skipEvents'] = 0
if 'MsgLimit' not in configMy.jobConfig:                      configMy.jobConfig['MsgLimit'] = 20 

if 'outputfileprefix' not in configMy.jobConfig:              configMy.jobConfig['outputfileprefix'] = ''
if 'outputfile' not in configMy.jobConfig:                    configMy.jobConfig['outputfile'] = configMy.jobConfig['outputfileprefix']+'beamspot.db'
if 'histfile' not in configMy.jobConfig:                      configMy.jobConfig['histfile'] = configMy.jobConfig['outputfileprefix']+'nt.root'
if 'monfile' not in configMy.jobConfig:                       configMy.jobConfig['monfile'] = configMy.jobConfig['outputfileprefix']+'beamspotmonitoring.root'
if 'jobpostprocsteps' not in configMy.jobConfig:              configMy.jobConfig['jobpostprocsteps'] = 'MergeNt PlotBeamSpot LinkResults AveBeamSpot DQBeamSpot'

#Job options for Monitoring algorithm
if 'beamspottag' not in configMy.jobConfig:                   configMy.jobConfig['beamspottag'] = ''
if 'MinTracksPerVtx' not in configMy.jobConfig:               configMy.jobConfig['MinTracksPerVtx'] = 5
if 'MinTrackPt' not in configMy.jobConfig:                    configMy.jobConfig['MinTrackPt'] = 500.
if 'useBeamSpot' not in configMy.jobConfig:
    # change to True as soon as I have PrimaryVertexMonitoring in as well
    configMy.jobConfig['useBeamSpot'] = configMy.jobConfig.get('beamspottag','')!='' or configMy.jobConfig.get('beamspotfile','')!=''

#Printout of job configuration
print("Job configuration: ")
for option in configMy.jobConfig:
    print("    ",option,': ',configMy.jobConfig[option])
print("    ")
    
flags.Exec.OutputLevel = configMy.jobConfig['outputlevel']
flags.Exec.SkipEvents = configMy.jobConfig['skipEvents']
flags.Exec.MaxEvents = configMy.jobConfig['maxEvents']

flags.Input.Files = []
for path in configMy.jobConfig['inputfiles']:
    print("path: ",path)
    print("glob: ",glob(path))
    flags.Input.Files += glob(path)

flags.Trigger.triggerConfig = "DB"
flags.DQ.enableLumiAccess = False
flags.Output.HISTFileName = configMy.jobConfig['monfile']
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))
acc.getService("MessageSvc").defaultLimit = configMy.jobConfig['MsgLimit']
acc.getService(acc.getAppProps()['EventLoop']).EventPrintoutInterval = 10000

if configMy.jobConfig['beamspottag']:
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    acc.merge(addOverride(flags, "/Indet/Beampos", configMy.jobConfig['beamspottag']))

from AthenaMonitoring import AthMonitorCfgHelper
helper = AthMonitorCfgHelper(flags, "BeamSpotMonitoring")
from InDetGlobalMonitoringRun3Test.InDetGlobalBeamSpotMonAlgCfg import (
    InDetGlobalBeamSpotMonAlgCfg )
InDetGlobalBeamSpotMonAlgCfg(helper, acc, flags,configMy.jobConfig)
acc.merge(helper.result())

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())




