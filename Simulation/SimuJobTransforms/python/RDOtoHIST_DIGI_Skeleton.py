# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DigiValid_tf')
    log.info( '****************** STARTING VALIDATION *****************' )

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, 'localgeo'):
        flags.ITk.Geometry.AllLocal = runArgs.localgeo

    if hasattr(runArgs, "PileUpPresampling"):
        from AthenaConfiguration.Enums import ProductionStep
        flags.Common.ProductionStep = ProductionStep.PileUpPresampling

    if hasattr(runArgs, 'inputRDOFile'):
        flags.Input.Files = runArgs.inputRDOFile
    else:
        raise RuntimeError('No input RDO file defined')

    if hasattr(runArgs, 'outputHIST_DIGIFile'):
        flags.Output.HISTFileName  = runArgs.outputHIST_DIGIFile
    else:
        log.warning('No output file set')
        flags.Output.HISTFileName = 'output.HIST_DIGI.root'

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True)

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

    from RDOAnalysis.RDOAnalysisConfig import EventInfoRDOAnalysisCfg, BCM_RDOAnalysisCfg, PixelRDOAnalysisCfg, SCT_RDOAnalysisCfg, TRT_RDOAnalysisCfg, ITkPixelRDOAnalysisCfg, ITkStripRDOAnalysisCfg, HGTD_RDOAnalysisCfg, PLR_RDOAnalysisCfg, LArRDOAnalysisCfg, TileRDOAnalysisCfg, CSC_RDOAnalysisCfg, MDT_RDOAnalysisCfg, RPC_RDOAnalysisCfg, TGC_RDOAnalysisCfg

    cfg.merge(EventInfoRDOAnalysisCfg(flags))

    if flags.Detector.EnableBCM:
        cfg.merge(BCM_RDOAnalysisCfg(flags))

    if flags.Detector.EnablePixel:
        cfg.merge(PixelRDOAnalysisCfg(flags))

    if flags.Detector.EnableSCT:
        cfg.merge(SCT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableTRT:
        cfg.merge(TRT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableITkPixel:
        cfg.merge(ITkPixelRDOAnalysisCfg(flags))

    if flags.Detector.EnableITkStrip:
        cfg.merge(ITkStripRDOAnalysisCfg(flags))

    if flags.Detector.EnableHGTD:
        cfg.merge(HGTD_RDOAnalysisCfg(flags))

    if flags.Detector.EnablePLR:
        cfg.merge(PLR_RDOAnalysisCfg(flags))

    if flags.Detector.EnableLAr:
        cfg.merge(LArRDOAnalysisCfg(flags))

    if flags.Detector.EnableTile:
        cfg.merge(TileRDOAnalysisCfg(flags))

    if flags.Detector.EnableCSC:
        cfg.merge(CSC_RDOAnalysisCfg(flags))

    if flags.Detector.EnableMDT:
        cfg.merge(MDT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableRPC:
        cfg.merge(RPC_RDOAnalysisCfg(flags))

    if flags.Detector.EnableTGC:
        cfg.merge(TGC_RDOAnalysisCfg(flags))

    # Silence HepMcParticleLink warnings
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(flags))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran RDOMerge_tf in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
