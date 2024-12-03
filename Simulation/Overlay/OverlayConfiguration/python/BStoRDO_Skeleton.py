# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.Enums import MetadataCategory, ProductionStep
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('BStoRDO')
    log.info('****************** STARTING BStoRDO *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    flags.Common.ProductionStep = ProductionStep.MinbiasPreprocessing

    # This is for data overlay
    flags.Overlay.DataOverlay = True

    # Setting input/output files
    if hasattr(runArgs, 'inputBSFile'):
        flags.Input.Files = runArgs.inputBSFile
    else:
        raise ValueError('No input BS file defined')

    if hasattr(runArgs, 'outputRDO_BKGFile'):
        flags.Output.RDOFileName = runArgs.outputRDO_BKGFile
    else:
        raise RuntimeError('No output RDO file defined')

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True)

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

    itemList = [] # items to store in RDO

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))

    if flags.Detector.EnablePixel:
        from InDetOverlay.PixelOverlayConfig import PixelDataOverlayExtraCfg
        cfg.merge(PixelDataOverlayExtraCfg(flags))
        itemList.append(f'PixelRDO_Container#{flags.Overlay.BkgPrefix}PixelRDOs')

    if flags.Detector.EnableSCT:
        from InDetOverlay.SCTOverlayConfig import SCTDataOverlayExtraCfg
        cfg.merge(SCTDataOverlayExtraCfg(flags))
        itemList.append(f'SCT_RDO_Container#{flags.Overlay.BkgPrefix}SCT_RDOs')

    if flags.Detector.EnableTRT:
        from InDetOverlay.TRTOverlayConfig import TRTDataOverlayExtraCfg
        cfg.merge(TRTDataOverlayExtraCfg(flags))
        itemList.append(f'TRT_RDO_Container#{flags.Overlay.BkgPrefix}TRT_RDOs')

    if flags.Detector.EnableLAr:
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        cfg.merge(LArRawDataReadingCfg(flags, LArRawChannelKey=f'{flags.Overlay.BkgPrefix}LArRawChannels'))
        itemList.append(f'LArRawChannelContainer#{flags.Overlay.BkgPrefix}LArRawChannels')

    if flags.Detector.EnableTile:
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge(TileRawDataReadingCfg(flags))
        itemList.append(f'TileRawChannelContainer#{flags.Overlay.BkgPrefix}TileRawChannelCnt')

    if flags.Detector.EnableCSC:
        from MuonConfig.CSC_OverlayConfig import CSC_DataOverlayExtraCfg
        cfg.merge(CSC_DataOverlayExtraCfg(flags))
        itemList.append(f'CscRawDataContainer#{flags.Overlay.BkgPrefix}CSCRDO')

    if flags.Detector.EnableMDT:
        from MuonConfig.MDT_OverlayConfig import MDT_DataOverlayExtraCfg
        cfg.merge(MDT_DataOverlayExtraCfg(flags))
        itemList.append(f'MdtCsmContainer#{flags.Overlay.BkgPrefix}MDTCSM')

    if flags.Detector.EnableRPC:
        from MuonConfig.RPC_OverlayConfig import RPC_DataOverlayExtraCfg
        cfg.merge(RPC_DataOverlayExtraCfg(flags))
        itemList.append(f'RpcPadContainer#{flags.Overlay.BkgPrefix}RPCPAD')

    if flags.Detector.EnableTGC:
        from MuonConfig.TGC_OverlayConfig import TGC_DataOverlayExtraCfg
        cfg.merge(TGC_DataOverlayExtraCfg(flags))
        itemList.append(f'TgcRdoContainer#{flags.Overlay.BkgPrefix}TGCRDO')

    if flags.Detector.EnablesTGC:
        from MuonConfig.sTGC_OverlayConfig import sTGC_DataOverlayExtraCfg
        cfg.merge(sTGC_DataOverlayExtraCfg(flags))
        itemList.append(f'Muon::STGC_RawDataContainer#{flags.Overlay.BkgPrefix}sTGCRDO')
    
    if flags.Detector.EnableMM:
        from MuonConfig.MM_OverlayConfig import MM_DataOverlayExtraCfg
        cfg.merge(MM_DataOverlayExtraCfg(flags))
        itemList.append(f'Muon::MM_RawDataContainer#{flags.Overlay.BkgPrefix}MMRDO')

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, 'RDO', itemList))

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, 'RDO', createMetadata=[MetadataCategory.IOVMetaData]))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
