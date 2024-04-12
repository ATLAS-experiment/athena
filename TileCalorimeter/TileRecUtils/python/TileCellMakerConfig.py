# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile Cell maker algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType
from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellNeighborsAverageCorrCfg


def CaloCellContainerCheckerToolCfg(flags):
    """Return component accumulator with configured private Calo cell container checker tool

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    CaloCellContainerCheckerTool=CompFactory.CaloCellContainerCheckerTool
    acc.setPrivateTools( CaloCellContainerCheckerTool() )

    return acc


def TileCellMakerCfg(flags, mergeChannels=True, **kwargs):
    """Return component accumulator with configured Tile Cell maker algorithm

    Arguments:
        flags  -- Athena configuration flags
        name -- name of Tile cell maker algorithm. Defautls to TileCellMaker
                or TileCellMakerHG/TileCellMakerLG depending on only used gain.
        SkipGain - skip given gain. Defaults to -1 [use all gains]. Possible values: 0 [LG], 1 [HG].
        CaloCellsOutputName -- name of the output calo cell container. Defautls to AllCalo
                               or AllCaloHG/AllCaloLG depending on only used gain.
        DoCaloNeighborsCorrection -- correct dead cells. Assign as energy the average energy of
                                     the surrounding cells. Defaults to False.
        mergeChannels -- merge DSP results with offline reco results. Defaults to True.
    """

    acc = ComponentAccumulator()

    useGain = {0 : 'HG', 1 : 'LG'} # Key is skipped gain

    skipGain = kwargs.get('SkipGain', -1) # Never skip any gain by default

    defaultName = 'TileCellMaker' if skipGain == -1 else 'TileCellMaker' + useGain[skipGain]
    name = kwargs.get('name', defaultName)

    defaultOutputCells = 'AllCalo' if skipGain == -1 else 'AllCalo' + useGain[skipGain]
    caloCellsOutputName = kwargs.get('CaloCellsOutputName', defaultOutputCells)

    doCaloNeighborsCorrection = kwargs.get('DoCaloNeighborsCorrection', False)

    from AthenaCommon.Logging import logging
    msg = logging.getLogger( 'TileCellMakerCfg' )

    CaloCellMaker, CaloCellContainerFinalizerTool=CompFactory.getComps("CaloCellMaker","CaloCellContainerFinalizerTool",)
    from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
    tileCellBuilder = acc.popToolsAndMerge( TileCellBuilderCfg(flags, SkipGain=skipGain, mergeChannels=mergeChannels) )

    cellMakerTools = [tileCellBuilder, CaloCellContainerFinalizerTool()]

    noiseFilter = flags.Tile.NoiseFilter

    doCellNoiseFilter = noiseFilter - noiseFilter % 100
    doRawChannelNoiseFilter = noiseFilter - doCellNoiseFilter - noiseFilter % 10

    if doRawChannelNoiseFilter == 10:
        msg.info('Use Tile raw channel noise filter')
        from TileRecUtils.TileRawChannelCorrectionConfig import TileRawChannelNoiseFilterCfg
        noiseFilter = acc.popToolsAndMerge( TileRawChannelNoiseFilterCfg(flags) )
        tileCellBuilder.NoiseFilterTools = [noiseFilter]

    if doCellNoiseFilter == 100:
        msg.info('Use Tile cell noise filter')
        from TileRecUtils.TileCellNoiseFilterConfig import TileCellNoiseFilterCfg
        cellNoiseFilter = acc.popToolsAndMerge( TileCellNoiseFilterCfg(flags) )
        cellMakerTools += [ cellNoiseFilter ]

    if doCaloNeighborsCorrection:
        msg.info('Use Calo cell neighbours average correction')
        caloCellNeighborsAverageCorrection = acc.popToolsAndMerge( CaloCellNeighborsAverageCorrCfg(flags) )
        cellMakerTools += [caloCellNeighborsAverageCorrection]


    caloCellContainerChecker = acc.popToolsAndMerge( CaloCellContainerCheckerToolCfg(flags) )
    cellMakerTools += [caloCellContainerChecker]

    cellMakerAlg = CaloCellMaker(name = name, CaloCellMakerToolNames = cellMakerTools,
                                 CaloCellsOutputName = caloCellsOutputName)

    acc.addEventAlgo(cellMakerAlg, primary = True)

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Tile.RunType = TileRunType.PHY
    flags.Tile.doOptATLAS = True
    flags.Tile.correctTimeJumps = True
    flags.Tile.NoiseFilter = 1
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Exec.MaxEvents=3
    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    acc.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )

    acc.merge( TileCellMakerCfg(flags) )

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    acc.merge(OutputStreamCfg(flags, "ESD",
                              ItemList = [ 'CaloCellContainer#*', 'TileCellContainer#*']))

    acc.getEventAlgo(outputStreamName("ESD")).ExtraInputs = {('CaloCellContainer', 'StoreGateSvc+AllCalo')}

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileCellMaker.pkl','wb') )

    # Needed to work around cling crash in dbg build...
    import ROOT
    ROOT.CaloCellContainer

    sc = acc.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

