# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('SimValid_tf')
    log.info('****************** STARTING VALIDATION *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, 'localgeo'):
        flags.ITk.Geometry.AllLocal = runArgs.localgeo

    if hasattr(runArgs, 'inputHITSFile'):
        flags.Input.Files = runArgs.inputHITSFile
    else:
        raise RuntimeError('No input HITS file defined')

    if hasattr(runArgs, 'outputHIST_SIMFile'):
        flags.Output.HISTFileName  = runArgs.outputHIST_SIMFile
    else:
        log.warning('No output file set')
        flags.Output.HISTFileName = 'output.HIST_SIM.root'

    # Generate detector list and setup detector flags
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from HitAnalysis.HitAnalysisConfig import HGTD_HitAnalysisCfg,  ALFAHitAnalysisCfg, AFPHitAnalysisCfg, LucidHitAnalysisCfg, ZDCHitAnalysisCfg

    from HitAnalysis.PostIncludes import SimHitAnalysis
    cfg.merge(SimHitAnalysis(flags))

    if flags.Detector.EnableHGTD:
        cfg.merge(HGTD_HitAnalysisCfg(flags))
    # Forward
    if flags.Detector.EnableALFA:
        cfg.merge(ALFAHitAnalysisCfg(flags))

    if flags.Detector.EnableAFP:
        cfg.merge(AFPHitAnalysisCfg(flags))

    if flags.Detector.EnableLucid:
        cfg.merge(LucidHitAnalysisCfg(flags))

    if flags.Detector.EnableZDC:
        cfg.merge(ZDCHitAnalysisCfg(flags))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran HITSMerge_tf in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
