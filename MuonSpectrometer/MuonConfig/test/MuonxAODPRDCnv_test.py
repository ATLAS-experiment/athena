#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from MuonConfig.MuonConfigUtils import SetupMuonStandaloneOutput, SetupMuonStandaloneCA
from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFindingCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

flags = initConfigFlags()    
args = flags.fillFromArgs()
flags.Muon.writexAODPRD = True # This is the flag that tells the convertors to produce xAOD PRDs
flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1']
flags.Output.ESDFileName='newESD.pool.root'

setupDetectorFlags(flags)
flags.lock()
flags.dump()

cfg = SetupMuonStandaloneCA(args, flags)

# Run the actual test.
acc = MuonSegmentFindingCfg(flags)
cfg.merge(acc)

itemsToRecord = ["xAOD::MdtDriftCircleContainer#*", "xAOD::MdtDriftCircleAuxContainer#*" ]
itemsToRecord += ["xAOD::sTgcStripContainer#*", "xAOD::sTgcStripAuxContainer#*" ]
itemsToRecord += ["xAOD::TgcStripContainer#*", "xAOD::TgcStripAuxContainer#*" ]
itemsToRecord += ["xAOD::RpcStripContainer#*", "xAOD::RpcStripAuxContainer#*" ]
SetupMuonStandaloneOutput(cfg, flags, itemsToRecord)

# cfg.getService("StoreGateSvc").Dump = True
cfg.printConfig()

if not args.config_only:
    sc = cfg.run(20)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")
else:
    cfg.wasMerged()