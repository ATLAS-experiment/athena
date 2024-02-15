# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('RAWtoALL')
    log.info('****************** STARTING Reconstruction (RAWtoALL) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    log.info('**** executing ROOT6Setup')
    from PyUtils.Helpers import ROOT6Setup
    ROOT6Setup(batch=True)

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    # TODO: event service?

    ## Inputs
    # BS
    inputsDRAW = [prop for prop in dir(runArgs) if prop.startswith('inputDRAW') and prop.endswith('File')]
    if hasattr(runArgs, 'inputBSFile'):
        if inputsDRAW:
            raise RuntimeError('Impossible to run RAWtoALL with input BS and DRAW files (one input type only!)')
        flags.Input.Files = runArgs.inputBSFile
    if len(inputsDRAW) == 1:
        flags.Input.Files = getattr(runArgs, inputsDRAW[0])
    elif len(inputsDRAW) > 1:
        raise RuntimeError('Impossible to run RAWtoALL with multiple input DRAW files (viz.: {0})'.format(inputsDRAW))

    # RDO
    if hasattr(runArgs, 'inputRDOFile'):
        flags.Input.Files = runArgs.inputRDOFile
    if hasattr(runArgs, 'inputRDO_TRIGFile'):
        flags.Input.Files = runArgs.inputRDO_TRIGFile
        # TODO: trigger setup

    # TODO: EVNT?

    ## Outputs
    if hasattr(runArgs, 'outputESDFile'):
        flags.Output.ESDFileName = runArgs.outputESDFile
        log.info("---------- Configured ESD output")

    if hasattr(runArgs, 'outputAODFile'):
        flags.Output.AODFileName = runArgs.outputAODFile
        log.info("---------- Configured AOD output")

    if hasattr(runArgs, 'outputHISTFile'):
        flags.Output.HISTFileName = runArgs.outputHISTFile
        flags.DQ.doMonitoring = True
        log.info("---------- Configured HIST output")

    if hasattr(runArgs, 'outputHIST_R2AFile'):
        flags.Output.HISTFileName = runArgs.outputHIST_R2AFile
        flags.DQ.doMonitoring = True
        log.info("---------- Configured HIST_R2A output")

    if hasattr(runArgs, 'outputDAOD_IDTIDEFile'):
        flagString = 'Output.DAOD_IDTIDEFileName'
        flags.addFlag(flagString, runArgs.outputDAOD_IDTIDEFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDAOD_IDTIDE', True)
        log.info("---------- Configured DAOD_IDTIDE output")

    if hasattr(runArgs, 'outputDAOD_PIXELVALIDFile'):
        flagString = 'Output.DAOD_PIXELVALIDFileName'
        flags.addFlag(flagString, runArgs.outputDAOD_PIXELVALIDFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDAOD_PIXELVALID', True)
        log.info("---------- Configured DAOD_PIXELVALID output")

    if hasattr(runArgs, 'outputDAOD_IDTRKVALIDFile'):
        flagString = 'Output.DAOD_IDTRKVALIDFileName'
        flags.addFlag(flagString, runArgs.outputDAOD_IDTRKVALIDFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDAOD_IDTRKVALID', True)
        log.info("---------- Configured DAOD_IDTRKVALID output")

    if hasattr(runArgs, 'outputDESDM_MCPFile'):
        flagString = 'Output.DESDM_MCPFileName'
        flags.addFlag(flagString, runArgs.outputDESDM_MCPFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDESDM_MCP', True)
        log.info("---------- Configured DESDM_MCP output")
    
    if hasattr(runArgs, 'outputDRAW_ZMUMUFile'):
        flagString = 'Output.DRAW_ZmumuFileName'
        flags.addFlag(flagString, runArgs.outputDRAW_ZMUMUFile)
        flags.Output.doWriteBS = True
        log.info("---------- Configured DRAW ZMUMU output")

    if hasattr(runArgs, 'outputDRAW_EGZFile'):
        flagString = 'Output.DRAW_EGZFileName'
        flags.addFlag(flagString, runArgs.outputDRAW_EGZFile)
        flags.Output.doWriteBS = True
        log.info("---------- Configured DRAW_EGZ output")

    if hasattr(runArgs, 'outputDRAW_TAULHFile'):
        flagString = 'Output.DRAW_TAULHFileName'
        flags.addFlag(flagString, runArgs.outputDRAW_TAULHFile)
        flags.Output.doWriteBS = True
        log.info("---------- Configured DRAW_TAULH output")

    if hasattr(runArgs, 'outputDRAW_JETFile'):
        flagString = 'Output.DRAW_JETFileName'
        flags.addFlag(flagString, runArgs.outputDRAW_JETFile)
        flags.Output.doWriteBS = True
        log.info("---------- Configured DRAW_JET output")

    if hasattr(runArgs, 'outputDAOD_L1CALO1File'):
        flagString = 'Output.DAOD_L1CALO1FileName'
        flags.addFlag(flagString, runArgs.outputDAOD_L1CALO1File)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDAOD_L1CALO1', True)
        log.info("---------- Configured DAOD_L1CALO1 output")

    if hasattr(runArgs, 'outputDAOD_L1CALO2File'):
        flagString = 'Output.DAOD_L1CALO2FileName'
        flags.addFlag(flagString, runArgs.outputDAOD_L1CALO2File)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDAOD_L1CALO2', True)
        log.info("---------- Configured DAOD_L1CALO2 output")

    if hasattr(runArgs, 'outputDESDM_PHOJETFile'):
        flagString = 'Output.DESDM_PHOJETFileName'
        flags.addFlag(flagString, runArgs.outputDESDM_PHOJETFile)
        flags.Output.doWriteDAOD = True
        flags.addFlag('Output.doWriteDESDM_PHOJET', True)
        log.info("---------- Configured DESDM_PHOJET output")

    if hasattr(runArgs, 'outputDESDM_ALLCELLSFile'):
        streamName = 'DESDM_ALLCELLS'
        flags.addFlag(f'Output.{streamName}FileName', runArgs.outputDESDM_ALLCELLSFile)
        flags.addFlag(f'Output.doWrite{streamName}', True)
        log.info("---------- Configured "+streamName+" output")

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

    # TODO: DESD, DAOD and DRAW

    # outputHIST_R2AFile
    # outputTAG_COMMFile
    # outputTXT_FTKIPFile
    # outputNTUP_MUONCALIBFile
    # outputTXT_JIVEXMLTGZFile

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    # Main reconstruction steering
    from RecJobTransforms.RecoSteering import RecoSteering
    cfg = RecoSteering(flags)

    # Performance DPDs 
    cfg.flagPerfmonDomain('PerfDPD')
    # IDTIDE
    for flag in [key for key in flags._flagdict.keys() if ("Output.DAOD_IDTIDEFileName" in key)]:
        from DerivationFrameworkInDet.IDTIDE import IDTIDECfg
        cfg.merge(IDTIDECfg(flags))
        log.info("---------- Configured IDTIDE perfDPD")

    # PixelVALID
    for flag in [key for key in flags._flagdict.keys() if ("Output.DAOD_PIXELVALIDFileName" in key)]:
        from DerivationFrameworkInDet.PIXELVALID import PixelVALIDCfg
        cfg.merge(PixelVALIDCfg(flags))
        log.info("---------- Configured PixelVALID perfDPD")

    # IDTRKVALID
    for flag in [key for key in flags._flagdict.keys() if ("Output.DAOD_IDTRKVALIDFileName" in key)]:
        from DerivationFrameworkInDet.IDTRKVALID import IDTRKVALIDCfg
        cfg.merge(IDTRKVALIDCfg(flags))
        log.info("---------- Configured IDTRKVALID perfDPD")

    # DESDM_MCP
    for flag in [key for key in flags._flagdict.keys() if ("Output.DESDM_MCPFileName" in key)]:
        from PrimaryDPDMaker.DESDM_MCP import DESDM_MCPCfg
        cfg.merge(DESDM_MCPCfg(flags))
        log.info("---------- Configured DESDM_MCP perfDPD")

    # DRAW ZMUMU
    for flag in [key for key in flags._flagdict.keys() if ("Output.DRAW_ZmumuFileName" in key)]:
        from PrimaryDPDMaker.DRAW_ZMUMU import DRAW_ZmumuCfg
        cfg.merge(DRAW_ZmumuCfg(flags))
        log.info("---------- Configured DRAW_ZMUMU perfDPD")

    #DRAW_EGZ
    for flag in [key for key in flags._flagdict.keys() if ("Output.DRAW_EGZFileName" in key)]:
        from PrimaryDPDMaker.DRAW_EGZ import DRAW_EGZCfg
        cfg.merge(DRAW_EGZCfg(flags))
        log.info("---------- Configured DRAW_EGZ perfDPD")

    #DRAW_TAULH
    for flag in [key for key in flags._flagdict.keys() if ("Output.DRAW_TAULHFileName" in key)]:
        from PrimaryDPDMaker.DRAW_TAULH import DRAW_TAULHCfg
        cfg.merge(DRAW_TAULHCfg(flags))
        log.info("---------- Configured DRAW_TAULH perfDPD")

    #DRAW_JET
    for flag in [key for key in flags._flagdict.keys() if ("Output.DRAW_JETFileName" in key)]:
        from PrimaryDPDMaker.DRAW_JET import DRAW_JETCfg
        cfg.merge(DRAW_JETCfg(flags))
        log.info("---------- Configured DRAW_JET perfDPD")

    # L1CALO1/2
    for flag in [key for key in flags._flagdict.keys() if ("Output.DAOD_L1CALO1FileName" in key)]:
        from DerivationFrameworkL1Calo.L1CALO1 import L1CALO1Cfg
        cfg.merge(L1CALO1Cfg(flags))
        log.info("---------- Configured L1CALO1 perfDPD")

    for flag in [key for key in flags._flagdict.keys() if ("Output.DAOD_L1CALO2FileName" in key)]:
        from DerivationFrameworkL1Calo.L1CALO2 import L1CALO2Cfg
        cfg.merge(L1CALO2Cfg(flags))
        log.info("---------- Configured L1CALO2 perfDPD")

    # DESDM PHOJET
    for flag in [key for key in flags._flagdict.keys() if ("Output.DESDM_PHOJETFileName" in key)]:
        from PrimaryDPDMaker.DESDM_PHOJET import DESDM_PHOJETCfg
        cfg.merge(DESDM_PHOJETCfg(flags))
        log.info("---------- Configured PHOJET perfDPD")

    # DESDM ALLCELLS
    for flag in [key for key in flags._flagdict.keys() if ("Output.DESDM_ALLCELLSFileName" in key)]:
        from PrimaryDPDMaker.DESDM_ALLCELLS import DESDM_ALLCELLSCfg
        cfg.merge(DESDM_ALLCELLSCfg(flags))
        log.info("---------- Configured ALLCELLS perfDPD")

    # Special message service configuration
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(flags))

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
