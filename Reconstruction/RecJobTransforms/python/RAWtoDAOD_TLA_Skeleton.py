# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from RecJobTransforms.RecoSteering import RecoSteering

from AthenaCommon.Logging import logging
log = logging.getLogger('RAWtoDAOD_TLA')


def configureFlags(runArgs):
    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)

    # Input
    if hasattr(runArgs, 'inputBSFile'):
        log.warning("Enters the inputBSFile if")
        flags.Input.Files = runArgs.inputBSFile

    # Output
    if hasattr(runArgs, 'outputDAOD_TLAFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAFile
        log.info("---------- Configured DAOD_TLA output")
        flags.Trigger.AODEDMSet='PhysicsTLA'
        from AthenaConfiguration.DetectorConfigFlags import allDetectors
        disabled_detectors = allDetectors
    elif hasattr(runArgs, 'outputDAOD_TLAFTAGPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAFTAGPEBFile
        log.info("---------- Configured DAOD_TLAFTAGPEB output")
        flags.Trigger.AODEDMSet='FTagPEBTLA'
        disabled_detectors = [
            'TRT',
            'LAr', 'Tile', 'MBTS',
            'CSC', 'MDT', 'RPC', 'TGC',
            'sTGC', 'MM',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]
    elif hasattr(runArgs, 'outputDAOD_TLADJETPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLADJETPEBFile
        log.info("---------- Configured DAOD_TLADJETPEB output")
        flags.Trigger.AODEDMSet='DarkJetPEBTLA'
        disabled_detectors = [
            'MBTS',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]
    elif hasattr(runArgs, 'outputDAOD_TLAEGAMPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAEGAMPEBFile
        log.info("---------- Configured DAOD_TLAEGAMPEB output")
        flags.Trigger.AODEDMSet='EgammaPEBTLA'
        disabled_detectors = [
            'MBTS',
            'CSC', 'MDT', 'RPC', 'TGC',
            'sTGC', 'MM',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]

    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    # Set non-default flags 
    flags.Trigger.doLVL1=False
    flags.Trigger.DecisionMakerValidation.Execute = False
    flags.Trigger.doNavigationSlimming = False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doCTP=False

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Reconstruction

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    if GetFileMD(flags.Input.Files)["GeoAtlas"] is None:
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import disableDetectors
    disableDetectors(
        flags, toggle_geometry=True,
        detectors=disabled_detectors,
    )

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    return flags



def fromRunArgs(runArgs):

    log.info('****************** STARTING TLA RAW Decoding (RAWtoDAOD_TLA) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    flags = configureFlags(runArgs)
    log.info("Configuring according to flag values listed below")
    flags.dump()

    cfg = RecoSteering(flags)

    # import the TLA decoding
    cfg.flagPerfmonDomain('Trigger')

    # add additional objects reconstructed in RAWtoDAOD step (i.e. not in trigger EDM)
    additional_output_items = {
        'PhysicsTLA': [],
        'FTagPEBTLA':
        [
            'xAOD::BTaggingContainer#BTagging_HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA',
            'xAOD::BTaggingAuxContainer#BTagging_HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLAAux.',
        ],
        'DarkJetPEBTLA': [],
        'EgammaPEBTLA': [],
    }[flags.Trigger.AODEDMSet]
    from TLARecoConfig.DAOD_TLA_OutputConfig import DAOD_TLA_OutputCfg
    cfg.merge( DAOD_TLA_OutputCfg(flags, additional_output_items) )

    if flags.Trigger.AODEDMSet == 'FTagPEBTLA':
        from TLARecoConfig.FTagPEBRecoConfig import FTagPEBJetTagConfig
        cfg.merge(FTagPEBJetTagConfig(flags))

    # setup Metadata writer
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags,'AOD'))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaCommon.Constants import INFO
    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig()

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoDAOD_TLA_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(sc.isFailure())
