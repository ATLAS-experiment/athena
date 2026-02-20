# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
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
    flags.LArShapeDump.outputNtup="CELLS"
    flags.LArShapeDump.doSCReco=runArgs.doReco

    #protection for LArPEB event:
    flags.Trigger.triggerConfig = 'DB'
    flags.Trigger.L1.doCTP=True
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False


    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addService(CompFactory.THistSvc(Output=["CELLS DATAFILE='"+runArgs.outputNTUP_LARCELLSFile+"' OPT='RECREATE'",]))
    if runArgs.isSC:
       from LArCafJobs.LArReadCellsConfig import LArReadSCCfg
       cfg.merge(LArReadSCCfg(flags))
    else:
       from LArCafJobs.LArReadCellsConfig import LArReadCellsCfg
       cfg.merge(LArReadCellsCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
