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
if 'doMonitoring' not in configMy.jobConfig:                  configMy.jobConfig['doMonitoring'] = True
if 'VertexNtuple' not in configMy.jobConfig:                  configMy.jobConfig['VertexNtuple'] = True
if 'WriteAllVertices' not in configMy.jobConfig:              configMy.jobConfig['WriteAllVertices'] = False
if 'VertexTreeName' not in configMy.jobConfig:                configMy.jobConfig['VertexTreeName'] = 'Vertices'

#Event selection options
if 'UseBCID' not in configMy.jobConfig:                       configMy.jobConfig['UseBCID'] = []
if 'UseFilledBCIDsOnly' not in configMy.jobConfig:            configMy.jobConfig['UseFilledBCIDsOnly'] = True

#Vertex Selection Options
if 'MinTracksPerVtx' not in configMy.jobConfig:            configMy.jobConfig['MinTracksPerVtx'] = 5
if 'MaxTracksPerVtx' not in configMy.jobConfig:            configMy.jobConfig['MaxTracksPerVtx'] = 1000000
if 'MinVtxNum' not in configMy.jobConfig:                  configMy.jobConfig['MinVtxNum'] = 100
if 'MaxVtxChi2' not in configMy.jobConfig:                 configMy.jobConfig['MaxVtxChi2'] = 10
if 'MaxTransverseErr' not in configMy.jobConfig:           configMy.jobConfig['MaxTransverseErr'] = 1000000
if 'VertexTypes' not in configMy.jobConfig:                configMy.jobConfig['VertexTypes'] = ['PriVtx']
if 'MinVtxProb' not in configMy.jobConfig:                 configMy.jobConfig['MinVtxProb'] = .001
if 'VertexContainer' not in configMy.jobConfig:            configMy.jobConfig['VertexContainer'] = 'PrimaryVertices'
if 'MaxAbsCorrelXY' not  in configMy.jobConfig:            configMy.jobConfig['MaxAbsCorrelXY'] = 0.8

#Options for sorting vertices into fits
if 'LumiRange' not in configMy.jobConfig:                  configMy.jobConfig['LumiRange'] = 0
if 'RunRange' not in configMy.jobConfig:                   configMy.jobConfig['RunRange'] = 0

#if 'GroupFitsByBCID' not in configMy.jobConfig:               configMy.jobConfig['GroupFitsByBCID'] = False
#if 'GroupFitsByPileup' not in configMy.jobConfig:             configMy.jobConfig['GroupFitsByPileup'] = False
if 'GroupFitsBy' not in configMy.jobConfig:                configMy.jobConfig['GroupFitsBy'] = 'none'
if 'EventRange' not in configMy.jobConfig:                 configMy.jobConfig['EventRange'] = 0

#Fit Options
if 'InitialKFactor' not in configMy.jobConfig:             configMy.jobConfig['InitialKFactor'] = 1.0
if 'ConstantKFactor' not in configMy.jobConfig:            configMy.jobConfig['ConstantKFactor'] = False

#Fit Options for official fit only
if 'MaxSigmaTr' not in configMy.jobConfig:                    configMy.jobConfig['MaxSigmaTr'] = 20.
if 'MaxSigmaVtx' not in configMy.jobConfig:                   configMy.jobConfig['MaxSigmaVtx'] = 2.0
if 'TruncatedRMS' not in configMy.jobConfig:                  configMy.jobConfig['TruncatedRMS'] = True
if 'SetInitialRMS' not in configMy.jobConfig:                 configMy.jobConfig['SetInitialRMS'] = False
if 'OutlierChi2Tr' not in configMy.jobConfig:                 configMy.jobConfig['OutlierChi2Tr'] = 20.
if 'BeamSpotToolList' not in configMy.jobConfig:              configMy.jobConfig['BeamSpotToolList'] = ['InDetBeamSpotRooFit','InDetBeamSpotVertex']
if 'FixWidth' not in configMy.jobConfig:                      configMy.jobConfig['FixWidth'] =  False

#Fit Options for RooFit only
if 'RooFitMaxTransverseErr' not in configMy.jobConfig:        configMy.jobConfig['RooFitMaxTransverseErr'] = 0.05

#Job options for Monitoring algorithm
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

from InDetBeamSpotFinder.InDetBeamSpotFinderConfig import InDetBeamSpotFinderCfg
acc.merge(InDetBeamSpotFinderCfg(flags,configMy.jobConfig))

from AthenaConfiguration.ComponentFactory import CompFactory
acc.addService(CompFactory.THistSvc(
    Output = ["INDETBEAMSPOTFINDER DATAFILE='%s' OPT='RECREATE'" % configMy.jobConfig['histfile']]))

if configMy.jobConfig['doMonitoring']:
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
