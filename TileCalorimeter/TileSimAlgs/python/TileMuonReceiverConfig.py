# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile pulse for muon receiver algorithm"""

from TileSimAlgs.TileHitVecToCntConfig import TileHitVecToCntCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep

def TilePulseForTileMuonReceiverCfg(flags, **kwargs):
    """Return component accumulator with configured Tile muon receiver algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('name', 'TilePulseForTileMuonReceiver')
    kwargs.setdefault('TileHitContainer', 'TileHitCnt')
    kwargs.setdefault('MuonReceiverRawChannelContainer', 'MuRcvRawChCnt')
    kwargs.setdefault('MaskBadChannels', False)
    kwargs.setdefault('UseCoolPulseShapes', True)
    kwargs.setdefault('UseCoolPedestal', False)
    kwargs.setdefault('UseCoolNoise', False)
    kwargs.setdefault('TilePulseShape', 'TileMuRcvPulseShape')

    acc = TileHitVecToCntCfg(flags)

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )
    infoLoader = acc.getService('TileInfoLoader')
    pedestal = infoLoader._descriptors['MuRcvPed'].default

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    acc.merge( TileSamplingFractionCondAlgCfg(flags) )

    if 'RndmSvc' not in kwargs:
        from RngComps.RngCompsConfig import AthRNGSvcCfg
        kwargs['RndmSvc'] = acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name

    if kwargs['UseCoolNoise'] or kwargs['UseCoolPedestal']:
        from TileConditions.TileSampleNoiseConfig import TileSampleNoiseCondAlgCfg
        acc.merge( TileSampleNoiseCondAlgCfg(flags) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    if kwargs['MaskBadChannels']:
        from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
        acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if kwargs['UseCoolPulseShapes']:
        from TileConditions.TilePulseShapeConfig import TilePulseShapeCondAlgCfg
        acc.merge( TilePulseShapeCondAlgCfg(flags, Source='FILE', TilePulseShape=kwargs['TilePulseShape'], PulseType='MURCV') )

    if 'TileRawChannelBuilderMF' not in kwargs:
        from TileConditions.TilePulseShapeConfig import TileCondToolMuRcvPulseShapeCfg
        pulseShapeTool = acc.popToolsAndMerge( TileCondToolMuRcvPulseShapeCfg(flags) )

        from TileConditions.TileOFCConfig import TileCondToolOfcCfg
        ofcTool = acc.popToolsAndMerge( TileCondToolOfcCfg(flags,
                                                           OptFilterDeltaCorrelation = True,
                                                           TileCondToolPulseShape = pulseShapeTool) )


        from TileRecUtils.TileRawChannelBuilderMFConfig import TileRawChannelBuilderMFCfg
        rawChanBuilder = acc.popToolsAndMerge( TileRawChannelBuilderMFCfg(flags, MF = 1,
                                                                          PedestalMode = 0,
                                                                          DefaultPedestal = pedestal,
                                                                          TileCondToolOfcOnFly = ofcTool,
                                                                          TileCondToolOfc = ofcTool,
                                                                          TileRawChannelContainer = "") )
        kwargs['TileRawChannelBuilderMF'] = rawChanBuilder


    kwargs.setdefault('IntegerDigits', flags.Common.ProductionStep != ProductionStep.PileUpPresampling)

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault('MuonReceiverDigitsContainer', flags.Overlay.BkgPrefix + 'MuRcvDigitsCnt')
    else:
        kwargs.setdefault('MuonReceiverDigitsContainer', 'MuRcvDigitsCnt')

    if flags.Common.isOverlay and flags.Concurrency.NumThreads > 0:
        kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

    TilePulseForTileMuonReceiver=CompFactory.TilePulseForTileMuonReceiver
    acc.addEventAlgo(TilePulseForTileMuonReceiver(**kwargs), primary = True)

    return acc


def TilePulseForTileMuonReceiverOutputCfg(flags, **kwargs):
    """Return component accumulator with configured Tile muon receiver algorithm and Output stream

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = TilePulseForTileMuonReceiverCfg(flags, **kwargs)
    tilePulseForMuRcv = acc.getPrimary()

    if hasattr(tilePulseForMuRcv, 'MuonReceiverDigitsContainer'):
        muRcvDigitsCnt = tilePulseForMuRcv.MuonReceiverDigitsContainer
    else:
        muRcvDigitsCnt = tilePulseForMuRcv.getDefaultProperty('MuonReceiverDigitsContainer')
    muRcvDigitsCnt = str(muRcvDigitsCnt).split('+').pop()
    outputItemList = ['TileDigitsContainer#' + muRcvDigitsCnt]

    if flags.Common.ProductionStep != ProductionStep.PileUpPresampling:
        if hasattr(tilePulseForMuRcv, 'MuonReceiverRawChannelContainer'):
            muRcvRawChCnt = tilePulseForMuRcv.MuonReceiverRawChannelContainer
        else:
            muRcvRawChCnt = tilePulseForMuRcv.getDefaultProperty('MuonReceiverRawChannelContainer')
        muRcvRawChCnt = str(muRcvRawChCnt).split('+').pop()
        outputItemList += ['TileRawChannelContainer#' + muRcvRawChCnt]

    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge( OutputStreamCfg(flags, streamName = 'RDO', ItemList = outputItemList) )

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.HITS_RUN2
    flags.Output.RDOFileName = 'myRDO-TileMuonReceiver.pool.root'
    flags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'
    flags.Digitization.PileUp = False
    flags.Exec.MaxEvents = 3

    flags.fillFromArgs()
    flags.lock()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    if 'EventInfo' not in flags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        acc.merge(EventInfoCnvAlgCfg(flags,
                                     inputKey='McEventInfo',
                                     outputKey='EventInfo'))

    acc.merge( TilePulseForTileMuonReceiverOutputCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    flags.dump()
    acc.store( open('TileMuonReceiver.pkl','wb') )

    sc = acc.run()
    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
