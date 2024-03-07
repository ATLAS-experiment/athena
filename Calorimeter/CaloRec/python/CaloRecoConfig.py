
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


def CaloRecoCfg(flags, clustersname=None):
    result = ComponentAccumulator()
    if flags.Input.Format is Format.BS:
        #Data-case: Schedule ByteStream reading for LAr & Tile
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        result.merge(LArRawDataReadingCfg(flags))

        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        result.merge( TileRawDataReadingCfg(flags) )

        if flags.Output.doWriteESD:
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterOutputCfg
            result.merge(TileDigitsFilterOutputCfg(flags))
        else: #Mostly for wrapping in RecExCommon
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
            result.merge(TileDigitsFilterCfg(flags))

        from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
        result.merge(LArRawChannelBuilderAlgCfg(flags))

        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        result.merge(TileRawChannelMakerCfg(flags))

    if not flags.Input.isMC and not flags.Common.isOnline:
        from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
        result.merge(LArTimeVetoAlgCfg(flags))

    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
        result.merge(LArFebErrorSummaryMakerCfg(flags))


    #Configure cell-building
    from CaloRec.CaloCellMakerConfig import CaloCellMakerCfg
    result.merge(CaloCellMakerCfg(flags))

    #Configure topo-cluster builder
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    result.merge(CaloTopoClusterCfg(flags, clustersname=clustersname))

    #Configure forward towers:
    from CaloRec.CaloFwdTopoTowerConfig import CaloFwdTopoTowerCfg
    result.merge(CaloFwdTopoTowerCfg(flags,CaloTopoClusterContainerKey="CaloCalTopoClusters"))

    #Configure NoisyROSummary
    from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
    result.merge(LArNoisyROSummaryCfg(flags))

    #Configure TileLookForMuAlg
    from TileMuId.TileMuIdConfig import TileLookForMuAlgCfg
    result.merge(TileLookForMuAlgCfg(flags))

    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        #Configure LArDigitsThinner:
        from LArROD.LArDigitThinnerConfig import LArDigitThinnerCfg
        result.merge(LArDigitThinnerCfg(flags))
        
    #Configure MBTSTimeDiff
    #Clients are BackgroundWordFiller and (deprecated?) DQTBackgroundMonTool
    #Consider moving to BackgroundWordFiller config
    if flags.Detector.GeometryMBTS:
        from TileRecAlgs.MBTSTimeDiffEventInfoAlgConfig import MBTSTimeDiffEventInfoAlgCfg
        result.merge(MBTSTimeDiffEventInfoAlgCfg(flags))


    #Configure AOD Cell-Thinning based on samplings:
    from CaloRec.CaloThinCellsBySamplingAlgConfig import CaloThinCellsBySamplingAlgCfg
    result.merge(CaloThinCellsBySamplingAlgCfg(flags,'StreamAOD', ['TileGap3']))
        
    # Optional: AOD Cell, rawCh, digits thinning, based on clusters, for XTalk studies
    if flags.Calo.TopoCluster.xtalkInfoDumper and not flags.Overlay.DataOverlay:
        from LArClusterCellDumper.CaloThinCellsInAODAlgConfig import CaloThinCellsInAODAlgCfg
        result.merge(CaloThinCellsInAODAlgCfg(flags))

    return result


def CaloRecoDebuggingCfg(flags):
    result = ComponentAccumulator()

    result.addEventAlgo(CompFactory.DumpLArRawChannels(LArRawChannelContainerName="LArRawChannels_FromDigits"))
    result.addEventAlgo(CompFactory.CaloCellDumper())

    ClusterDumper = CompFactory.ClusterDumper
    result.addEventAlgo(ClusterDumper("TopoDumper", ContainerName="CaloCalTopoClusters", FileName="TopoCluster.txt"))
    result.addEventAlgo(ClusterDumper("FwdTopoDumper", ContainerName="CaloCalFwdTopoTowers", FileName="FwdTopoCluster.txt"))

    return result


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG,INFO
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    log.setLevel(DEBUG)
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data",]
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

    flags.Exec.OutputLevel=INFO
    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    acc.merge(CaloRecoCfg(flags))


    CaloCellDumper=CompFactory.CaloCellDumper
    acc.addEventAlgo(CaloCellDumper(),sequenceName="AthAlgSeq")

    ClusterDumper=CompFactory.ClusterDumper
    acc.addEventAlgo(ClusterDumper("TopoDumper",ContainerName="CaloCalTopoClusters",FileName="TopoCluster.txt"),sequenceName="AthAlgSeq")

    f=open("CaloRec.pkl","wb")
    acc.store(f)
    f.close()

    acc.run(10)
