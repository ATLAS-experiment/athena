# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PRWOutputCfg(flags, output_name="NTUP_PILEUP"):
    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg # NB In main this function has moved to MuonConfig.MuonConfigUtils
    return setupHistSvcCfg(flags, out_file = flags.Output.HISTFileName, out_stream = output_name)


def PileupReweightingProviderCfg(flags, **kwargs):
    cfg = ComponentAccumulator()
    kwargs.setdefault("ConfigOutputStream", "NTUP_PILEUP")
    from AsgAnalysisAlgorithms.PileupReweightingAlgConfig import PileupReweightingProviderToolCfg
    kwargs.setdefault("Tool", cfg.addPublicTool(cfg.popToolsAndMerge(PileupReweightingProviderToolCfg(flags))))
    cfg.addEventAlgo(CompFactory.CP.PileupReweightingProvider(**kwargs))
    return cfg


def PRWSteeringCfg(flags):
    cfg = ComponentAccumulator()
    cfg.merge(PileupReweightingProviderCfg(flags))

    #include("AthAnalysisBaseComps/SuppressLogging.py")       #Optional include to suppress as much athena output as possible
    #https://gitlab.cern.ch/atlas/athena/-/blob/release/23.0.20/Control/AthAnalysisBaseComps/share/SuppressLogging.py
    cfg.merge(PRWOutputCfg(flags))
    return cfg


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('PRWConfig_tf')
    log.info( '****************** STARTING AODtoNTUP_PILEUP *****************' )

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, 'inputAODFile'):
        flags.Input.Files = runArgs.inputAODFile
    else:
        raise RuntimeError('No input AOD file defined')

    if hasattr(runArgs, 'outputNTUP_PILEUPFile'):
        flags.Output.HISTFileName  = runArgs.outputNTUP_PILEUPFile # FIXME Add a separate output file type flag??
    else:
        log.warning('No output file set')
        flags.Output.HISTFileName = 'output.NTUP_PILEUP.root' # FIXME Add a separate output file type flag??

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

    cfg.merge(PRWSteeringCfg(flags))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran PRWConfig_tf in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
