# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

from AthenaCommon import SystemOfUnits as Units

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True



def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('ESDtoALL')
    log.info('****************** STARTING ESD-based Reconstruction (ESDtoALL) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    from PyUtils.Helpers import ROOTSetup
    ROOTSetup(batch=True)

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # set per-event timeout (in ns)
    flags.Exec.EventTimeOut = 3600*Units.second

    commonRunArgsToFlags(runArgs, flags)

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    ## Inputs
    # ESD 
    if hasattr(runArgs, 'inputESDFile'):
        flags.Input.Files = runArgs.inputESDFile

    ## Outputs
    if hasattr(runArgs, 'outputAODFile'):
        flags.Output.AODFileName = runArgs.outputAODFile
        log.info("---------- Configured AOD output")

    if hasattr(runArgs, 'outputDESDM_EOVERPFile'):
        flagString = 'Output.DESDM_EOVERPFileName'
        flags.addFlag(flagString, runArgs.outputDESDM_EOVERPFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDESDM_EOVERP', True)
        log.info("---------- Configured DESDM_EOVERP output")

    # Reconstruction flags should be parsed after inputs are set
    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Reconstruction

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)
    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Disable event timeout if debugging has been requested.
    if flags.Exec.DebugStage != '':
        flags.Exec.EventTimeOut = 0

    # Lock flags
    flags.lock()

    # Main reconstruction steering
    from RecJobTransforms.RecoSteering import RecoSteering
    cfg = RecoSteering(flags)

    # Performance DPDs 
    cfg.flagPerfmonDomain('PerfDPD')

    # DESDM_EOVERP
    for flag in [key for key in flags._flagdict.keys() if ("Output.DESDM_EOVERPFileName" in key)]:
        from PrimaryDPDMaker.DESDM_EOVERP import DESDM_EOVERPCfg
        cfg.merge(DESDM_EOVERPCfg(flags))
        log.info("---------- Configured DESDM_EOVERP perfDPD")
        
    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, cfg)

    # Write some metadata into TagInfo
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    cfg.merge(
        TagInfoMgrCfg(
            flags,
            tagValuePairs={
                "beam_type": flags.Beam.Type.value,
                "beam_energy": str(int(flags.Beam.Energy)),
                "triggerStreamOfFile": ""
                if flags.Input.isMC
                else flags.Input.TriggerStream,
                "project_name": "IS_SIMULATION"
                if flags.Input.isMC
                else flags.Input.ProjectName,
                f"AtlasRelease_{runArgs.trfSubstepName}": flags.Input.Release or "n/a",
            },
        )
    )
    if not flags.Input.isMC and flags.Input.DataYear > 0:
        cfg.merge(TagInfoMgrCfg(flags, tagValuePairs={
            "data_year": str(flags.Input.DataYear)
        }))

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    # Print PerfMon domain information when running detailed monitoring
    if flags.PerfMon.doFullMonMT:
        cfg.printPerfmonDomains()

    timeConfig = time.time()
    log.info("configured in %d seconds", timeConfig - timeStart)

    log.info("Configured according to flag values listed below")
    flags.dump()

    # Print sum information about AccumulatorCache performance
    from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
    AccumulatorDecorator.printStats() 

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoALL_skeleton in %d seconds (running %d seconds)", timeFinal - timeStart, timeFinal - timeConfig)

    import sys
    sys.exit(not sc.isSuccess())
