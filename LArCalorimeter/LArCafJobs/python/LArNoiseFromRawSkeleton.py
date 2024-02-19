# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCafJobs.LArNoiseConfig import LArNoiseFromRawCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags    
    flags=initConfigFlags()
    
    from LArCafJobs.LArNoiseFlags import addNoiseFlags
    addNoiseFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    flags.Input.Files=runArgs.inputBSFile

    if hasattr(runArgs,"outputHIST_LARNOISEFile"):
       flags.LArNoise.outHistLAr=runArgs.outputHIST_LARNOISEFile
       flags.Output.HISTFileName =runArgs.outputHIST_LARNOISEFile

    if hasattr(runArgs,"outputNTUP_LARNOISEFile"):
       flags.LArNoise.outNtupLAr=runArgs.outputNTUP_LARNOISEFile

    if hasattr(runArgs,"outputNTUP_HECNOISEFile"):
        flags.LArNoise.HECNoiseNtup=runArgs.outputNTUP_HECNOISEFile

    if not hasattr(runArgs,"conditionsTag") or runArgs.conditionsTag=="CURRENT":
        print("Resolving 'CURRENT' express conditions tag ...")
        sys.path.append('/afs/cern.ch/user/a/atlcond/utils22/')
        from CondUtilsLib.AtlCoolBKLib import resolveAlias
        resolver=resolveAlias()
        currentGlobalES=resolver.getCurrentES().replace("*","ST")
        print("Found ",currentGlobalES)
        flags.IOVDb.GlobalTag=currentGlobalES
    else:
        flags.IOVDb.GlobalTag=runArgs.conditionsTag

    if hasattr(runArgs,"skipEvents"):
        flags.Exec.SkipEvents=runArgs.skipEvents

    if hasattr(runArgs,"maxEvents"):
        flags.Exec.MaxEvents=runArgs.maxEvents

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.Trigger.triggerConfig = 'DB'    
    flags.Trigger.L1.doCTP = True
    flags.Trigger.doID=False
    flags.Trigger.doMuon=False
    flags.Trigger.doLVL1=False
    flags.Trigger.doHLT=False

    flags.Calo.Cell.doDeadCellCorr=True

    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    cfg.merge(LArNoiseFromRawCfg(flags))

    #OFL LUMI tag not connected to ES tak, doing it here:
    cfg.getService("IOVDbSvc").overrideTags+=['<prefix>/TRIGGER/OFLLUMI/OflPrefLumi</prefix><tag>OflPrefLumi-RUN2-UPD4-12</tag>']
    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
