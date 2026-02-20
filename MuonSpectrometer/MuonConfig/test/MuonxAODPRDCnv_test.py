#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from MuonConfig.MuonConfigUtils import SetupMuonStandaloneOutput, SetupMuonStandaloneCA
from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFindingCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultTestFiles

flags = initConfigFlags() 
flags.Scheduler.ShowDataDeps = True
flags.Scheduler.CheckDependencies = True
flags.Scheduler.ShowDataFlow = True
flags.Scheduler.ShowControlFlow = True
flags.Concurrency.NumThreads  = 1
flags.Concurrency.NumConcurrentEvents = 1
flags.Exec.FPE= 500

flags.Muon.writexAODPRD = True # This is the flag that tells the convertors to produce xAOD PRDs

flags.Input.Files = defaultTestFiles.RDO_RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
flags.Output.ESDFileName='newESD.pool.root'

# If we're running in a debug build, limit to 10 events to reduce the
# time required.
import os
if os.environ.get('Athena_PLATFORM', '').find ('dbg') >= 0:
    flags.Exec.MaxEvents = 10

setupDetectorFlags(flags)
flags.lock()
flags.dump()

cfg = SetupMuonStandaloneCA(flags)

# Run the actual test.
acc = MuonSegmentFindingCfg(flags)
cfg.merge(acc)

itemsToRecord = ["xAOD::MdtDriftCircleContainer#*", "xAOD::MdtDriftCircleAuxContainer#*" ]
itemsToRecord += ["xAOD::sTgcStripContainer#*", "xAOD::sTgcStripAuxContainer#*" ]
itemsToRecord += ["xAOD::MMClusterContainer#*", "xAOD::MMClusterAuxContainer#*" ]
itemsToRecord += ["xAOD::TgcStripContainer#*", "xAOD::TgcStripAuxContainer#*" ]
itemsToRecord += ["xAOD::RpcStripContainer#*", "xAOD::RpcStripAuxContainer#*" ]
SetupMuonStandaloneOutput(cfg, flags, itemsToRecord)


from MuonConfig.MuonConfigUtils import executeTest
executeTest(cfg)

