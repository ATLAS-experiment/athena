# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCafJobs.LArShapeDumperConfig import LArShapeDumperCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags    

    flags=initConfigFlags()
    from LArCafJobs.LArShapeDumperFlags import addShapeDumpFlags
    addShapeDumpFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.LAr.ROD.forceIter=True
    flags.LAr.OFCShapeFolder="4samples3bins17phases"
    flags.Input.Files=runArgs.inputBSFile
    flags.LArShapeDump.outputNtup=runArgs.outputNTUP_SAMPLESMONFile

    #protection for LArPEB event:
    flags.Trigger.triggerConfig = 'DB'
    flags.Trigger.L1.doCTP = True
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False

    if hasattr(runArgs,"outputNTUP_HECNOISEFile"):
        flags.LArShapeDump.HECNoiseNtup=runArgs.outputNTUP_HECNOISEFile
        
    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    cfg.merge(LArShapeDumperCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
