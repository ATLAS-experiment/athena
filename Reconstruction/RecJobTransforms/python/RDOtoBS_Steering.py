# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod

from AthenaCommon.Logging import logging

__log = logging.getLogger('RDOtoBS_Steering')


def RDOtoBS_Steering(flags):
    acc = ComponentAccumulator()

    itemList = []

    # LAr
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    from LArByteStream.LArByteStreamConfig import LArRawDataContByteStreamToolCfg
    larBS, larExtraInputs = LArRawDataContByteStreamToolCfg(flags, InitializeForWriting=True,DSPRunMode = 4, RodBlockVersion = 10)
    acc.merge(larBS)
    itemList += ["LArRawChannelContainer#*"]
    # Tile
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))
    from TileByteStream.TileByteStreamConfig import TileRawChannelContByteStreamToolCfg
    tileCfg, tileExtraInputs = TileRawChannelContByteStreamToolCfg(flags, InitializeForWriting=True)
    acc.merge(tileCfg)
    itemList += ["TileRawChannelContainer#*"]
    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if flags.GeoModel.Run is LHCPeriod.Run4:
        #ITk pixel
        from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
        acc.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))

        from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
        acc.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

        from ITkPixelByteStreamCnv.ITkPixelEncodingAlgConfig import ITkPixelEncodingAlgCfg
        acc.merge( ITkPixelEncodingAlgCfg(flags) )
        itemList += ['ITkPixelRDO_Container#*']

    else:
        if flags.Trigger.enableL1CaloLegacy or not flags.Trigger.enableL1MuonPhase1:
            itemList += ["ROIB::RoIBResult#RoIBResult"]

        if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
            itemList += ["xAOD::TrigCompositeContainer#L1TriggerResult"]

        from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamEncoderCfg
        acc.merge(L1TriggerByteStreamEncoderCfg(flags))

        from InDetConfig.InDetPrepRawDataFormationConfig import (
            PixelClusterizationCfg,
            SCTClusterizationCfg,
            InDetTRT_RIO_MakerCfg,
        )

        # Pixel
        from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg, PixelHitDiscCnfgAlgCfg
        acc.merge(PixelCablingCondAlgCfg(flags))
        acc.merge(PixelHitDiscCnfgAlgCfg(flags))
        acc.merge(PixelClusterizationCfg(flags))
        itemList += ["PixelRDO_Container#*"]
        # SCT

        acc.merge(SCTClusterizationCfg(flags))
        itemList += ["SCT_RDO_Container#*"]
        # TRT
        acc.merge(InDetTRT_RIO_MakerCfg(flags))
        itemList += ["TRT_RDO_Container#*"]

        from MuonConfig.MuonRdoDecodeConfig import (
            MuonRDOtoPRDConvertorsCfg
        )

        acc.merge(MuonRDOtoPRDConvertorsCfg(flags))

        # MDT
        itemList += ["MdtCsmContainer#*"]
        # RPC
        itemList += ["RpcPadContainer#*"]
        # TGC
        itemList += ["TgcRdoContainer#*"]
        # MMG -- no converter?
        # itemList += ["Muon::MM_RawDataContainer#*"]
        # sTGC -- no converter?
        # itemList += ["Muon::STGC_RawDataContainer#*"]

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    acc.merge(ByteStreamWriteCfg(flags, itemList))

    return acc

