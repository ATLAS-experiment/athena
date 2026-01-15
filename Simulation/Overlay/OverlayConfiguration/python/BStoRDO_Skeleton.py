# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.Enums import MetadataCategory, ProductionStep, HIMode
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

    flags.Reco.EnableHI = True
    flags.Reco.HIMode = HIMode.HI
    flags.Tracking.doCaloSeededAmbi = False
    flags.Tracking.doCaloSeededBrem = False

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
    acceptAlgs = [] # skimming algs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    cfg.merge(CutFlowSvcCfg(flags))

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))

    from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
    cfg.merge(LumiBlockMuWriterCfg(flags))

    if flags.Detector.EnableBCM:
        from InDetOverlay.BCMOverlayConfig import BCMRawDataProviderAlgCfg
        cfg.merge(BCMRawDataProviderAlgCfg(flags))
        itemList.append(f'BCM_RDO_Container#{flags.Overlay.BkgPrefix}BCM_RDOs')

    if flags.Detector.EnablePixel:
        from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import PixelRawDataProviderAlgCfg
        cfg.merge(PixelRawDataProviderAlgCfg(flags))
        itemList.append(f'PixelRDO_Container#{flags.Overlay.BkgPrefix}PixelRDOs')
        itemList.append('IDCInDetBSErrContainer#PixelByteStreamErrs')

    if flags.Detector.EnableSCT:
        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import SCTRawDataProviderCfg, SCTEventFlagWriterCfg
        cfg.merge(SCTRawDataProviderCfg(flags))
        cfg.merge(SCTEventFlagWriterCfg(flags))
        itemList.append(f'SCT_RDO_Container#{flags.Overlay.BkgPrefix}SCT_RDOs')
        itemList.append('IDCInDetBSErrContainer#SCT_ByteStreamErrs')

    if flags.Detector.EnableTRT:
        from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import TRTRawDataProviderCfg
        cfg.merge(TRTRawDataProviderCfg(flags))
        itemList.append(f'TRT_RDO_Container#{flags.Overlay.BkgPrefix}TRT_RDOs')
        itemList.append('TRT_BSErrContainer#TRT_ByteStreamErrs')

    if flags.Detector.EnableLAr:
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        cfg.merge(LArRawDataReadingCfg(flags))
        itemList.append(f'LArDigitContainer#{flags.Overlay.BkgPrefix}LArDigitContainer_data')
        itemList.append('LArFebHeaderContainer#LArFebHeader')

    if flags.Detector.EnableTile:
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge(TileRawDataReadingCfg(flags, readMuRcv=False))
        itemList.append(f'TileRawChannelContainer#{flags.Overlay.BkgPrefix}TileRawChannelCnt')
        itemList.append(f'TileDigitsContainer#{flags.Overlay.BkgPrefix}TileDigitsCnt')

    if flags.Detector.EnableCSC:
        from MuonConfig.MuonBytestreamDecodeConfig import CscBytestreamDecodeCfg
        cfg.merge(CscBytestreamDecodeCfg(flags))
        itemList.append(f'CscRawDataContainer#{flags.Overlay.BkgPrefix}CSCRDO')

    if flags.Detector.EnableMDT:
        from MuonConfig.MuonBytestreamDecodeConfig import MdtBytestreamDecodeCfg
        cfg.merge(MdtBytestreamDecodeCfg(flags))
        itemList.append(f'MdtCsmContainer#{flags.Overlay.BkgPrefix}MDTCSM')

    if flags.Detector.EnableRPC:
        from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg
        cfg.merge(RpcBytestreamDecodeCfg(flags))
        itemList.append(f'RpcPadContainer#{flags.Overlay.BkgPrefix}RPCPAD')

    if flags.Detector.EnableTGC:
        from MuonConfig.MuonBytestreamDecodeConfig import TgcBytestreamDecodeCfg
        cfg.merge(TgcBytestreamDecodeCfg(flags))
        itemList.append(f'TgcRdoContainer#{flags.Overlay.BkgPrefix}TGCRDO')

    if flags.Detector.EnablesTGC:
        from MuonConfig.MuonBytestreamDecodeConfig import sTgcBytestreamDecodeCfg
        cfg.merge(sTgcBytestreamDecodeCfg(flags))
        itemList.append(f'Muon::STGC_RawDataContainer#{flags.Overlay.BkgPrefix}sTGCRDO')
    
    if flags.Detector.EnableMM:
        from MuonConfig.MuonBytestreamDecodeConfig import MmBytestreamDecodeCfg
        cfg.merge(MmBytestreamDecodeCfg(flags))
        itemList.append(f'Muon::MM_RawDataContainer#{flags.Overlay.BkgPrefix}MMRDO')

    if flags.Reco.EnableTracking:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        cfg.merge(InDetTrackRecoCfg(flags))
        itemList.append(f'xAOD::VertexContainer#{flags.Overlay.BkgPrefix}PrimaryVertices')
        itemList.append(f'xAOD::VertexAuxContainer#{flags.Overlay.BkgPrefix}PrimaryVerticesAux.x.y.z')

        from OverlayUtilities.OverlayUtilitiesConfig import OverlayVertexSkimmingAlgCfg
        cfg.merge(OverlayVertexSkimmingAlgCfg(flags))
        acceptAlgs.append('OverlayVertexSkimmingAlg')

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, 'RDO', AcceptAlgs=acceptAlgs,
                                        createMetadata=[MetadataCategory.CutFlowMetaData]))

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
