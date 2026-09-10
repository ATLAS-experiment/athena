# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod

from AthenaCommon.Logging import logging

__log = logging.getLogger('RDOtoBS_Steering')


def RDOtoBS_Steering(flags):
    acc = ComponentAccumulator()

    itemList = []

    # LAr
    if flags.Detector.EnableLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        acc.merge(LArGMCfg(flags))
        from LArByteStream.LArByteStreamConfig import LArRawDataContByteStreamToolCfg
        larBS, larExtraInputs = LArRawDataContByteStreamToolCfg(flags, InitializeForWriting=True,DSPRunMode = 4, RodBlockVersion = 10)
        acc.merge(larBS)
        itemList += ["LArRawChannelContainer#*"]
    # Tile
    if flags.Detector.EnableTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        acc.merge(TileGMCfg(flags))
        from TileByteStream.TileByteStreamConfig import TileRawChannelContByteStreamToolCfg
        tileCfg, tileExtraInputs = TileRawChannelContByteStreamToolCfg(flags, InitializeForWriting=True)
        acc.merge(tileCfg)
        itemList += ["TileRawChannelContainer#*"]
        from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
        acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        # ITk pixel
        if flags.Detector.EnableITkPixel:
            from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
            acc.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

            from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
            acc.merge( ITkPixelEncodingAlgCfg(flags) )
            itemList += ['ITkPixelRDO_Container#ITkPixelRDOs']

        # ITk strip
        if flags.Detector.EnableITkStrip:
            from ITkStripsByteStreamCnv.ITkStripByteStreamCnvConfig import ITkStripRawContByteStreamToolProviderToolCfg
            acc.merge(ITkStripRawContByteStreamToolProviderToolCfg(flags))
            itemList += ['SCT_RDO_Container#ITkStripRDOs']

    else:
        # Pixel
        if flags.Detector.EnablePixel:
            from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
            acc.merge (PixelReadoutManagerCfg(flags))
            from InDetConfig.InDetPrepRawDataFormationConfig import PixelClusterizationCfg
            from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg, PixelHitDiscCnfgAlgCfg
            acc.merge(PixelCablingCondAlgCfg(flags))
            acc.merge(PixelHitDiscCnfgAlgCfg(flags))
            acc.merge(PixelClusterizationCfg(flags))
            itemList += ["PixelRDO_Container#*"]

        # SCT
        if flags.Detector.EnableSCT:
            from SCT_RawDataByteStreamCnv.testSCTEncodeNewConf import SCTRawContByteStreamToolProviderToolCfg
            acc.merge(SCTRawContByteStreamToolProviderToolCfg(flags))
            from InDetConfig.InDetPrepRawDataFormationConfig import SCTClusterizationCfg
            acc.merge(SCTClusterizationCfg(flags))
            itemList += ["SCT_RDO_Container#*"]

        # TRT
        if flags.Detector.EnableTRT:
            from InDetConfig.InDetPrepRawDataFormationConfig import InDetTRT_RIO_MakerCfg
            acc.merge(InDetTRT_RIO_MakerCfg(flags))
            itemList += ["TRT_RDO_Container#*"]

    # Muon
    if flags.Detector.EnableMuon:
        from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
        acc.merge(MuonRDOtoPRDConvertorsCfg(flags))

        # MDT
        if flags.Detector.EnableMDT:
            itemList += ["MdtCsmContainer#*"]
        # RPC
        if flags.Detector.EnableRPC:
            itemList += ["RpcPadContainer#*"]
        # TGC
        if flags.Detector.EnableTGC:
            itemList += ["TgcRdoContainer#*"]
        # MMG -- no converter?
        # itemList += ["Muon::MM_RawDataContainer#*"]
        # sTGC -- no converter?
        # itemList += ["Muon::STGC_RawDataContainer#*"]

    # L1 trigger
    if flags.Trigger.enableL1CaloLegacy or not flags.Trigger.enableL1MuonPhase1:
          itemList += ["ROIB::RoIBResult#RoIBResult"]

    if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
          itemList += ["xAOD::TrigCompositeContainer#L1TriggerResult"]

    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamEncoderCfg
    acc.merge(L1TriggerByteStreamEncoderCfg(flags))

    # MC EventInfo encoding (for MC ByteStream)
    if flags.Input.isMC:
        from ByteStreamCnvSvc.ByteStreamConfig import MCEventInfoByteStreamToolCfg
        mcEventInfoTool = acc.popToolsAndMerge(MCEventInfoByteStreamToolCfg(flags, writeBS=True))
        acc.addPublicTool(mcEventInfoTool)
        itemList += ["xAOD::EventAuxInfo#*"]

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    acc.merge(ByteStreamWriteCfg(flags, itemList))

    return acc

